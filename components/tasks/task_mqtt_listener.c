#include "task_mqtt_listener.h"

void task_mqtt_listener(void *arg)
{
    (void)arg;

    const char *TAG = "task_mqtt_listener";

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