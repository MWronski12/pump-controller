#include "system_init.h"

void top_water_sensor_isr_handler()
{

    if (REFILLING_FLAG == 1)
    {

        pump_off(PUMP_MAIN_PIN);
        REFILLING_FLAG = 0;
    }
}

void bottom_water_sensor_isr_handler()
{

    REFILLING_FLAG = 1;
    pump_on(PUMP_MAIN_PIN);
}

/* -------------------------------------------------------------------------- */
/*         Configures gpio according to values defined in app_config.h        */
/* -------------------------------------------------------------------------- */
void system_init()
{
    const char *TAG = "system_init";
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

    ESP_LOGI(TAG, "System initialized!");
}