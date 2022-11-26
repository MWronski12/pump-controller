// Standard library
#include "string.h"

// Espressif
#include "esp_heap_caps.h"
#include "esp_log.h"

// App config file
#include "app_config.h"

// Header
#include "form_parser.h"

static const char *TAG = "form_parser";

/* -------------------------------------------------------------------------- */
/*                       parse form encoded mqtt payload                      */
/* -------------------------------------------------------------------------- */
pump_controller_msg_t *form_parser(char *payload, int length)
{
    ESP_LOGI(TAG, "Parsing message=%.*s", length, payload);

    /* ------------ allocate memory for pump_controller_msg_t on heap ----------- */
    pump_controller_msg_t *msg = (pump_controller_msg_t *)heap_caps_malloc(sizeof(pump_controller_msg_t), MALLOC_CAP_32BIT);
    if (msg == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for message on heap!");
    }
    msg->type = NEW_TASK_MSG;

    /* ---------------------------- trim whitespaces ---------------------------- */
    if (length < 0 || length > MQTT_MAX_PAYLOAD_LENGTH)
    {
        return NULL;
    }

    char payload_trimmed[MQTT_MAX_PAYLOAD_LENGTH];
    memcpy(payload_trimmed, payload, length);
    payload_trimmed[length] = '\0';
    for (int i = 0; i < length; i++)
    {
        if (payload_trimmed[i] == ' ' || payload_trimmed[i] == '\n' || payload_trimmed[i] == '\t')
        {
            memcpy(payload_trimmed + i, payload_trimmed + i + 1, length - i);
            length--;
            i--;
        }
    }

    /* ---------------- assert there is exactly one '&' and 2 '=' --------------- */
    int ampersand_count = 0;
    int equal_count = 0;
    for (int i = 0; i < length; i++)
    {
        if (payload_trimmed[i] == '&')
        {
            ampersand_count++;
        }
        else if (payload_trimmed[i] == '=')
        {
            equal_count++;
        }
    }

    /* ------------------ extract pump_gpio and duration_s keys ----------------- */
    if (ampersand_count != 1 || equal_count != 2)
    {
        ESP_LOGE(TAG, "Special chars count error!");
        return NULL;
    }

    char *pump_gpio_str = strstr(payload_trimmed, "pumpGpio");
    char *duration_s_str = strstr(payload_trimmed, "durationS");

    if (pump_gpio_str == NULL || duration_s_str == NULL)
    {
        ESP_LOGE(TAG, "Keys error!");
        return NULL;
    }

    if (*(pump_gpio_str + strlen("pumpGpio")) != '=' || *(duration_s_str + strlen("durationS")) != '=')
    {
        ESP_LOGE(TAG, "Special chars placement error!");
        return NULL;
    }

    /* -------------------------- parse pump_gpio value ------------------------- */
    char pump_gpio[12];
    char *c = pump_gpio_str + strlen("pumpGpio") + 1;
    int i = 0;

    while (*(c + i) != '&' && *(c + i) != '\0')
    {
        if (i > 10)
        {
            ESP_LOGE(TAG, "Deciaml represantation of pump_gpio value is too big!");
            return NULL;
        }
        pump_gpio[i] = *(c + i);
        i++;
    }
    pump_gpio[i] = '\0';

    /* ------------------------- parse duration_s value ------------------------- */
    char duration_s[12];
    c = duration_s_str + strlen("durationS") + 1;
    i = 0;

    while (*(c + i) != '&' && *(c + i) != '\0')
    {
        if (i > 10)
        {

            ESP_LOGE(TAG, "Deciaml represantation of durationS value is too big!");
            return NULL;
        }
        duration_s[i] = *(c + i);
        i++;
    }
    duration_s[i] = '\0';

    /* ------------------- assert pump_gpio is a valid uint8_t ------------------ */
    int pump_gpio_int = atoi(pump_gpio);
    if (pump_gpio_int < 0 || pump_gpio_int > 0xff)
    {

        ESP_LOGE(TAG, "pump_gpio overflow!");
        return NULL;
    }

    /* ------------------ assert duration_s is a valid uint16_t ----------------- */
    int duration_s_int = atoi(duration_s);
    if (duration_s_int <= 0 || duration_s_int > 0xffff)
    {

        ESP_LOGE(TAG, "durationS overflow!");
        return NULL;
    }

    msg->pump_gpio = (uint8_t)pump_gpio_int;
    msg->duration_s = (uint16_t)duration_s_int;

    return msg;
}
