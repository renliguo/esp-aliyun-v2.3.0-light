
#ifndef __DRIVER_CONTROL_H__
#define __DRIVER_CONTROL_H__

#include "esp_err.h"


void GPIO_init(void);
void LED_toggle(void);

 esp_err_t AM2320_i2c_master_init(void);
void AM2320_GetValue(int* hum1,int* temp1);

#endif
