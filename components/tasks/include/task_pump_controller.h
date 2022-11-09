#pragma once

#include "FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "pump_driver.h"
#include "system_init.h"

#include "app_config.h"

extern QueueHandle_t pump_controller_msg_queue;

enum pump_controller_msg_type
{
    NEW_TASK,
    PAUSE_TASKS,
    START_TASKS,
} typedef pump_controller_msg_type;

struct pump_controller_msg_t
{
    pump_controller_msg_type type;
    uint8_t pump_id;
    uint8_t duration_s;
} typedef pump_controller_msg_t;

struct pump_t
{
    uint8_t id;
    gpio_num_t gpio;
    uint8_t has_active_task;
    TimerHandle_t timer;
} typedef pump_t;

static void timer_callback(TimerHandle_t timer);
static void pump_timers_config();
void task_pump_controller(void *arg);