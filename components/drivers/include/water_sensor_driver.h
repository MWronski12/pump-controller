#pragma once

// Espressif
#include "driver/gpio.h"

// App configuration file
#include "app_config.h"

void water_sensor_config(
    gpio_num_t signal_pin,
    gpio_num_t mode_pin);
