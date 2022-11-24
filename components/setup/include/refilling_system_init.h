#pragma once

// Espressif
#include "driver/gpio.h"
#include "esp_log.h"

// Drivers
#include "pump_driver.h"
#include "water_sensor_driver.h"

// Tasks
#include "task_pump_controller.h"

// App configuration file
#include "app_config.h"

extern TaskHandle_t task_mqtt_logger_handle;
extern volatile uint8_t REFILLING_FLAG;

void refilling_system_init();