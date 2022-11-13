// Freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

// Espressif
#include "esp_log.h"

// Drivers
#include "water_sensor_driver.h"
#include "pump_driver.h"

// Setup
#include "refilling_system_init.h"
#include "mqtt_init.h"

// Tasks
#include "task_pump_controller.h"

// App config file
#include "app_config.h"

// Global variables
QueueHandle_t pump_controller_msg_queue = NULL;
esp_mqtt_client_handle_t client = NULL;
volatile uint8_t REFILLING_FLAG = 0;

void app_main()
{
    const char *TAG = "main";

    // Global variables
    pump_controller_msg_queue = xQueueCreate(10, sizeof(pump_controller_msg_t));
    client = esp_mqtt_client_init(&mqtt_cfg);

    // Setup
    refilling_system_init();
    mqtt_init();

    // Tasks
    xTaskCreate(task_pump_controller, "task_pump_controller", 2048, NULL, 10, NULL);

    ESP_LOGI(TAG, "System initialized!");
}
