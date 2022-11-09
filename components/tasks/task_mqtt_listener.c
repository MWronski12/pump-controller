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
        vTaskDelay(pdMS_TO_TICKS(1000 * 10));
        ESP_LOGI(TAG, "Sending message to pump controller! duration_s=%d s, pump_id=%d", msg.duration_s, msg.pump_id);

        // Disable interrupts, which also use this queue to send messages
        taskENTER_CRITICAL();
        BaseType_t send_success = xQueueSend(pump_controller_msg_queue, &msg, 0);
        taskEXIT_CRITICAL();

        if (send_success)
        {
            ESP_LOGI(TAG, "Message sent successfully!");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send message!");
        }
    }
}