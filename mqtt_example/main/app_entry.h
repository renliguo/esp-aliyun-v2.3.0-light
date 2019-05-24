/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#ifndef __APP_ENTRY_H__
#define __APP_ENTRY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define BUF_SIZE (1024)

typedef struct {
    int argc;
    char **argv;
}app_main_paras_t;

void set_iotx_info(void);
int linkkit_main(void *paras);
#endif
