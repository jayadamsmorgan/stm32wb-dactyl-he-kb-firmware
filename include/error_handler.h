#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "hal/hal_err.h"

#define ERR_INV_SYSCLOCK -102

#define ERR_H(ERR_CODE)                                                        \
    do {                                                                       \
        error_handler(ERR_CODE);                                               \
    } while (0)

void setup_error_handler();

void error_handler(hal_err error_code);

#endif // ERROR_HANDLER_H
