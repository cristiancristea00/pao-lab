#pragma config WDTE = OFF
#pragma config CP = OFF
#pragma config MCLRE = OFF


#define _XTAL_FREQ    ( 4000000UL )


#include <xc.h>
#include <stdbool.h>


#define PULSE_BASE_US        ( 14 )

#define PULSE_2_MS           ( 91 )
#define PULSE_0_DOT_5_MS     ( 22 )

#define DELAY_27_DOT_8_MS    ( 217 )
#define DELAY_1_DOT_5_MS     ( 12 )
#define DELAY_3_DOT_5_MS     ( 27 )
#define DELAY_32_DOT_5_MS    ( 246 )


void delay(uint8_t count);
void pulse(uint8_t count);


void main(void)
{
    OPTION = 0x06;
    asm("CLRF GPIO");
    asm("TRIS GPIO");
    
    while (true)
    {
        /* START COMMAND SIGNAL*/
        pulse(PULSE_2_MS);
        
        delay(DELAY_27_DOT_8_MS); 
        
        pulse(PULSE_0_DOT_5_MS);
        
        delay(DELAY_1_DOT_5_MS);
        
        pulse(PULSE_0_DOT_5_MS);
        
        delay(DELAY_3_DOT_5_MS);
        
        pulse(PULSE_0_DOT_5_MS);
        /* END COMMAND SIGNAL*/
        
        delay(DELAY_32_DOT_5_MS); delay(DELAY_32_DOT_5_MS);
    }
}

void delay(uint8_t count)
{
    asm("CLRF TMR0");
    while (TMR0 != count) { }
}

void pulse(uint8_t count)
{
wave:
    asm("DECF GPIO, 1"); // 0x00 -> 0xFF
    __delay_us(PULSE_BASE_US);
    asm("INCF GPIO, 1"); // 0xFF -> 0x00

    if (--count != 0)
    {
        goto wave;
    }
}