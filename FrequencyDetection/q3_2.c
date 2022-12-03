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
    
    //Prescaler to 1024
    set(TCCR3B,CS30);
    clear(TCCR3B,CS31);
    set(TCCR3B,CS32);

    //set(PORTC,7); // External pullup already connected so don't need it

    //Set PORTB5 AND 6 as Output in driver
    set(DDRB,5);
    set(DDRB,6);
    float hz = 0; //Frequency



    while(1) {
        //We can use the waitforpress subroutine here
        //Since we are just detecting the time between a high signal and low signal
        //It will function exactly as intended for us here
        hz = waitforpress(); 

        //If hz between 10 and 30 Hz then assume its 23 hz frequency
        if(hz<30 && hz>10){
            set(PORTB,5);
            clear(PORTB,6);

        }

        //If greater than 650 hz, then assume its 700 hz
        else if(hz>650){
            set(PORTB,6);
            clear(PORTB,5);
        }
        
        else{
            clear(PORTB,5);
            clear(PORTB,6);
        }
        //Ideally we don't want any delay but for debugging and knowing the LOS I have kept this
        usb_tx_decimal(hz);
        m_usb_tx_string("hz \n ");
    
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
    return (float)count*0.000064251*1000; //For some reason I am unable to print high precision decimals
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
    *   Frequency between press
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
    tperiod = ICR3-oldtime; //Get time period
    float fz = 1000/(float)convert_counter_to_seconds(tperiod); //Frequency = (1/t)*1000 ms
    fz = 0.5*fz;    //Halve the frequency to get accurate hz

    return fz;
}