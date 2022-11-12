#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Espressif
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "protocol_examples_common.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

// Components
#include "task_mqtt_client.h"
#include "task_pump_controller.h"

// App config file
#include "app_config.h"

void task_mqtt_client(void);
