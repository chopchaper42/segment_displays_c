#include "delay.h"
#include "stm32f1.h"
#include <stdint.h>

#define  ZERO 0b11111100
#define   ONE 0b11000000
#define   TWO 0b10110101
#define THREE 0b11010101
#define  FOUR 0b11001001
#define  FIVE 0b01011101
#define   SIX 0b01111101
#define SEVEN 0b11000100
#define EIGHT 0b11111101
#define  NINE 0b11011101

#define DELAY_TIME 500

uint8_t digits[] = {0b11111100, 0b11000000,
                    0b10110101, 0b11010101,
                    0b11001001, 0b01011101,
                    0b01111101, 0b11000100,
                    0b11111101, 0b11011101};

int get_nth_bit(volatile unsigned *addr, int bit_number) {
    return *addr & (1 << bit_number) >> bit_number;
}

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

// port A from PA2 doesn't work
void setup() {
    // enable clock for ports A, B and C
    RCC_APB2ENR |= RCC_IOPCEN | RCC_IOPAEN | RCC_IOPBEN; 
    
    // clear configuration registers
    GPIOC_CRL &= ~0xff;
    GPIOC_CRH &= ~0xff;
    GPIOB_CRL &= ~0xff;
    GPIOA_CRL &= ~0xfff; // clear config for PA0, PA1, PA2

    GPIOC_CRH |= 0x1;  // PC8 output
    GPIOC_CRL |= 0x11; // PC0 (DATA) and PC1 (CLK)

    GPIOB_CRL |= 0x888; // PB0 (+ button), PB1 (- button) and PB2 (conf button)

    GPIOA_CRL |= 0x118; // PA0 (Start button), PA1 (left display activation), PA2 (right display activation)

}

void loop() {
    int btn_pressed_counter = 0;
    int plus_btn = 0;
    int minus_btn = 0;
    int confirm_btn = 0;


}

void set_clock(LOGICAL_LEVEL level) {
    if (level == LOW) {
        GPIOB_ODR &= ~0x200;
    } else {
        GPIOB_ODR |= 0x200;
    }

}

int toggle_led() {
    GPIOC_ODR ^= 0x100;
    //return ((GPIOC_ODR & (1 << 7)) >> 7);
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
    for (int8_t i = 15; i >= 0; i--) {
        /* get the bit to send here */
        uint16_t bit =  (bits & (1 << i)) >> i; 

        data_set(bit); // set DATA to bit level
        delay(DELAY_TIME);
        clk_set(1); // on the rising CLK edge shift register loads the value from DATA
        delay(DELAY_TIME);
        data_set(0); // return data to the dafault state
        delay(DELAY_TIME);
        clk_set(0);  // return CLK to the default state
        delay(DELAY_TIME);

    }
}

void send_serial_delay(uint16_t bits, int del) {
    for (int8_t i = 15; i >= 0; i--) {
        /* get the bit to send here */
        uint16_t bit =  (bits & (1 << i)) >> i; 

        data_set(bit); // set DATA to bit level
        delay(del);
        clk_set(1); // on the rising CLK edge shift register loads the value from DATA
        delay(del);
        data_set(0); // return data to the dafault state
        delay(del);
        clk_set(0);  // return CLK to the default state
        delay(del);

    }
}

void main() {
    setup();

    GPIOA_ODR |= 0x6; // transistors ON

    send_serial_delay((uint16_t)0xFFFF, 5);

    // the code below WORKS
    // GPIOC_ODR |= 0x1; // PC0 on
    // GPIOC_ODR |= 0x2; // PC1 on
    //while (1)
    //{
        delay(1000);

        uint16_t value = (SEVEN << 8 | NINE);
        send_serial_delay(~value, 10);
        /* for (int i = 0; i < 10; i++) {
            uint16_t value = ((uint16_t)0) | digits[i];

            send_serial(value);
        }

        delay(3000); */

    //}

        /* uint16_t null_byte = 0;

        send_serial(null_byte);

        uint16_t byte = 0xF0F0; //0b1111111100000000;
        //uint16_t byte = 0b1111111100000000;

        delay(2000);

        send_serial(byte);

        delay(2000); */
    //}
    /* GPIOC_ODR |= 1 << 8;

    delay(2000);

    byte = 0b11000110;

    send_serial(byte); */

    /* while (1)
    {

        uint8_t byte = 0b11111111;
        send_serial(byte);

        delay(1000); */

        //send_serial(0b10101010);

        //delay(3000);
        
        /* GPIOC_ODR |= 0x1;  // PC0 HIGH
        GPIOC_ODR |= 0x2;  // PC1 HIGH

        delay(1000);

        GPIOC_ODR &= ~0x2; // PC1 LOW
        GPIOC_ODR &= ~0x1; // PC0 LOW

        delay(1000); */

        /* toggle_led();
        
        // turn left on, turn right off
        GPIOA_ODR |= 0x4;
        GPIOA_ODR &= ~0x2;

        delay(2);

        toggle_led();
        GPIOA_ODR |= 0x2;
        GPIOA_ODR &= ~0x4;

        delay(2); */

        /*
        
            * buttons part *

            deactivate right digits
            activate left digits
            send left digits
        
            dalay 2ms

            deactivate left digits
            activate right digits
            send right digits

            delay 2ms
        
        */
    //}
}
