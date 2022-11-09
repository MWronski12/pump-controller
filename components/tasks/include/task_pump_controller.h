#pragma once

#include "FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "pump_driver.h"

#include "app_config.h"

extern QueueHandle_t pump_controller_msg_queue;

struct pump_controller_msg_t
{
    uint8_t pump_id;
    uint8_t duration_s;
} typedef pump_controller_msg_t;

void timer_callback(TimerHandle_t timer);
void pump_timers_config();
void task_pump_controller(void *arg);