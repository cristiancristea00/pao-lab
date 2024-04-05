#include <xc.inc>
    
    
#define RETURN_VAL           ( 0x00 )
    
#define COUNT                ( 0x10 )
#define DELAY_COUNT          ( 0x11 )

#define BASE_DELAY           ( 4 )
 
#define PULSE_2_MS           ( 91 )
#define PULSE_0_DOT_5_MS     ( 22 )

#define DELAY_27_DOT_8_MS    ( 217 )
#define DELAY_1_DOT_5_MS     ( 12 )
#define DELAY_3_DOT_5_MS     ( 27 )
#define DELAY_32_DOT_5_MS    ( 246 )
  
    
PSECT  code, global, class=CODE, abs, ovrld, delta=2, keep
    
ORG  0x00

    
main:
    MOVLW 0x06
    OPTION
    CLRF GPIO
    TRIS GPIO
loop:
    MOVLW PULSE_2_MS
    CALL pulse
    
    MOVLW DELAY_27_DOT_8_MS
    CALL delay
    
    MOVLW PULSE_0_DOT_5_MS
    CALL pulse
    
    MOVLW DELAY_1_DOT_5_MS
    CALL delay
    
    MOVLW PULSE_0_DOT_5_MS
    CALL pulse
    
    MOVLW DELAY_3_DOT_5_MS
    CALL delay
    
    MOVLW PULSE_0_DOT_5_MS
    CALL pulse
    
    MOVLW DELAY_32_DOT_5_MS
    CALL delay
    
    MOVLW DELAY_32_DOT_5_MS
    CALL delay

    GOTO loop
    
    
delay:
    MOVWF COUNT
    CLRF TMR0
delay_step:
    MOVF TMR0, W
    XORWF COUNT, W
    BTFSC STATUS, STATUS_Z_POSITION
    RETLW RETURN_VAL
    
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
    RETLW RETURN_VAL
    GOTO next_pulse