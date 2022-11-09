#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

// Espressif
#include "esp_log.h"

// Components
#include "water_sensor_driver.h"
#include "pump_driver.h"
#include "system_init.h"
#include "task_pump_controller.h"
#include "task_mqtt_listener.h"

// App config file
#include "app_config.h"

// Global variables
QueueHandle_t pump_controller_msg_queue = NULL;
volatile uint8_t REFILLING_FLAG = 0;

void app_main()
{
    const char *TAG = "main";

    system_init();

    // Global variables initialization
    pump_controller_msg_queue = xQueueCreate(10, sizeof(pump_controller_msg_t));

    // Create tasks
    xTaskCreate(task_pump_controller, "task_pump_controller", 2048, NULL, 10, NULL);
    xTaskCreate(task_mqtt_listener, "task_mqtt_listen", 2048, NULL, 10, NULL);

    ESP_LOGI(TAG, "System initialized!");
}
