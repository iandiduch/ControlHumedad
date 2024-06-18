#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "driver/adc.h"

#define WIFI_SSID "Ing en Sistemas"
#define WIFI_PASS "ingenieros26"

static const char *TAG = "HTTP_CLIENT";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to the WiFi network...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
    }
}

void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            break;
        default:
            break;
    }
    return ESP_OK;
}

void http_post_task(void *pvParameters) {
    esp_http_client_config_t config = {
        .url = "http://192.168.0.103:5000/post",  // URL de tu endpoint Flask
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    while (1) {
        // Leer del canal ADC1_0 (GPIO36), ADC1_3 (GPIO39), y ADC1_6
        int adc_reading_1 = adc1_get_raw(ADC1_CHANNEL_0);  
        int adc_reading_2 = adc1_get_raw(ADC1_CHANNEL_3);
	int adc_reading_3 = adc1_get_raw(ADC1_CHANNEL_6);
        
	// Verificar los valores leídos del ADC para debug
        ESP_LOGI(TAG, "ADC Reading 1: %d", adc_reading_1);
        ESP_LOGI(TAG, "ADC Reading 2: %d", adc_reading_2);
	ESP_LOGI(TAG, "ADC Reading 3: %d", adc_reading_3);

        // Validar el rango de los valores del ADC
        if (adc_reading_1 < 0) adc_reading_1 = 0;
        if (adc_reading_1 > 4095) adc_reading_1 = 4095;
        if (adc_reading_2 < 0) adc_reading_2 = 0;
        if (adc_reading_2 > 4095) adc_reading_2 = 4095;
	if (adc_reading_3 < 0) adc_reading_3 = 0;
        if (adc_reading_3 > 4095) adc_reading_3 = 4095;

        // Conversión de los valores ADC a % de humedad
        float humidity1 = (1.0 - ((float)adc_reading_1 / 4095.0)) * 100.0;
        float humidity2 = (1.0 - ((float)adc_reading_2 / 4095.0)) * 100.0;
        float humidity3 = (1.0 - ((float)adc_reading_3 / 4095.0)) * 100.0;

        // Verificar los valores calculados de humedad para debug
        ESP_LOGI(TAG, "Calculated Humidity 1: %.2f%%", humidity1);
        ESP_LOGI(TAG, "Calculated Humidity 2: %.2f%%", humidity2);
	ESP_LOGI(TAG, "Calculated Humidity 3: %.2f%%", humidity3);

        char post_data[200];
        snprintf(post_data, sizeof(post_data), "{\"humidity1\":%.2f,\"humidity2\":%.2f, \"humidity3\":%.2f}", humidity1, humidity2, humidity3);

        esp_http_client_set_url(client, "http://192.168.0.103:5000/post");  // URL de tu endpoint Flask
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                     esp_http_client_get_status_code(client),
                     (long long int)esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Esperar 1 segundo entre cada solicitud
    }

    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();

    // Configurar los canales ADC
    adc1_config_width(ADC_WIDTH_BIT_12); // Configurar el ancho de 12 bits para la lectura ADC
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // Configurar la atenuación para 0-3.3V en el canal 0
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11); // Configurar la atenuación para 0-3.3V en el canal 3
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // Configurar la atenuación para 0-3.3V en el canal 6
    // Crear tareas con diferentes tamaños de pila
    xTaskCreate(&http_post_task, "http_post_task", 8192, NULL, 5, NULL); // Tamaño de pila de 8192 bytes
}
