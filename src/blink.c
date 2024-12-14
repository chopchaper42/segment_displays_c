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

}

void loop() {

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

// need to send in the big endian style: the MSB first, the LSB last
// THERE IS A PROBLEM!!!
void send_serial(uint16_t bits) {
    bits = ~bits;
    for (int8_t i = 15; i >= 0; i--) {
        uint16_t bit = (bits & (1 << i)) >> i; 

        data_set(bit); // set DATA to bit level
        clk_set(1); // on the rising CLK edge shift register loads the value from DATA
        data_set(0); // return data to the dafault state
        clk_set(0);  // return CLK to the default state

    }
}

void send_serial_delay(uint16_t bits, int del) {
    bits = ~bits;
    for (int8_t i = 15; i >= 0; i--) {
        /* get the bit to send here */
        uint16_t bit =  (bits & (1 << i)) >> i; 

        data_set(bit); // set DATA to bit level
        //delay(del);
        clk_set(1); // on the rising CLK edge shift register loads the value from DATA
        //delay(del);
        data_set(0); // return data to the dafault state
        //delay(del);
        clk_set(0);  // return CLK to the default state
        //delay(del);

    }
}

void main() {
    setup();
    
    GPIOA_ODR |= 0x6; // transistors ON

    send_serial((uint16_t)DISPLAY_OFF);

    delay(2000);
    for (int i = 1; i < 10; i++) {

        uint16_t value = (digits[i - 1] << 8) | digits[i];

        send_serial_delay(value, 1);
        delay(1000);
    }
}
