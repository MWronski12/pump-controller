#include "task_pump_controller.h"

static const char *TAG = "task_pump_controller";

static pump_t pumps_config[] = {
    {.id = 0, .gpio = PUMP_0_PIN, .has_active_task = 0, .timer = NULL},
    // {.id = 1, .gpio = PUMP_1_PIN, .has_active_task = 0, .timer = NULL},
    // {.id = 2, .gpio = PUMP_2_PIN, .has_active_task = 0, .timer = NULL},
};

static pump_t *get_pump_by_id(uint8_t pump_id)
{
    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
    {
        if (pumps_config[i].id == pump_id)
        {
            return &pumps_config[i];
        }
    }
    ESP_LOGW(TAG, "Could not get pump with id=%d", pump_id);
    return NULL;
}

static void timer_callback(TimerHandle_t timer)
{
    const char *timer_name = pcTimerGetName(timer);
    const uint32_t timer_id = *(uint32_t *)pvTimerGetTimerID(timer);
    const uint8_t pump_id = (uint8_t)timer_id;
    ESP_LOGI(TAG, "%s expired pumid=%d", timer_name, pump_id);
    pump_off(pump_id);
}

static void pump_timers_config()
{
    pump_t *pump;
    char timer_name[10];

    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
    {
        pump = &pumps_config[i];
        sprintf(timer_name, "Timer %d", pump->id);

        pump->timer = xTimerCreate(
            timer_name,
            pdMS_TO_TICKS(5000),
            pdFALSE,
            (void *)(uint32_t)pump->id,
            timer_callback);

        if (pump->timer == NULL)
        {
            ESP_LOGE(TAG, "Timer for pump with id=%d was not created!", pump->id);
        }
        else
        {
            ESP_LOGI(TAG, "Timer for pump with id=%d created succesfully!", pump->id);
        }
    }
}

static void on_new_task_msg(pump_controller_msg_t *msg)
{
    pump_t *pump;
    TickType_t duration_ticks;

    pump = get_pump_by_id(msg->pump_id);

    if (pump == NULL)
    {
        ESP_LOGE(TAG, "Pump with id=%d does not exist! Aborting new task request...", msg->pump_id);
        return;
    }

    if (pump->has_active_task)
    {
        ESP_LOGW(TAG, "Previous task of pump with id=%d was not finished yet! Aborting new task request...", pump->id);
        return;
    }

    pump->has_active_task = 1;
    duration_ticks = pdMS_TO_TICKS(msg->duration_s * 1000);
    xTimerChangePeriod(pump->timer, duration_ticks, pdMS_TO_TICKS(100)); // This starts the timer

    if (REFILLING_FLAG == 1)
    {
        xTimerStop(pump->timer, pdMS_TO_TICKS(100));
        ESP_LOGW(TAG, "%s stopped, new task request during refilling the tank", pcTimerGetName(pump->timer));
    }
    else if (REFILLING_FLAG == 0)
    {
        pump_on(pump->id);
        ESP_LOGI(TAG, "%s started with period=%ds", pcTimerGetName(pump->timer), (uint32_t)(xTimerGetPeriod(pump->timer) / configTICK_RATE_HZ));
    }
}

static void on_pause_tasks_msg()
{
    pump_t *pump;

    if (REFILLING_FLAG == 1)
    {
        for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
        {
            pump = &pumps_config[i];

            if (pump->has_active_task && xTimerIsTimerActive(pump->timer))
            {
                if (xTimerStop(pump->timer, pdMS_TO_TICKS(100)) == pdPASS)
                {
                    pump_off(pump->gpio);
                    ESP_LOGI(TAG, "%s paused successfully!", pcTimerGetTimerName(pump->timer));
                }
                else
                {
                    ESP_LOGE(TAG, "Pause command was not sent to %s!", pcTimerGetName(pump->timer));
                }
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "PAUSE_TASKS message received, but tank refilling was not happening!");
    }
}

static void on_start_tasks_msg()
{
    pump_t *pump;

    if (REFILLING_FLAG == 0)
    {
        for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
        {
            pump = &pumps_config[i];

            if (pump->has_active_task && !xTimerIsTimerActive(pump->timer))
            {
                if (xTimerStart(pump->timer, pdMS_TO_TICKS(100)) == pdPASS)
                {
                    pump_on(pump->gpio);
                    ESP_LOGI(TAG, "%s started successfully!", pcTimerGetName(pump->timer));
                }
                else
                {
                    ESP_LOGE(TAG, "Start command was not sent to %s!", pcTimerGetName(pump->timer));
                }
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "START_TASKS message received, but tank refilling was happening!");
    }
}

/* ---------------------------------- main ---------------------------------- */
void task_pump_controller(void *arg)
{
    (void)arg;

    pump_timers_config();

    pump_controller_msg_t msg;

    for (;;)
    {
        if (xQueueReceive(pump_controller_msg_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            switch (msg.type)
            {

            case NEW_TASK:

                ESP_LOGI(TAG, "NEW_TASK msg received! duration_d=%d, pump_id=%d", msg.duration_s, msg.pump_id);
                on_new_task_msg(&msg);
                break;

            case PAUSE_TASKS:

                ESP_LOGI(TAG, "PAUSE_TASKS msg received!");
                on_pause_tasks_msg();
                break;

            case START_TASKS:

                ESP_LOGI(TAG, "START_TASKS msg reveiced!");
                on_start_tasks_msg();
                break;

            default:
                ESP_LOGW(TAG, "Unknown message received!");
                break;
            }
        }
        else
        {
            ESP_LOGE(TAG, "Error receiving message from the queue!");
        }
    }
}