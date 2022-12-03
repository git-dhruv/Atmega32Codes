/* Name: q1_1.c
 * Author: Dhruv Parikh
 * Copyright: Copyright (C) 2022 All rights reserved
 * License: GNU license
 * For Q2.1.1 
 * Write code that will read the state of the switch and make an LED turn on when the switch is depressed and turn off when the switch is not pressed.
*/

#include "MEAM_general.h"  // includes the resources included in the MEAM_general.h file
#include "m_usb.h"

void delay_wrapper(float);

int main(){
    _clockdivide(0); //Set the system prescaler
    m_usb_init(); //Initialize USB c
    
    //PORTB-5 Output
    set(DDRB,5);

    while(1){
        // Because we have pullup in circuit, we have to invert the logic
        if (bit_is_set(PINF,5)) {
            clear(PORTB,5);
        }
        else{
            set(PORTB,5);
        }
        delay_wrapper(100);
    }

}

void delay_wrapper(float delay){
    /*
    Wraps the default _delay_ms function
    @Parameters: 
        Input: Float Delay (in ms)
    @Output: NULL
    */

   /*
    If you dont want any delay, exit immediately. 
    This is useful for Duty cycle problem else the LED keeps glowing (extremely dim)
   */
    if(delay==0){
        return;
    }

    //Starting from one since the delay of less than 1ms has to be ignored. 
    for(int i=0;i<delay;i++){
       _delay_ms(1); //keep delaying 1 ms for the amount of milliseconds
    }
}