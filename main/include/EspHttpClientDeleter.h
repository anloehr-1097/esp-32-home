#ifndef ESP_HTTP_CLIENT_DELETER_H
#define ESP_HTTP_CLIENT_DELETER_H

#include <memory>
#include <stdexcept>

#include "esp_http_client.h"

// Custom deleter for esp_http_client_handle_t
struct EspHttpClientDeleter {
    void operator()(esp_http_client_handle_t client) const noexcept {
        if (client) {
            esp_http_client_cleanup(client);
        }
    }
};

// Type alias for convenience
using EspHttpClientHandle =
    std::unique_ptr<esp_http_client, EspHttpClientDeleter>;

#endif  // ESP_HTTP_CLIENT_DELETER_H
