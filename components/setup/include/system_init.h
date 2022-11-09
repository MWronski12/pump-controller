#pragma once

#include "driver/gpio.h"
#include "esp_log.h"

#include "pump_driver.h"
#include "water_sensor_driver.h"

#include "app_config.h"

extern volatile uint8_t REFILLING_FLAG;

void top_water_sensor_isr_handler();
void bottom_water_sensor_isr_handler();
void system_init();