#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal/hal_err.h"
#include "stm32wbxx.h"
#include <stdbool.h>

// TYPES

#define ADC_CHANNEL_NA -1 // Not available

typedef struct {
    volatile GPIO_TypeDef *gpio;
    uint8_t num;
    int8_t adc_chan;
} gpio_pin_t;

#define HIGH true
#define LOW false

typedef enum {
    GPIO_MODE_INPUT = 0b00,
    GPIO_MODE_OUTPUT = 0b01,
    GPIO_MODE_AF = 0b10,
    GPIO_MODE_ANALOG = 0b11,
} gpio_mode;

typedef enum {
    GPIO_AF_MODE_0 = 0b0000,
    GPIO_AF_MODE_1 = 0b0001,
    GPIO_AF_MODE_2 = 0b0010,
    GPIO_AF_MODE_3 = 0b0011,
    GPIO_AF_MODE_4 = 0b0100,
    GPIO_AF_MODE_5 = 0b0101,
    GPIO_AF_MODE_6 = 0b0110,
    GPIO_AF_MODE_7 = 0b0111,
    GPIO_AF_MODE_8 = 0b1000,
    GPIO_AF_MODE_9 = 0b1001,
    GPIO_AF_MODE_10 = 0b1010,
    GPIO_AF_MODE_11 = 0b1011,
    GPIO_AF_MODE_12 = 0b1100,
    GPIO_AF_MODE_13 = 0b1101,
    GPIO_AF_MODE_14 = 0b1110,
    GPIO_AF_MODE_15 = 0b1111,
} gpio_af_mode;

typedef enum {
    GPIO_PUPD_NONE = 0b00,
    GPIO_PULLUP = 0b01,
    GPIO_PULLDOWN = 0b10,
} gpio_pupd;

typedef enum {
    GPIO_SPEED_LOW = 0b00,
    GPIO_SPEED_MED = 0b01,
    GPIO_SPEED_FAST = 0b10,
    GPIO_SPEED_HIGH = 0b11,
} gpio_speed;

typedef enum {
    GPIO_OUTPUT_PUSH_PULL = 0b0,
    GPIO_OUTPUT_OPEN_DRAIN = 0b1,
} gpio_output_type;

typedef enum {
    GPIO_CALIB_INPUT_SINGLE_ENDED,
    GPIO_CALIB_INPUT_DIFFERENTIAL,
} gpio_calib_input_mode;

// PINS

#define PA0 (gpio_pin_t){.gpio = GPIOA, .num = 0, .adc_chan = 5}
#define PA1 (gpio_pin_t){.gpio = GPIOA, .num = 1, .adc_chan = 6}
#define PA2 (gpio_pin_t){.gpio = GPIOA, .num = 2, .adc_chan = 7}
#define PA3 (gpio_pin_t){.gpio = GPIOA, .num = 3, .adc_chan = 8}
#define PA4 (gpio_pin_t){.gpio = GPIOA, .num = 4, .adc_chan = 9}
#define PA5 (gpio_pin_t){.gpio = GPIOA, .num = 5, .adc_chan = 10}
#define PA6 (gpio_pin_t){.gpio = GPIOA, .num = 6, .adc_chan = 11}
#define PA7 (gpio_pin_t){.gpio = GPIOA, .num = 7, .adc_chan = 12}
#define PA8 (gpio_pin_t){.gpio = GPIOA, .num = 8, .adc_chan = 15}
#define PA9 (gpio_pin_t){.gpio = GPIOA, .num = 9, .adc_chan = 16}

#define PA10                                                                   \
    (gpio_pin_t) { .gpio = GPIOA, .num = 10, .adc_chan = ADC_CHANNEL_NA }
#define PA11                                                                   \
    (gpio_pin_t) { .gpio = GPIOA, .num = 11, .adc_chan = ADC_CHANNEL_NA }
#define PA12                                                                   \
    (gpio_pin_t) { .gpio = GPIOA, .num = 12, .adc_chan = ADC_CHANNEL_NA }
#define PA13                                                                   \
    (gpio_pin_t) { .gpio = GPIOA, .num = 13, .adc_chan = ADC_CHANNEL_NA }
#define PA14                                                                   \
    (gpio_pin_t) { .gpio = GPIOA, .num = 14, .adc_chan = ADC_CHANNEL_NA }
#define PA15                                                                   \
    (gpio_pin_t) { .gpio = GPIOA, .num = 15, .adc_chan = ADC_CHANNEL_NA }

