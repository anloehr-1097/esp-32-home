#ifndef MAIN_INCLUDE_HTTPCLIENT_H_
#define MAIN_INCLUDE_HTTPCLIENT_H_

#include "./EspHttpClientDeleter.h"
#include "./esp_http_client.h"

class HttpClient {
   private:
    EspHttpClientHandle http_client;
    esp_http_client_config_t config;

   public:
    explicit HttpClient(const char* url);
    int set_method(esp_http_client_method_t method);
    int set_url(const char* url);
    int set_header(const char* key, const char* value);
    int set_post_field(const char* data, int len);
    int clear_post_field();
    int perform();
};

#endif  // MAIN_INCLUDE_HTTPCLIENT_H_
