#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#include "water_sensor_driver.h"
#include "pump_driver.h"
#include "include/config.h"

#include "esp_log.h"

/* -------------------------------------------------------------------------- */
/*                              REFILLING SYSTEM                              */
/* -------------------------------------------------------------------------- */
volatile uint8_t REFILLING_FLAG = 0;

static void top_water_sensor_isr_handler()
{

    if (REFILLING_FLAG == 1)
    {

        pump_off(PUMP_MAIN_PIN);
        REFILLING_FLAG = 0;
    }
}

static void bottom_water_sensor_isr_handler()
{

    REFILLING_FLAG = 1;
    pump_on(PUMP_MAIN_PIN);
}

void system_gpio_config()
{
    const char *TAG = "system_gpio_config";
    // Bottom sensor config
    water_sensor_config(SENSOR_LOW_SIGNAL_PIN, SENSOR_LOW_MODE_PIN);
    gpio_set_intr_type(SENSOR_LOW_SIGNAL_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(SENSOR_LOW_SIGNAL_PIN, bottom_water_sensor_isr_handler, NULL);

    // Top sensor config
    water_sensor_config(SENSOR_HIGH_SIGNAL_PIN, SENSOR_HIGH_MODE_PIN);
    gpio_set_intr_type(SENSOR_HIGH_SIGNAL_PIN, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(SENSOR_HIGH_SIGNAL_PIN, top_water_sensor_isr_handler, NULL);

    // Pumps config
    pump_config(PUMP_0_PIN);
    // pump_config(PUMP_1_PIN);
    // pump_config(PUMP_2_PIN);
    pump_config(PUMP_MAIN_PIN);

    ESP_LOGI(TAG, "System gpio initialized!");
}

/* -------------------------------------------------------------------------- */
/*                               PUMP CONTROLLER                              */
/* -------------------------------------------------------------------------- */
static QueueHandle_t pump_controller_msg_queue = NULL;

struct pump_controller_msg_t
{
    uint8_t pump_id;
    uint8_t duration_s;
} typedef pump_controller_msg_t;

static void timer_callback(TimerHandle_t timer)
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

void thread_pump_controller(void *arg)
{
    (void)arg;

    const char *TAG = "thread_pump_controller";

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

/* -------------------------------------------------------------------------- */
/*                                 MQTT Module                                */
/* -------------------------------------------------------------------------- */
void thread_mqtt_listener(void *arg)
{
    (void)arg;

    const char *TAG = "thread_mqtt_listener";

    pump_controller_msg_t msg = {
        .duration_s = 5,
        .pump_id = 0,
    };

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Sending message to pump controller! duration_s=%d s, pump_id=%d", msg.duration_s, msg.pump_id);

        if (xQueueSend(pump_controller_msg_queue, &msg, 0))
        {
            ESP_LOGI(TAG, "Message sent successfully!");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send message!");
        }
    }
}

/* -------------------------------------------------------------------------- */
/*                                    MAIN                                    */
/* -------------------------------------------------------------------------- */
void app_main()
{
    const char *TAG = "main";

    system_gpio_config();

    // Global handles initialization
    pump_controller_msg_queue = xQueueCreate(10, sizeof(pump_controller_msg_t));

    // Fill the water for the first time
    if (gpio_get_level(SENSOR_LOW_SIGNAL_PIN) != 1)
    {
        ESP_LOGI(TAG, "Tank was empty, filling the tank for the first time");
        REFILLING_FLAG = 1;
        pump_on(PUMP_MAIN_PIN);
    }
    else
    {
        ESP_LOGW(TAG, "Tank was already full when system initialized!");
    }

    // Create tasks
    xTaskCreate(thread_pump_controller, "thread_pump_controller", 2048, NULL, 10, NULL);
    xTaskCreate(thread_mqtt_listener, "thread_mqtt_listen", 2048, NULL, 10, NULL);

    ESP_LOGI(TAG, "System initialized!");
}
