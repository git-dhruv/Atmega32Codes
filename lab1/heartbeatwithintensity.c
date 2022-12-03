/* Name: main.c
 * Author: Dhruv Parikh
 * Copyright: Copyright (C) 2022 All rights reserved
 * License: GNU license
 */
#include "MEAM_general.h" // includes the resources included in the MEAM_general.h file
void delay_wrapper(float);
#define COMPARE_VAL 6250 // Comparision Value for the TCNT1 register
void pwm_gen(float, float, float, float);
int main()
{
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
    OCR1A = (0);                           // Setting the OCR1A = 0 for reset before we start toggling
    set(TCCR1A, COM1A1);                   // clear at OCR1A, set at roll over
    set(TCCR1A, COM1A0);                   // clear at OCR1A, set at roll over
    // Below is the waveform generation for ICR1
    set(TCCR1B, WGM13);
    set(TCCR1B, WGM12);
    set(TCCR1A, WGM11);
    while (1)
    {
        // Brightness function!
        /*
        The below lines are just going from low_val to high_val in time indicated in the question.
        */
        pwm_gen(0, 100, 0.1, total_time);
        pwm_gen(100, 0, 0.4, total_time);
        pwm_gen(0, 50, 0.1, total_time);
        pwm_gen(50, 0, 0.4, total_time);
        delay_wrapper(2000);
        pwm_gen(0, 100, 0.1, total_time);

        pwm_gen(100, 0, 0.4, total_time);
        pwm_gen(0, 50, 0.1, total_time);
        pwm_gen(50, 0, 0.4, total_time);
        delay_wrapper(2000);
    }
    return 0;
}
void pwm_gen(float from_val, float to_val, float time, float total_time)
{
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
    float duty_cycle = 0;
    duty_cycle = 100 - duty_cycle;
    /*
    Increment step - less steps = more smoothing
    Increment is calculated based on 10 ms of delay in each step
    10 ms * (Total Value Change/ Total time for waveform) = time delay for each step
    */
    float increment = 10 * (to_val - from_val) * 0.001 / time;
    if (from_val < to_val)
    {
        // High cycle
        float current_duty_cycle = from_val;
        while (current_duty_cycle <= to_val)
        {
            // I am using inverse duty cycle for my convinience.
            duty_cycle = current_duty_cycle;
            duty_cycle = 100 - duty_cycle;
            // Set the OCR1A to cutoff at that duty cycle
            OCR1A = (total_time * duty_cycle * 0.01);
            // Delay wrapper according to the timestep
            delay_wrapper(10);
            // Increment the duty cycle
            current_duty_cycle += increment;
        }
    }
    else
    {
        // Low cycle
        float current_duty_cycle = from_val; // Reset the duty cycle to from_val
        while (current_duty_cycle >= to_val)
        {
            // I am using inverse duty cycle for my convinience.
            duty_cycle = current_duty_cycle;
            duty_cycle = 100 - duty_cycle;
            // Set the OCR1A to cutoff at that duty cycle
            OCR1A = (total_time * duty_cycle * 0.01);
            // Delay wrapper according to the timestep
            delay_wrapper(10);
            // Keep decrementing (increment in this case will be negative)
            current_duty_cycle += increment;
        }
    }
}
void delay_wrapper(float delay)
{
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
    if (delay == 0)
    {
        return;
    }
    // Starting from one since the delay of less than 1ms has to be ignored.
    for (int i = 0; i < delay; i++)
    {
        _delay_ms(1); // keep delaying 1 ms for the amount of milliseconds
    }
}