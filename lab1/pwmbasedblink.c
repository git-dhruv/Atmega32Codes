/* Name: main.c
 * Author: Dhruv Parikh
 * Copyright: Copyright (C) 2022 All rights reserved
 * License: GNU license
 */
#include "MEAM_general.h" // includes the resources included in the MEAM_general.h file
void delay_wrapper(float);
int main()
{
    float duty_cycle = 0;
    duty_cycle = 100 - duty_cycle;
    _clockdivide(0); // Set the system prescaler
    DDRB |= 0x20;    // Setting the Pin to output mode
    clear(PORTB, 5);
    /*
    Since we are using a 16 bit timer -> 65,500 bits,
    16 Mhz has to be scaled down to 65kHz to get a 1-1 ratio
    So 16*10^6/65*10^3 = 246 hz which is close to 256 Hz
    */
    set(TCCR1B, CS12); // Prescaling by 256 Hz
    // Adjusting total time to account for varying brightness
    int total_time = (16e6 / 256) * 10e-3; // 16Mhz/256hz = 62500
    ICR1 = total_time;                     // Setting ICR1 to total time, so the waveform has a timeperiod of "total time"
    OCR1A = (total_time * duty_cycle * 0.01);
    set(TCCR1A, COM1A1); // clear at OCR1A, set at roll over
    set(TCCR1A, COM1A0); // clear at OCR1A, set at roll over

    // Below is the waveform generation for ICR1
    set(TCCR1B, WGM13);
    set(TCCR1B, WGM12);
    set(TCCR1A, WGM11);
    // Number of steps - more steps = more smoothing
    int steps = 50;
    // Time required to reach HIGH volt routine
    float higher_lim_time = 0.3;
    // Time required to reach LOW volt
    float lower_lim_time = 0.7;
    // Here is our blinking!
    while (1)
    {
        // High cycle
        for (int i = 0; i <= steps; i++)
        {
            // I am using inverse duty cycle for my convinience.
            //  Basically duty cycle = i/steps
            duty_cycle = i * 100 / steps;
            duty_cycle = 100 - duty_cycle;
            // Set the OCR1A to cutoff at that duty cycle
            OCR1A = (total_time * duty_cycle * 0.01);
            // Delay wrapper according to the timestep
            delay_wrapper(1000 * higher_lim_time / steps);
        }
        // low cycle
        for (int i = steps; i >= 0; i--)
        {
            // Inverse duty cycle
            duty_cycle = i * 100 / steps;
            duty_cycle = 100 - duty_cycle;
            // OCR1A Cutoff
            OCR1A = (total_time * duty_cycle * 0.01);
            // Delay wrapper
            delay_wrapper(1000 * lower_lim_time / steps);
        }
        // Delay to differentiate cycles
        delay_wrapper(2000);
    }
    return 0;
}
void delay_wrapper(float delay)
{
    /*
    Wraps the default _delay_ms function
    @Parameters:
    @Input: Float Delay (in ms)
    @Output: NULL
    */
    /*
    If you dont want any delay, exit immediately.
    This is useful for Duty cycle problem else the LED keeps glowing (extremely dim)
    */
    if (delay == 0)
    {
        return;
    }
    13 MEAM 5100 - lab 1 Dhruv Parikh—dhruvkp @seas.upenn.edu
                   // Starting from one since the delay of less than 1ms has to be ignored.
                   for (int i = 0; i < delay; i++)
    {
        _delay_ms(1); // keep delaying 1 ms for the amount of milliseconds
    }
}
