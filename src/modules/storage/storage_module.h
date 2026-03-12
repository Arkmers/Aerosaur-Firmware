#ifndef STORAGE_MODULE_H
#define STORAGE_MODULE_H

#include <stdbool.h>
#include <stddef.h>

bool storage_module_init(void);
bool storage_module_save_wifi(const char *ssid, const char *pass);
bool storage_module_load_wifi(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
bool storage_module_clear_wifi(void);

#endif
