#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "driver/gpio.h"

#include "app_config.h"

#define START_WATER 1
#define STOP_WATER 0

struct pump_t
{
    uint8_t id;
    gpio_num_t gpio;
    TimerHandle_t timer;
} typedef pump_t;

static pump_t pumps_config[] = {
    {.id = 0, .gpio = PUMP_0_PIN, .timer = NULL},
    {.id = 1, .gpio = PUMP_1_PIN, .timer = NULL},
    {.id = 2, .gpio = PUMP_2_PIN, .timer = NULL},
};

gpio_num_t pump_config(gpio_num_t gpio_pin);
void pump_on(gpio_num_t gpio_pin);
void pump_off(gpio_num_t gpio_pin);