#define PB0                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 0, .adc_chan = ADC_CHANNEL_NA }
#define PB1                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 1, .adc_chan = ADC_CHANNEL_NA }
#define PB2                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 2, .adc_chan = ADC_CHANNEL_NA }
#define PB3                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 3, .adc_chan = ADC_CHANNEL_NA }
#define PB4                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 4, .adc_chan = ADC_CHANNEL_NA }
#define PB5                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 5, .adc_chan = ADC_CHANNEL_NA }
#define PB6                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 6, .adc_chan = ADC_CHANNEL_NA }
#define PB7                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 7, .adc_chan = ADC_CHANNEL_NA }
#define PB8                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 8, .adc_chan = ADC_CHANNEL_NA }
#define PB9                                                                    \
    (gpio_pin_t) { .gpio = GPIOB, .num = 9, .adc_chan = ADC_CHANNEL_NA }

#define PB10                                                                   \
    (gpio_pin_t) { .gpio = GPIOB, .num = 10, .adc_chan = ADC_CHANNEL_NA }
#define PB11                                                                   \
    (gpio_pin_t) { .gpio = GPIOB, .num = 11, .adc_chan = ADC_CHANNEL_NA }
#define PB12                                                                   \
    (gpio_pin_t) { .gpio = GPIOB, .num = 12, .adc_chan = ADC_CHANNEL_NA }
#define PB13                                                                   \
    (gpio_pin_t) { .gpio = GPIOB, .num = 13, .adc_chan = ADC_CHANNEL_NA }
#define PB14                                                                   \
    (gpio_pin_t) { .gpio = GPIOB, .num = 14, .adc_chan = ADC_CHANNEL_NA }
#define PB15                                                                   \
    (gpio_pin_t) { .gpio = GPIOB, .num = 15, .adc_chan = ADC_CHANNEL_NA }

#define PC0 (gpio_pin_t){.gpio = GPIOC, .num = 0, .adc_chan = 0}
#define PC1 (gpio_pin_t){.gpio = GPIOC, .num = 1, .adc_chan = 1}
#define PC2 (gpio_pin_t){.gpio = GPIOC, .num = 2, .adc_chan = 2}
#define PC3 (gpio_pin_t){.gpio = GPIOC, .num = 3, .adc_chan = 3}
#define PC4 (gpio_pin_t){.gpio = GPIOC, .num = 4, .adc_chan = 4}
#define PC5                                                                    \
    (gpio_pin_t) { .gpio = GPIOC, .num = 5, .adc_chan = ADC_CHANNEL_NA }
#define PC6                                                                    \
    (gpio_pin_t) { .gpio = GPIOC, .num = 6, .adc_chan = ADC_CHANNEL_NA }

#define PC10                                                                   \
    (gpio_pin_t) { .gpio = GPIOC, .num = 10, .adc_chan = ADC_CHANNEL_NA }
#define PC11                                                                   \
    (gpio_pin_t) { .gpio = GPIOC, .num = 11, .adc_chan = ADC_CHANNEL_NA }
#define PC12                                                                   \
    (gpio_pin_t) { .gpio = GPIOC, .num = 12, .adc_chan = ADC_CHANNEL_NA }
#define PC13                                                                   \
    (gpio_pin_t) { .gpio = GPIOC, .num = 13, .adc_chan = ADC_CHANNEL_NA }
#define PC14                                                                   \
    (gpio_pin_t) { .gpio = GPIOC, .num = 14, .adc_chan = ADC_CHANNEL_NA }
#define PC15                                                                   \
    (gpio_pin_t) { .gpio = GPIOC, .num = 15, .adc_chan = ADC_CHANNEL_NA }

#define PD0                                                                    \
    (gpio_pin_t) { .gpio = GPIOD, .num = 0, .adc_chan = ADC_CHANNEL_NA }

#define PE4                                                                    \
    (gpio_pin_t) { .gpio = GPIOE, .num = 4, .adc_chan = ADC_CHANNEL_NA }

#define PH3                                                                    \
    (gpio_pin_t) { .gpio = GPIOH, .num = 3, .adc_chan = ADC_CHANNEL_NA }

// FUNCTIONS

void gpio_turn_on_port(volatile GPIO_TypeDef *port);

void gpio_turn_off_port(volatile GPIO_TypeDef *port);

void gpio_set_mode(gpio_pin_t pin, gpio_mode mode);

void gpio_set_af_mode(gpio_pin_t pin, gpio_af_mode mode);

void gpio_set_pupd(gpio_pin_t pin, gpio_pupd pupd);

void gpio_set_speed(gpio_pin_t pin, gpio_speed speed);

void gpio_set_output_type(gpio_pin_t pin, gpio_output_type type);

void gpio_digital_write(gpio_pin_t pin, bool val);

bool gpio_digital_read(gpio_pin_t pin);

hal_err gpio_enable_interrupt(gpio_pin_t pin);

hal_err gpio_set_interrupt_falling(gpio_pin_t pin);

hal_err gpio_set_interrupt_rising(gpio_pin_t pin);

#endif // HAL_GPIO_H
