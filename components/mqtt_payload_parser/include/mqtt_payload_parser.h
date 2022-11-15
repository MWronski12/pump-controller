#pragma once

#include "string.h"

#include "esp_heap_caps.h"

#include "task_pump_controller.h"

mqtt_payload_to_pump_controller_msg_t(char *payload, int length);
