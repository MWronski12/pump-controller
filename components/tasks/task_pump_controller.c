#include "task_pump_controller.h"

pump_t pumps_config[] = {
    {.id = 0, .gpio = PUMP_0_PIN, .has_active_task = 0, .timer = NULL},
    // {.id = 1, .gpio = PUMP_1_PIN, .has_active_task = 0, .timer = NULL},
    // {.id = 2, .gpio = PUMP_2_PIN, .has_active_task = 0, .timer = NULL},
};

void timer_callback(TimerHandle_t timer)
{
    uint32_t pump_id = (uint32_t)pvTimerGetTimerID(timer);
    ESP_LOGI("timer_callback", "timer expired pumid=%d", pump_id);
    (&pumps_config[pump_id])->has_active_task = 0;
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
    pump_t *pump;
    TickType_t duration_ticks;

    for (;;)
    {
        if (xQueueReceive(pump_controller_msg_queue, &msg, portMAX_DELAY))
        {

            ESP_LOGI(TAG, "New message received!");

            switch (msg.type)
            {

            case NEW_TASK:

                pump = &pumps_config[msg.pump_id];
                assert(pump->id == msg.pump_id);

                if (pump->has_active_task == 1)
                {
                    ESP_LOGE(TAG, "Previous task of pump with id=%d was not finished yet! Aborting task request...", pump->id);
                    break;
                }

                ESP_LOGI(TAG, "{ type = %d, durations_s = %d s, pump_id = %d }", msg.type, msg.duration_s, msg.pump_id);

                ESP_LOGW(TAG, "pump->has_active_task=%d", pump->has_active_task);
                pump->has_active_task = 1;
                ESP_LOGW(TAG, "Refilling flag=%d", REFILLING_FLAG);

                duration_ticks = pdMS_TO_TICKS(msg.duration_s * 1000);

                xTimerChangePeriod(pump->timer, duration_ticks, 0);
                xTimerStop(pump->timer, 100);

                if (REFILLING_FLAG == 0)
                {
                    xTimerStart(pump->timer, 0);
                    pump_on(pump->id);
                    ESP_LOGI(TAG, "Timer %d started with period=%ds", (uint32_t)pvTimerGetTimerID(pump->timer), (uint32_t)(xTimerGetPeriod(pump->timer) / configTICK_RATE_HZ));
                }

                break;

            case PAUSE_TASKS:

                ESP_LOGI(TAG, "{ type = PAUSE_TASKS }");

                if (REFILLING_FLAG == 1)
                {
                    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
                    {
                        pump = &pumps_config[i];

                        if (pump->has_active_task && xTimerIsTimerActive(pump->timer))
                        {
                            if (xTimerStop(pump->timer, pdMS_TO_TICKS(100)))
                            {
                                pump_off(pump->gpio);
                                ESP_LOGI(TAG, "Timer paused successfully!");
                            }
                            else
                            {
                                ESP_LOGE(TAG, "Pause command was not sent to timer!");
                            }
                        }
                    }
                }
                else
                {
                    ESP_LOGE(TAG, "Tasks should be paused only when refilling was started!");
                }
                break;

            case START_TASKS:

                ESP_LOGI(TAG, "{ type = START_TASKS }");

                if (REFILLING_FLAG == 0)
                {
                    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
                    {
                        pump = &pumps_config[i];

                        if (pump->has_active_task && !xTimerIsTimerActive(pump->timer))
                        {
                            if (xTimerStart(pump->timer, pdMS_TO_TICKS(100)))
                            {
                                pump_on(pump->gpio);
                                ESP_LOGI(TAG, "Timer started successfully!");
                            }
                            else
                            {
                                ESP_LOGE(TAG, "Start command was not sent to timer!");
                            }
                        }
                    }
                }
                else
                {
                    ESP_LOGE(TAG, "Tasks should be started only when refilling is finished!");
                }

                break;

            default:
                break;
            }
        }
        else
        {
            ESP_LOGE(TAG, "Error receiving message from the queue!");
        }
    }
}