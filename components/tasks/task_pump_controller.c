#include "task_pump_controller.h"

pump_t pumps_config[] = {
    {.id = 0, .gpio = PUMP_0_PIN, .timer = NULL},
    {.id = 1, .gpio = PUMP_1_PIN, .timer = NULL},
    {.id = 2, .gpio = PUMP_2_PIN, .timer = NULL},
};

void timer_callback(TimerHandle_t timer)
{
    uint32_t pump_id = (uint32_t)pvTimerGetTimerID(timer);
    pump_off(pump_id);
}

void pump_timers_config()
{
    const char *TAG = "pump_timers_config";
    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
    {
        pumps_config[i].timer = xTimerCreate(
            "Timer",
            pdMS_TO_TICKS(5000),
            pdFALSE,
            (void *)i,
            timer_callback);

        if (pumps_config[i].timer == NULL)
        {
            ESP_LOGE(TAG, "Timer for pump with id=%d was not created!", pumps_config[i].id);
        }
        else
        {
            ESP_LOGI(TAG, "Timer for pump with id=%d created succesfully!", pumps_config[i].id);
        }
    }
}

void task_pump_controller(void *arg)
{
    (void)arg;

    const char *TAG = "task_pump_controller";

    pump_timers_config();

    pump_controller_msg_t msg;
    pump_t pump;
    TickType_t duration_ticks;

    for (;;)
    {
        if (xQueueReceive(pump_controller_msg_queue, &msg, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "New message received! duration_s=%ds, pump_id=%d", msg.duration_s, msg.pump_id);

            pump = pumps_config[msg.pump_id];
            assert(pump.id == msg.pump_id);

            duration_ticks = pdMS_TO_TICKS(msg.duration_s * 1000);
            xTimerChangePeriod(pump.timer, duration_ticks, 0);
            xTimerStart(pump.timer, 0);
            pump_on(pump.id);

            ESP_LOGI(TAG, "Timer %d started with period=%ds", (uint32_t)pvTimerGetTimerID(pump.timer), (uint32_t)(xTimerGetPeriod(pump.timer) / configTICK_RATE_HZ));
        }
        else
        {
            ESP_LOGE(TAG, "Error receiving message from the queue!");
        }
    }
}