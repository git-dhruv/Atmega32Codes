/* Name: main.c
 * Author: Dhruv Ketan Parikh
 * Copyright: Copyright (C) 2022 All rights reserved
 * License: GNU license
 * Lab: 3.1 Waldo Input
*/

#include "MEAM_general.h"  // includes the resources included in the MEAM_general.h file
#include "m_usb.h"

//Function defs inside the block
void delay_wrapper(float);
int analog_read(void);  //ADC reader
void setup_adc(int);    //ADC port setup
float multiple_adc(int *, int *);   //Function to read multiple ADC values

void pwm_gen(float,float,int); //PWM Generator
float map(float,float,float,float,float); //Map
float constrain(float,float,float);
float expsmoothing(float,float,float);


int main(){
    _clockdivide(0); //Set the system prescaler
    m_usb_init();

    int ans = 0;
    //Port that you want to read
    int port[2] = {7,0}; //A0,A5

    //Index tracker
    int active = 0;

    float arm_pwm = 0, base_pwm = 0;


    //PWM SERVO
    DDRB |= 0x20;   //Base
    set(DDRB,6);    //ARM
    clear(PORTB,5);
    clear(PORTB,6);
    
    /*
        Since we are using a 16 bit timer -> 65,500 bits, 
        16 Mhz has to be scaled down to 65kHz to get a 1-1 ratio
        So 16*10^6/65*10^3 = 246 hz which is close to 256 Hz
    */
    set(TCCR1B,CS12); //Prescaling by 256 Hz
    
    //Currently we dont have any system prescaler

    //Adjusting total time to account for varying brightness
    int total_time = 2*(16e6/256)*10e-3; //16Mhz/256hz = 62500
    ICR1  = total_time; // Setting ICR1 to total time, so the waveform has a timeperiod of "total time"
    
    OCR1A = (0); //Setting the OCR1A = 0 for reset before we start toggling
    set(TCCR1A,COM1A1); // clear at OCR1A, set at roll over 
    set(TCCR1A,COM1A0); // clear at OCR1A, set at roll over 

    //Below is the waveform generation for ICR1
    set(TCCR1B,WGM13);   
    set(TCCR1B,WGM12);  
    set(TCCR1A,WGM11);  


    //For timer B
    OCR1B = 0;
    set(TCCR1A,COM1B1); // clear at OCR1A, set at roll over 
    set(TCCR1A,COM1B0); // clear at OCR1A, set at roll over 



    //These are debugging and testing purpose - kept in code for future reference
    //3.5 is 90 degs
    //8 is 0 degs
    //10 - 90 degs
    float angle = 0;
    //max_angle_dc*0.50; //50 = 270/50 =54 degs
    float what_i_want = map(angle,0,200,15,3);


    while(1){
        
        ans = multiple_adc(port, &active); 
    
        //Calculation - You can set the angle whatever you want - I want 180 degrees
        
        ans = ans*((float)180/(float)1023); 
        if(ans>200){
            ans = 200;
        }
        
        //If Base readings are provided from adc
        if(active==0){
            // Two steps happen here
            // 1.  Mapping from 0,180 to 15,3 duty cycle
            // 2. Exponential Smoothing for this new value
            base_pwm =  expsmoothing(map(ans,0,180,15,3),base_pwm,0.7);
            //Constrain the pwm so the motors don't break
            base_pwm = constrain(base_pwm,15,3);
            //Generat the pwm
            pwm_gen(base_pwm,total_time,active);
        }
        //Same as above for arm
        else{
            arm_pwm =  expsmoothing(map(ans,0,180,15,3),arm_pwm,0.7);
            arm_pwm = constrain(arm_pwm,15,3); 
            pwm_gen(arm_pwm,total_time,active);
        
        }

        
    }


}

