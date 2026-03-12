#ifndef INPUT_MODULE_H
#define INPUT_MODULE_H

#include <stdbool.h>

void input_mobile_init(int btn_gpio, int debounce_ms);
void input_module_update(void);
bool input_module_is_setup_mode(void);
bool input_module_fan_pressed(void);

#endif
