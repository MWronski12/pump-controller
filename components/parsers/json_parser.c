// Standard library
#include <stdio.h>
#include <string.h>

// Espressif
#include "cJSON.h"
#include "esp_log.h"

// Tasks
#include "task_pump_controller.h"

// App config file
#include "app_config.h"

// Header
#include "json_parser.h"

static const char *TAG = "json_parser";

/* -------------------------------------------------------------------------- */
/*                       parse json encoded mqtt payload                      */
/* -------------------------------------------------------------------------- */
pump_controller_msg_t *json_parser(char *payload, int length)
{
    ESP_LOGI(TAG, "Parsing message=%.*s", length, payload);

    /* ------------------- make payload NULL terminated string ------------------ */
    char payload_buff[MQTT_MAX_PAYLOAD_LENGTH];
    memcpy(payload_buff, payload, length);
    payload_buff[length] = '\0';

    /* ------------ allocate memory for pump_controller_msg_t on heap ----------- */
    pump_controller_msg_t *msg = (pump_controller_msg_t *)heap_caps_malloc(sizeof(pump_controller_msg_t), MALLOC_CAP_32BIT);
    if (msg == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for message on heap!");
    }
    msg->type = NEW_TASK_MSG;

    /* ------------------------- initialize cJSON struct ------------------------ */
    const cJSON *pump_gpio = NULL;
    const cJSON *duration_s = NULL;

    cJSON *message = cJSON_Parse(payload_buff);

    if (message == NULL)
    {
        ESP_LOGW(TAG, "Could not parse payload: %.*s", length, payload);
        msg = NULL;
        goto end;
    }

    /* ----------------------------- parse "pumpId" ----------------------------- */
    pump_gpio = cJSON_GetObjectItemCaseSensitive(message, "pumpGpio");
    if (cJSON_IsNumber(pump_gpio) && pump_gpio->valueint >= 0 && pump_gpio->valueint <= 0xff)
    {
        ESP_LOGI(TAG, "Checking pump_gpio=%d", pump_gpio->valueint);
        msg->pump_gpio = pump_gpio->valueint;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid pumpId value!");
        msg = NULL;
        goto end;
    }

    /* ----------------------------- parse durationS ---------------------------- */
    duration_s = cJSON_GetObjectItemCaseSensitive(message, "durationS");
    if (cJSON_IsNumber(duration_s) && duration_s->valueint > 0 && duration_s->valueint <= 0xffff)
    {
        ESP_LOGI(TAG, "Checking duration_s=%d", duration_s->valueint);
        msg->duration_s = duration_s->valueint;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid durationS value!");
        msg = NULL;
        goto end;
    }

/* ----------------------- free memory and return msg ----------------------- */
end:
    cJSON_Delete(message);
    return msg;
}