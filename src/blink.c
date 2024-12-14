#include "delay.h"
#include "stm32f1.h"
#include <stdint.h>

#define HIGH 1
#define LOW 0

#define DISPLAY_OFF 0xFFFF

uint8_t digits[] = {0b11111100, 0b11000000,
                    0b10110101, 0b11010101,
                    0b11001001, 0b01011101,
                    0b01111101, 0b11000100,
                    0b11111101, 0b11011101};

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

void main() {
    setup();
    

    /* delay(2000);
    for (int i = 1; i < 10; i++) {

        uint16_t value = (digits[i - 1] << 8) | digits[i];

        send_serial_delay(value, 1);
        delay(1000);
    } */

    delay(1000);

    //GPIOA_ODR |= 0x6;
    /* uint16_t value = (digits[6] << 8) | digits[1]; // construct the value: zero << 8 + one
    send_serial(value); // send the data */

    // int set_time = 5;

    //uint16_t value = 0;
    /* while (1)
    {
        // buttons


        // activate displays and send data

        // activate left displays [PA1]
        GPIOA_ODR |= 0x2;
        value = (digits[1] << 8) | digits[1]; // construct the value: zero << 8 + one
        send_serial(value); // send the data
        //GPIOA_ODR &= ~0x2; // turn the left displays off

        // wait some 1-2 ms
        delay(1000);

        // activate right displays [PA2]
        //GPIOA_ODR |= 0x4;
        //value = (digits[0] << 8) | digits[6]; // construct the value: zero << 8 + six
        //send_serial(value); // send the data
        //GPIOA_ODR &= ~0x4; // turn the left displays off

        //delay(1000);
    } */
}
