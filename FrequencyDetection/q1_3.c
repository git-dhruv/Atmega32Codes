/* Name: q1_3.c
 * Author: Dhruv Parikh
 * Copyright: Copyright (C) 2022 All rights reserved
 * License: GNU license
 * For Q2.1.3
*/

#include "MEAM_general.h"  // includes the resources included in the MEAM_general.h file
#include "m_usb.h"

//Look below for definitions
void delay_wrapper(float);
void waitforpress(void);
float convert_counter_to_seconds(int);

//Global vars for storing vals
unsigned int oldtime, tperiod, state=1;

int main(){
    _clockdivide(0); //Set the system prescaler
    m_usb_init();
    
    //Prescaler to 1024
    set(TCCR3B,CS30);
    clear(TCCR3B,CS31);
    set(TCCR3B,CS32);

    //set(PORTC,7); // External pullup already connected

    while(1) {
        m_usb_tx_string("Time taken ");
        waitforpress(); 
    }
}

float convert_counter_to_seconds(int count){
    /*
    * !!Warning!! 
        This function expects prescalar of 1024!
    * @input:
    * Count - integer of timer counts
    * @returns:
    * float var of time taken in ms
    */

    //Returning ms as I am unable to print high precision decimals
    return (float)count*0.000064251*1000; 
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

void waitforpress(void) {
    /*
    * We have to get time between pressing button and 
    * releasing it. So it is a signal of High and low ----____
    * We measure t1 when we get a low flag and then state a flag that we have measured.
    * Then the moment it becomes low, we can calculate for how much time the bit was low. 
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
    state = 0;  //reset the state
    tperiod = ICR3-oldtime;
    usb_tx_decimal(convert_counter_to_seconds(tperiod));
    m_usb_tx_string("ms \n ");
}