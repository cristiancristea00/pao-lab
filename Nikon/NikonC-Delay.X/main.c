#pragma config WDTE = OFF
#pragma config CP = OFF
#pragma config MCLRE = OFF

#define _XTAL_FREQ    4000000UL

#include <xc.h>
#include <stdbool.h>


void delay(uint8_t count)
{
step:
    __delay_us(100);

    if (--count == 0)
    {
        goto step;
    }
}

void pulse(uint8_t count)
{
wave:
    asm("DECF GPIO, 1"); // 0x00 -> 0xFF
    __delay_us(13);
    asm("INCF GPIO, 1"); // 0xFF -> 0x00

    if (--count == 0)
    {
        goto wave;
    }
}


void main(void)
{
    // Reduced 3 -> 2 instructions
    asm ("CLRF GPIO");
    asm ("TRIS GPIO");
    
    while (true)
    {
        pulse(83);
        delay(28);
        pulse(21);
        delay(15);
        pulse(21);
        delay(35);
        pulse(21);
        
        delay(63);
    }
}