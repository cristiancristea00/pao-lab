config WDTE = OFF
config CP = OFF
config MCLRE = OFF

    
#include <xc.inc>


#define COUNT                ( 0x10 )
#define DELAY_COUNT          ( 0x11 )

#define BASE_DELAY           ( 4 )
 
#define PULSE_2_MS           ( 100 )
#define PULSE_0_DOT_5_MS     ( 22 )

#define DELAY_27_DOT_8_MS    ( 39 )
#define DELAY_1_DOT_5_MS     ( 244 )
#define DELAY_3_DOT_5_MS     ( 229 )
#define DELAY_32_DOT_5_MS    ( 10 )
  
    
PSECT  code, global, class=CODE, abs, ovrld, delta=2, keep
    
ORG  0x00

    
main:
    TRIS GPIO
    MOVLW 0xC6
    OPTION
loop:
    MOVLW PULSE_2_MS
    CALL pulse
    
    CALL delay
    
    CALL pulse
    
    MOVLW DELAY_1_DOT_5_MS
    CALL delay
    
    CALL pulse
    
    MOVLW DELAY_3_DOT_5_MS
    CALL delay
    
    CALL pulse
    
    MOVLW DELAY_32_DOT_5_MS
    CALL delay
    
    MOVLW DELAY_32_DOT_5_MS
    CALL delay

    GOTO loop
    
    
delay:
    MOVWF TMR0
delay_step:
    MOVF TMR0, W
    BTFSC STATUS, STATUS_Z_POSITION
    RETLW PULSE_0_DOT_5_MS
    GOTO delay_step
    
    
pulse:
    MOVWF COUNT

next_pulse:
    DECF GPIO, F
    
    MOVLW BASE_DELAY
    MOVWF DELAY_COUNT
stall:
    DECFSZ DELAY_COUNT, F
    GOTO stall
    
    INCF GPIO, F
    
    DECF COUNT, F
    BTFSC STATUS, STATUS_Z_POSITION
    RETLW DELAY_27_DOT_8_MS
    GOTO next_pulse