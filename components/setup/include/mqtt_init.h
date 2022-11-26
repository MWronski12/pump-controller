#pragma once

// Espressif
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t client;
esp_mqtt_client_config_t mqtt_cfg;

/* ----------------------------- CA certificate ----------------------------- */
extern const uint8_t ca_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_cert_pem_end[] asm("_binary_ca_cert_end");

/* --------------------------- Client certificate --------------------------- */
extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");

/* ------------------------------- Client key ------------------------------- */
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");

void mqtt_init(void);
