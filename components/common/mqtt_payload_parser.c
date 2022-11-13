#include "mqtt_payload_parser.h"

pump_controller_msg_t *mqtt_payload_to_pump_controller_msg_t(char *payload, int length)
{
    pump_controller_msg_t *msg = (pump_controller_msg_t *)heap_caps_malloc(sizeof(pump_controller_msg_t), MALLOC_CAP_8BIT);

    msg->type = NEW_TASK;

    /* ---------------------------- trim whitespaces ---------------------------- */
    if (length < 0 || length > MQTT_MAX_PAYLOAD_LENGTH)
    {
        return NULL;
    }

    char payload_trimmed[MQTT_MAX_PAYLOAD_LENGTH];
    strcpy(payload_trimmed, payload);
    for (int i = 0; i < length; i++)
    {
        if (payload_trimmed[i] == ' ' || payload_trimmed[i] == '\n' || payload_trimmed[i] == '\t')
        {
            memcpy(payload_trimmed + i, payload_trimmed + i + 1, length - i);
            length--;
            i--;
        }
    }

    /* ---------------- assert there is exactly one '&' and 2 '=' --------------- */
    int ampersand_count = 0;
    int equal_count = 0;
    for (int i = 0; i < length; i++)
    {
        if (payload_trimmed[i] == '&')
        {
            ampersand_count++;
        }
        else if (payload_trimmed[i] == '=')
        {
            equal_count++;
        }
    }

    /* ------------------- extract pump_id and duration_s keys ------------------ */
    if (ampersand_count != 1 || equal_count != 2)
    {
        return NULL;
    }

    char *pump_id_str = strstr(payload_trimmed, "pump_id");
    char *duration_s_str = strstr(payload_trimmed, "duration_s");

    if (pump_id_str == NULL || duration_s_str == NULL)
    {
        return NULL;
    }

    if (*(pump_id_str + strlen("pump_id")) != '=' || *(duration_s_str + strlen("duration_s")) != '=')
    {
        return NULL;
    }

    /* --------------------------- parse pump_id value -------------------------- */
    char pump_id[12];
    char *c = pump_id_str + strlen("pump_id") + 1;
    int i = 0;

    while (*(c + i) != '&' && *(c + i) != '\0')
    {
        if (i > 10)
        {
            return NULL;
        }
        pump_id[i] = *(c + i);
        i++;
    }
    pump_id[i] = '\0';

    /* ------------------------- parse duration_s value ------------------------- */
    char duration_s[12];
    c = duration_s_str + strlen("duration_s") + 1;
    i = 0;

    while (*(c + i) != '&' && *(c + i) != '\0')
    {
        if (i > 10)
        {
            return NULL;
        }
        duration_s[i] = *(c + i);
        i++;
    }
    duration_s[i] = '\0';

    /* -------------------- assert pump_id is a valid uint8_t ------------------- */
    int pump_id_int = atoi(pump_id);
    if (pump_id_int < 0 || pump_id_int > 255)
    {
        return NULL;
    }

    /* ------------------ assert duration_s is a valid uint16_t ----------------- */
    int duration_s_int = atoi(duration_s);
    if (duration_s_int <= 0 || duration_s_int > 65535)
    {
        return NULL;
    }

    msg->pump_id = (uint8_t)pump_id_int;
    msg->duration_s = (uint16_t)duration_s_int;

    return msg;
}
