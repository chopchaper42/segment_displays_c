#include "delay.h"
#include "stm32f1.h"
#include <stdint.h>

#define HIGH 1
#define LOW 0

#define ONE_SECOND 250

#define DISPLAY_OFF 0xFFFF

uint8_t digits[] = {0b11111100, 0b11000000,
                    0b10110101, 0b11010101,
                    0b11001001, 0b01011101,
                    0b01111101, 0b11000100,
                    0b11111101, 0b11011101};

void data_set(uint16_t level)
{
    if (!level)
    {
        GPIOC_ODR &= ~0x1; // PC0 LOW
    }
    else
    {
        GPIOC_ODR |= 0x1;  // PC0 HIGH
    }
}

void clk_set(uint16_t level)
{
    if (!level)
    {
        GPIOC_ODR &= ~0x2; // PC1 LOW
    }
    else
    {
        GPIOC_ODR |= 0x2;  // PC1 HIGH
    }
}

void send_serial(uint16_t bits) {
    bits = ~bits;
    for (int8_t i = 15; i >= 0; i--) {
        uint16_t bit = (bits & (1 << i)) >> i; 

        data_set(bit); // set DATA to bit level
        clk_set(HIGH); // on the rising CLK edge shift register loads the value from DATA
        data_set(LOW); // return data to the dafault state
        clk_set(LOW);  // return CLK to the default state

    }
}

void setup() {
    // enable clock for ports A, B and C
    RCC_APB2ENR |= RCC_IOPCEN | RCC_IOPAEN | RCC_IOPBEN; 
    
    // clear configuration registers
    GPIOC_CRL &= ~0xff;  // clear PC0, PC1
    GPIOC_CRH &= ~0xf;   // clear PC8
    GPIOB_CRL &= ~0xfff; // clear PB0, PB1, PB2
    GPIOA_CRL &= ~0xfff; // clear config for PA0, PA1, PA2

    GPIOC_CRH |= 0x1;  // PC8 output
    GPIOC_CRL |= 0x11; // PC0 (DATA) and PC1 (CLK)

    GPIOB_CRL |= 0x888; // PB0 (+ button), PB1 (- button) and PB2 (conf button)

    GPIOA_CRL |= 0x118; // PA0 (Start button), PA1 (left display activation), PA2 (right display activation)

    send_serial((uint16_t)DISPLAY_OFF);

}

void main() {
    setup();
    
    int set_time = 10;

    int raw = 0;
    int debounced = 0;
    int counter = 10;

    uint16_t value = 0;

    int timer = ONE_SECOND;

    while (1)
    {
        // buttons
        /* raw = GPIOB_IDR & ~0x1;

        if (raw != debounced)
        {
            counter = 10;
        }

        if (raw == 1)
        {
            counter--;
        }

        if (counter == 0)
        {
            debounced = raw;
        } */

        if (counter > 0) {
            if (timer > 0) timer--;

            if (timer == 0)
            {
                counter--;
                timer = ONE_SECOND;
            }
        }

        // activate displays and send data

        GPIOA_ODR &= ~0x4; // turn the left displays off
        // activate left displays [PA1]
        GPIOA_ODR |= 0x2;
        value = (digits[counter / 10] << 8) | digits[counter / 10]; // construct the value: zero << 8 + one
        send_serial(value); // send the data

        // wait some 1-2 ms
        delay(2);

        GPIOA_ODR &= ~0x2; // turn the left displays off
        // activate right displays [PA2]
        GPIOA_ODR |= 0x4;
        value = (digits[counter % 10] << 8) | digits[counter % 10]; // construct the value: zero << 8 + six
        send_serial(value); // send the data

        delay(2);

        if (counter == 0) counter = 10;
    }
}
