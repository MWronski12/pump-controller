#pragma once

// SYSTEM
#define TANK_ID "69"
#define TASK_MQTT_CLIENT_PRIORITY 10
#define TASK_MQTT_LOGGER_PRIORITY 10
#define TASK_PUMP_CONTROLLER_PRIORITY 10
#define _TIMER_TASK_PRIORITY 12 // This has to be set in FreeRTOSConfig.h file

// GPIO
#define PUMP_0_PIN GPIO_NUM_0
#define PUMP_1_PIN GPIO_NUM_13
#define PUMP_2_PIN GPIO_NUM_15
#define PUMP_MAIN_PIN GPIO_NUM_2

#define SENSOR_LOW_SIGNAL_PIN GPIO_NUM_5
#define SENSOR_LOW_MODE_PIN GPIO_NUM_4

#define SENSOR_HIGH_SIGNAL_PIN GPIO_NUM_14
#define SENSOR_HIGH_MODE_PIN GPIO_NUM_12

// MQTT
#define MQTT_BROKER_ADDR "mqtt://172.111.242.63:6666"
#define MQTT_MAX_PAYLOAD_LENGTH 50
#define MQTT_USERNAME "roslina"
#define MQTT_PASSWORD "smartcrops"

#define SUBSCRIBE_TOPIC "command/" TANK_ID
#define PUBLISH_TOPIC "logs/" TANK_ID
