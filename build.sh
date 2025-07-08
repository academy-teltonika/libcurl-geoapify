LIBCURL_PREFIX=/home/studentas/Projects/arm/curl-8.11.0/target/arm-ssl
LIBSSL_PREFIX=/home/studentas/Projects/arm/openssl-3.0.14/target/arm/prefix


~/arm_gcc \
main.c \
cJSON.h cJSON.c \
-o geoapify \
-I $LIBCURL_PREFIX/include \
-L $LIBCURL_PREFIX/lib \
-lcurl \
-L $LIBSSL_PREFIX/lib \
-lssl -lcrypto
