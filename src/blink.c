#include "delay.h"
#include "stm32f1.h"
#include <stdint.h>

#define HIGH 1
#define LOW 0

#define ONE_SECOND 250 // depands on the values used in the delay. Currently 4ms are used in total => 1 sec / 4 ms = 250
#define RAW_READING_COUNTER 10

#define DISPLAY_OFF 0xFFFF

typedef enum EVENT {
    ADD,
    SUB,
    NO_EVENT
} EVENT;

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

void show_time(int time_set, int time_left)
{
    uint16_t value = 0;
    GPIOA_ODR &= ~0x4; // turn the left displays off
    // activate left displays [PA1]
    GPIOA_ODR |= 0x2;
    value = (digits[time_set / 10] << 8) | digits[time_left / 10]; // construct the value: zero << 8 + one
    send_serial(value); // send the data

    // wait some 1-2 ms
    delay(2);

    GPIOA_ODR &= ~0x2; // turn the left displays off
    // activate right displays [PA2]
    GPIOA_ODR |= 0x4;
    value = (digits[time_set % 10] << 8) | digits[time_left % 10]; // construct the value: zero << 8 + six
    send_serial(value); // send the data

    delay(2);
}

void setup() {
    // enable clock for ports A, B and C
    RCC_APB2ENR |= RCC_IOPCEN | RCC_IOPAEN | RCC_IOPBEN; 
    
    // clear configuration registers
    GPIOC_CRL &= ~0xff;  // clear PC0, PC1
    GPIOC_CRH &= ~0xff;   // clear PC8, 9
    GPIOB_CRL &= ~0xfff; // clear PB0, PB1, PB2
    GPIOA_CRL &= ~(0xfff); // clear config for PA0, PA1, PA2

    GPIOC_CRH |= 0x11;  // PC8 output, 9
    GPIOC_CRL |= 0x11; // PC0 (DATA) and PC1 (CLK)

    //GPIOB_CRL |= 0x888; // PB0 (+ button), PB1 (- button) and PB2 (conf button)
    GPIOB_CRL |= 0x888; // PB0 (+ button), PB1 (- button) and PB2 (conf button)

    GPIOA_CRL |= 0x118; // PA0 (Start button), PA1 (left display activation), PA2 (right display activation)

    send_serial((uint16_t)DISPLAY_OFF);

}

void main() {
    setup();

    int running = 0;
    
    int time_set = 10;
    int time_left = 10;
    int time_unset = 10;

    int raw_reading = 0;
    int debounced = -1;
    int raw_counter = RAW_READING_COUNTER;
    int handled = 0;

    int indicator_timer = 50;
    int indicate = 0;

    uint16_t value = 0;

    int timer = 0;
    int press_timer = 0;

    EVENT event = NO_EVENT;

    while (1)
    {
        // buttons
        // only one button can be pressed at the time, so if the button is pressed, we will get the value 1
        // PB0 - "+", PB1 - "-", PB2 - "CONF", PA0 - "START"
        raw_reading = GPIOA_IDR & 0x1 | GPIOB_IDR & 0x1 | (GPIOB_IDR & 0x2) >> 1 | (GPIOB_IDR & 0x4) >> 2;

        if (raw_reading == HIGH)
        {
            if (debounced == 1 && !handled)
            {
                GPIOC_ODR |= 0x100;
                press_timer++;

                if (running)
                {
                    // if PA0 is HIGH
                    if ((GPIOA_IDR & 0x1) == HIGH)
                    {
                        time_left = time_set;
                        timer = 0;
                        handled = 1;
                    }
                }
                else
                {
                    // if PA0 is HIGH
                    if ((GPIOA_IDR & 0x1) == HIGH)
                    {
                        running = 1;
                        handled = 1;

                    }
                    // if PB0 is HIGH
                    if ((GPIOB_IDR & 0x1) == HIGH)
                    {
                        //if (time_set < 99)
                          //  time_unset++;
                        event = ADD;
                    }
                    // PB1 is HIGH
                    if (((GPIOB_IDR & 0x2) >> 1) == HIGH)
                    {
                        event = SUB;
                        //if (time_set > 0)
                          //  time_unset--;
                    }
                    // if PB2 is HIGH
                    if (((GPIOB_IDR & 0x4) >> 2) == HIGH)
                    {
                        time_set = time_unset;
                        time_left = time_set;
                        handled = 1;
                    }                    
                }
                
            }
            else
            {
                GPIOC_ODR &= ~0x100;
                press_timer = 0;
            }

            if (raw_counter > 0)
            {
                raw_counter--;
            }
            else
            {
                raw_counter = RAW_READING_COUNTER;
                debounced = raw_reading;
            }
            
        }
        else
        {
            if (event != NO_EVENT)
            {
                int value = press_timer > 250 ? 5 : 1;

                if (event == ADD)
                {
                    if (time_set + value <= 99)
                        time_unset += value;
                }
                else
                {
                    if (time_set - value >= 0)
                        time_unset -= value;
                }

                event = NO_EVENT;
            }

            handled = 0;
            press_timer = 0;
            raw_counter = RAW_READING_COUNTER;


        }

        // activate displays and send data

        if (running)
        {
            GPIOC_ODR |= 0x100;

            if (time_left < 4)
            {
                if (indicator_timer == 0)
                {
                    indicate = !indicate;
                    indicator_timer = 50;
                }
                indicator_timer--;
            }

            if (indicate)
            {
                GPIOC_ODR |= 0x200;
            }
            else
            {
                GPIOC_ODR &= ~0x200;
            }

            show_time(time_set, time_left);

            // decrease timer
            if (timer > 0)
            {
                timer--;
            }
            else
            {
                if (time_left == 0)
                {
                    running = 0;
                    time_left = time_set;
                    time_unset = time_set;
                    indicate = 0;
                    indicator_timer = 50;
                }
                else
                {
                    timer = ONE_SECOND;
                    time_left--;
                }
            }
        }
        else
        {
            GPIOC_ODR &= ~0x100;
            show_time(time_set, time_unset);
        }

    }
}
