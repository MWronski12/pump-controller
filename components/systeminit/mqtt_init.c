// Standard library
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Espressif
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "protocol_examples_common.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// Espressif
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

// Payload parsers
#include "form_parser.h"
#include "json_parser.h"

// Tasks
#include "task_pump_controller.h"

// App config file
#include "app_config.h"

// Header
#include "mqtt_init.h"

static const char *TAG = "mqtt_init";

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

esp_mqtt_client_config_t mqtt_cfg = {
    .uri = MQTT_BROKER_ADDR,
    .event_handle = mqtt_event_handler,
    .task_prio = TASK_MQTT_CLIENT_PRIORITY,
    .client_cert_pem = (const char *)client_cert_pem_start,
    .client_key_pem = (const char *)client_key_pem_start,
    .cert_pem = (const char *)ca_cert_pem_start,
    // .lwt_topic = "TODO!",
    // .lwt_msg = "TODO!",
};

static void on_msg(esp_mqtt_event_handle_t event)
{
    pump_controller_msg_t *msg;

    msg = json_parser(event->data, event->data_len);
    if (msg == NULL)
    {
        msg = form_parser(event->data, event->data_len);
    }

    if (msg != NULL)
    {
        taskENTER_CRITICAL();
        xQueueSend(pump_controller_msg_queue, msg, pdMS_TO_TICKS(100));
        taskEXIT_CRITICAL();
    }
    else
    {
        ESP_LOGE(TAG, "Error parsing mqtt message!");
    }

    heap_caps_free(msg);
    return;
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, SUBSCRIBE_TOPIC, 2);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        on_msg(event);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/*                 Connect to WiFi and start MQTT client task                 */
/* -------------------------------------------------------------------------- */
void mqtt_init(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    esp_mqtt_client_start(client);
}
