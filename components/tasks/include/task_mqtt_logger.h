#pragma once

// Standard library
#include "string.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "freertos/task.h"

// Espressif
#include "mqtt_client.h"
#include "esp_log.h"

// Espressif
#include "app_config.h"

extern esp_mqtt_client_handle_t client;

enum events
{
    REFILLING_START,
    REFILLING_FINISH,
    TASK_START,
    TASK_FINISH,
    TASK_BAD_REQUEST,
};

void task_mqtt_logger(void *arg);
