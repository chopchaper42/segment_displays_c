#include "delay.h"
#include "stm32f1.h"
#include <stdint.h>

typedef enum{
    TIMER,
    SET
} DISPLAY;

typedef enum {
    LOW = 0,
    HIGH = 1
} LOGICAL_LEVEL;

/* void send_serial(int16_t b_value, DISPLAY display) {
    // clock up  <-----
    // data up        |
    // clock down     |
    // data down ------

    // sequentially send 16 bits
    for (int i = 0; i < 16; i++) {
        uint8_t bit = 
        set_data_line();
        set_clock_line(HIGH);
        set_data_line(LOW);
        set_clock_line(LOW);
    }

} */

void setup() {
    // enable clock for ports A, B and C
    RCC_APB2ENR |= RCC_IOPCEN | RCC_IOPAEN | RCC_IOPBEN; 
    
    // clear configuration registers
    GPIOC_CRL &= ~0xff;
    GPIOC_CRH &= ~0xff;
    GPIOB_CRL &= ~0xff;
    GPIOA_CRL &= ~0xff;

    GPIOC_CRH |= 0x1;  // PC8 output
    GPIOC_CRL |= 0x88; // PC0 (DATA) and PC1 (CLK)
    GPIOB_CRL |= 0x888; // PB0 (+ button), PB1 (- button) and PB2 (conf button)
    GPIOA_CRL |= 0x118; // PA0 (Start button), PA1 (left display activation), PA2 (right display activation)

}

void loop() {
    int btn_pressed_counter = 0;
    int plus_btn = 0;
    int minus_btn = 0;
    int confirm_btn = 0;


}

/* void set_data(LOGICAL_LEVEL level) {

    GPIOB_ODR |=  << 7;

} */

void set_clock(LOGICAL_LEVEL level) {
    if (level == LOW) {
        GPIOB_ODR &= ~0x200;
    } else {
        GPIOB_ODR |= 0x200;
    }

}

void main() {
    setup();

    GPIOB_ODR |= 0x100; // set DATA HIGH ...1000 0000

    while (1) {
        //loop();
        delay(500);
        set_clock(HIGH);
        delay(500);
        set_clock(LOW);
    }
}
