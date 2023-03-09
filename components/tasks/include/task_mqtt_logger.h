#pragma once

// FreeRTOS
#include "FreeRTOS.h"
#include "freertos/task.h"

// Espressif
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t client;
extern TaskHandle_t task_mqtt_logger_handle;

enum events
{
    REFILLING_START,
    REFILLING_FINISH,
    TASK_START,
    TASK_FINISH,
    TASK_BAD_REQUEST,
};

void task_mqtt_logger(void *arg);