float multiple_adc(int *port, int *index){
    /*
    Mutiple ADC reader one by one. 
    At each call of the function, port is incremented by one.

    @Input: 
    (int *) port: Array of ports that you want to scan
    (int *) index: This is a helper that lets you keep track of index - make sure that you don't get random values
    @Output: (float) Value of ADC
    */

    static int active; //Port tracker

    size_t no_of_ports = sizeof(port) / sizeof(int);

    active++;   //Increment the port

    //Saturate at no_of_ports
    if(active>(int)no_of_ports){
        active = 0;
    }

    //Let user get the reference
    *index = active;

    //Setup that port
    setup_adc(port[active]);

    //Read ADC
    float ans = analog_read();

    return ans;
}

void setup_adc(int portno){
    /*
    * @Input: Integer for portno
    * Description: Based on bitmask, setup the ADC
    */

    //Let's not blow up stuff
    if(portno==2 || portno==3){return;}

    //Defined all the bitmask here for multiplexers
    int bin_mux2[14] = {0,0,0,0,1,1,1,1,0,0,0,0,1,1};
    int bin_mux1[14] = {0,0,0,0,0,0,1,1,0,0,1,1,0,0};
    int bin_mux0[14] = {0,1,0,0,0,1,0,1,0,1,0,1,0,1};

    //Voltage reference as Vcc
    clear(ADMUX,REFS1);
    set(ADMUX,REFS0);

    //ADC Prescalar
    set(ADCSRA, ADPS0); 
    set(ADCSRA, ADPS1); 
    set(ADCSRA, ADPS2); 
    
    if(portno<=7){
        //Disable DIDR
        set(DIDR0, portno);

        //Just clear the ADCSRB HERE
        clear(ADCSRB,MUX5);
    }
    else{
        set(DIDR2, portno-8); //check this

        //Set ADCSRB
        set(ADCSRB,MUX5);
    }

    //Simply check the binaries and set accordingly
    if(bin_mux2[portno]) {set(ADMUX,MUX2);}
    else {clear(ADMUX,MUX2);}

    if(bin_mux1[portno]) {set(ADMUX,MUX1);}
    else {clear(ADMUX,MUX1);}

    if(bin_mux0[portno]) {set(ADMUX,MUX0);}
    else {clear(ADMUX,MUX0);}        
}

int analog_read(void){
    int reading;
    set(ADCSRA,ADEN);
    set(ADCSRA, ADSC); 
    
    while(!bit_is_set(ADCSRA, ADIF)); 

    //Get the ADC Value
    reading = ADC;

    set(ADCSRA, ADIF); 
    return reading;
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



void pwm_gen(float to_val, float total_time, int portno){
    /*
        PWM Generator from a percentage of duty cycle to a percentage of duty cycle
        You need to supply the Time period of the frequency
        @Parameters:
            @input:     
                from_val: float - From a duty cycle (in percentage)
                to_val: float - To a duty cycle (in percentage)
                time: float - Time period to reach that duty cycle (in seconds)
                total_time - Time period of the generated duty cycle (in seconds)
            @output: None
    */
    float duty_cycle = to_val;
    duty_cycle = 100-duty_cycle; 
    //Set the OCR1A to cutoff at that duty cycle
    if(portno==0){
    OCR1A = (total_time*duty_cycle*0.01);
    }
    else{
    OCR1B = (total_time*duty_cycle*0.01);
    }
    }
 
float map(float x, float from_min, float from_max, float to_min, float to_max) {
    /*
    * Function that maps from one range to another.
    * Reference: https://www.arduino.cc/reference/en/language/functions/math/map/
    */
  return (x - from_min) * (to_max - to_min) / (from_max - from_min) + to_min;
}


float expsmoothing(float newval, float oldval, float weight){
    /*
    Low cost exponential smoothing that handles floating point values well. 
    Weight that is passed is for old val so should be greater than 0.6 ideally
    */
    return weight*oldval+(1-weight)*newval;
}

float constrain(float val, float high, float low){
    /*
    Constrains a value between high and low
    */

    if (val>high) val = high;
    if (val<low) val = low;
    return val;
}