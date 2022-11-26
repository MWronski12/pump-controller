#pragma once

// FreeRTOS
#include "FreeRTOS.h"
#include "freertos/task.h"

extern TaskHandle_t task_mqtt_logger_handle;
extern volatile uint8_t REFILLING_FLAG;

void refilling_system_init();