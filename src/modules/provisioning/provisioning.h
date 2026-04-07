#pragma once

#include <stdbool.h>

void provisioning_start(void);
void provisioning_stop(void);
void provisioning_send_ack(bool success, const char *error_msg);
