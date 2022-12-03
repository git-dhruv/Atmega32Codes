/* Name: main.c
 * Author: Dhruv Parikh
 * Copyright: Copyright (C) 2022 All rights reserved
 * License: GNU license
*/

#include "MEAM_general.h"  // includes the resources included in the MEAM_general.h file
#include "m_usb.h"

void delay_wrapper(float);
float waitforpress(void);
float convert_counter_to_seconds(int);

//Global vars for storing vals
unsigned int oldtime, tperiod, state=1;

int main(){
    _clockdivide(0); //Set the system prescaler
    m_usb_init();
    
    //Prescaler to 256
    clear(TCCR3B,CS30);
    clear(TCCR3B,CS31);
    set(TCCR3B,CS32);

    //set(PORTC,7); // External pullup already connected
    float t = 0.0;
    int count = 0;
    while(1) {
        m_usb_tx_string("Start the game ");

        while(count<5){
        t += waitforpress(); 
        count++;
        }
        count = 0;
        m_usb_tx_string("Average time taken ");
        usb_tx_decimal(t*0.2);
        m_usb_tx_string("\n");

        //Delay to know that we have finished the counting
        delay_wrapper(500);
    }
}

float convert_counter_to_seconds(int count){
    /*
    * !!Warning!! 
        This function expects prescalar of 256!
    * @input:
    * Count - integer of timer counts
    * @returns:
    * float var of time taken in us
    */
    return (float)count*0.016*1000; 
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

float waitforpress(void) {
    /*
    * Function to wait for press
    *
    * @input:
    *   None
    * @output:
    *   None
    */
    //while(open)
    while(!bit_is_set(TIFR3,ICF3)){
        if(state==0){     //if state was closed
            oldtime = ICR3;
            state = 1; //change the flag
        }        
    }     // check input capture flag 
    set (TIFR3, ICF3); // clear flag by writing 1 to flag (!)
    state = 0;  //
    tperiod = ICR3-oldtime;

    return convert_counter_to_seconds(tperiod);
    // usb_tx_decimal(convert_counter_to_seconds(tperiod));
    // m_usb_tx_string("ms \n ");
}
