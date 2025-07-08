#include "curl/curl.h"
#include "curl/easy.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memory {
  char *data;
  size_t size;
};

void _cleanup_free(void *p) {
  if (*(void**) p != NULL) {
    // printf("***\nFreeing string: %s\n***\n", *(char**) p);
    free(*(void**) p);
  }
}

void _cleanup_cJSON(cJSON** p) {
  // puts("Deleting cJSON");
  cJSON_Delete(*p);
}

void _cleanup_curl(CURL **p) {
  // puts("Cleaning up CURL");
  curl_easy_cleanup(*p);
}

void _cleanup_memory(struct memory *p) {
  free(p->data);
}

#define AUTOFREE __attribute__((cleanup(_cleanup_free)))
#define AUTOFREE_CJSON __attribute__((cleanup(_cleanup_cJSON)))
#define AUTOFREE_CURL __attribute__((cleanup(_cleanup_curl)))
#define AUTOFREE_MEMORY __attribute__((cleanup(_cleanup_memory)))

char* parse_response(char* json_string) {
  AUTOFREE_CJSON
  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    puts("PARSE FAILED");
    return NULL;
  }

  cJSON *city = cJSON_GetObjectItemCaseSensitive(json, "city");
  if (!city) {
    return NULL;
  }

  cJSON *name = cJSON_GetObjectItemCaseSensitive(city, "name");
  if (!cJSON_IsString(name) || name->valuestring == NULL) {
    return NULL;
  }

  char *name_string = calloc(strlen(name->valuestring) + 1, sizeof(char));
  if (name_string == NULL) {
    return NULL;
  }
  strcpy(name_string, name->valuestring);

  return name_string;
}

size_t write_cb(char *data, size_t size, size_t nmemb, struct memory *mem) {
  size_t realsize = size * nmemb;
  char *ptr = realloc(mem->data, mem->size + realsize + 1);
  if (!ptr) {
    return -1;
  }

  mem->data = ptr;
  memcpy(&(mem->data[mem->size]), data, realsize);
  mem->size += realsize;
  mem->data[mem->size] = 0;

  return realsize;
}

int main(void) {
  AUTOFREE_CURL
  CURL *curl = curl_easy_init();
  if (!curl) {
    puts("Failed to init curl.");
    return -1;
  }

  CURLcode res;
  // TODO: Should pass this as an argument.
  curl_easy_setopt(curl, CURLOPT_URL, "http://api.geoapify.com/v1/ipinfo?&apiKey=0b7fbe526477431eb0d099582aedeba3");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

  AUTOFREE_MEMORY
  struct memory response = {0};
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    puts("Curl returned an error.");
    return res;
  }

  AUTOFREE
  char *city_name = parse_response(response.data);
  if (city_name == NULL) {
    puts("Failed to parse city name.");
    return -2;
  }
  printf("City name: \"%s\"\n", city_name);
}
