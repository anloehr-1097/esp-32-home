#include "include/HttpClient.h"

#include "./esp_err.h"
#include "./esp_log.h"
#include "esp_http_client.h"

HttpClient::HttpClient(const char* url) {
    config = {
        .url = url,
        .timeout_ms = 15000,
    };
    esp_http_client_handle_t raw_client = esp_http_client_init(&config);

    if (raw_client == NULL) {
        ESP_LOGE("HTTP_CLIENT", "Failed to initialize HTTP client");
    }
    http_client.reset(raw_client);
}

int HttpClient::set_method(esp_http_client_method_t method) {
    if (http_client.get() == nullptr) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client is not initialized");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_set_method(http_client.get(), method);
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to set HTTP method: %s",
                 esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

int HttpClient::set_url(const char* url) {
    if (http_client.get() == nullptr) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client is not initialized");
        return ESP_FAIL;
    }
    esp_err_t err = esp_http_client_set_url(http_client.get(), url);
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to set URL: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

int HttpClient::set_header(const char* key, const char* value) {
    if (http_client.get() == nullptr) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client is not initialized");
        return ESP_FAIL;
    }
    esp_err_t err = esp_http_client_set_header(http_client.get(), key, value);
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to set header %s: %s", key,
                 esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

int HttpClient::set_post_field(const char* data, int len) {
    if (http_client.get() == nullptr) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client is not initialized");
        return ESP_FAIL;
    }
    esp_err_t err =
        esp_http_client_set_post_field(http_client.get(), data, len);
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to set post field: %s",
                 esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

int HttpClient::clear_post_field() {
    if (http_client.get() == nullptr) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client is not initialized");
        return ESP_FAIL;
    }
    char* post_field = "";
    if (esp_http_client_get_post_field(http_client.get(), &post_field) == 0) {
        ESP_LOGI("HTTP_CLIENT", "Post field is already cleared");
        return ESP_OK;
    }

    esp_err_t err = esp_http_client_set_post_field(http_client.get(), "", 0);
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to clear post field: %s",
                 esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

int HttpClient::perform() {
    if (http_client.get() == nullptr) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client is not initialized");
        return ESP_FAIL;
    }
    esp_err_t err = esp_http_client_perform(http_client.get());
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to perform HTTP request: %s",
                 esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}
