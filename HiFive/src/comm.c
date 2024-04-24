#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "eecs388_lib.h"

void auto_brake(int devid)
{
    uint16_t dist = 0;
    uint16_t dist_L;
    uint16_t dist_H;
    if ('Y' == ser_read(devid) && 'Y' == ser_read(devid)) {
        dist_L = ser_read(devid);
        dist_H = ser_read(devid);

        //shift and bitwise operator, combines the low and high dist into a single 16bit int
        dist = (dist_H << 8) | dist_L;
        //printf("dist: %u\n",dist);
        /*
        Task 2.2:
        - turn on the red LED if the distance measured is less than 50 cm.
        - otherwise turn on the green LED
        - print the measured value to the console (i.e. the serial monitor) 
        - You can use printf() or ser_printline() or ser_write()

        - Helpful guide for printing with printf()
        - https://en.cppreference.com/w/cpp/io/c/fprintf
        */
        if (dist > 200) {
            gpio_write(RED_LED, OFF);
            gpio_write(GREEN_LED, ON);
        } else if ((100 < dist) && (dist <= 200)) {
            gpio_write(RED_LED, ON);
            gpio_write(GREEN_LED, ON);
        } else if ((60 < dist) && (dist <= 100)) {
            gpio_write(RED_LED, ON);
            gpio_write(GREEN_LED, OFF);
        } else if (dist <= 60) {
            gpio_write(GREEN_LED, OFF);
            gpio_write(RED_LED, ON);
            delay(100);
            gpio_write(RED_LED, OFF);
            delay(100);
        }
    }
}


int read_from_pi(int devid)
{
    char ser_bytes[4];
    int final;
    if (ser_isready(devid)) {

        ser_readline(devid, 4, ser_bytes);

        sscanf(ser_bytes, "%d", &final);
        return final;
    }
    return 0;
}

void steering(int gpio, int pos)
{
    // Task-3: 
    // Your code goes here (Use Lab 05 for reference)
    // Check the project document to understand the task
    // Defines servo duty duration from the min and max PWM signal and a half circle in degrees
    // const int duty_duration = SERVO_PULSE_MAX - SERVO_PULSE_MIN;
    // const int halfCircle = 180; 
    // // Defines the proportion of on and off time given the duty cycle duration
    // int on_duration = (duty_duration/halfCircle)*pos; // Finds proportion of duty cycle to be on
    // int off_duration = SERVO_PERIOD - on_duration;
    // // Turns on and off pulse givn proportion of the pos angle and duty cycle duration
    // gpio_write(gpio, ON);
    // delay_usec(on_duration);
    // gpio_write(gpio, OFF);
    // delay_usec(off_duration);
    int duty = 544 + (SERVO_PULSE_MAX - SERVO_PULSE_MIN) * pos / 180;
    gpio_write(gpio, ON);
    delay_usec(duty);
    gpio_write(gpio, OFF);
    delay_usec(SERVO_PERIOD - duty);
    }


int main()
{
    // initialize UART channels
    ser_setup(0); // uart0
    ser_setup(1); // uart1
    int pi_to_hifive = 1; //The connection with Pi uses uart 1
    int lidar_to_hifive = 0; //the lidar uses uart 0
    
    printf("\nUsing UART %d for Pi -> HiFive", pi_to_hifive);
    printf("\nUsing UART %d for Lidar -> HiFive", lidar_to_hifive);
    
    //Initializing PINs
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("Setup completed.\n");
    printf("Begin the main loop.\n");
    int gpio = PIN_19;
    steering(gpio,0);
    
    while (1) {
        auto_brake(lidar_to_hifive); // measuring distance using lidar and braking
        int angle = read_from_pi(pi_to_hifive); //getting turn direction from pi
        if (angle != 0) {
            printf("\nangle=%d", angle);
        }
         
        for (int i = 0; i < 10; i++){
            // Here, we set the angle to 180 if the prediction from the DNN is a positive angle
            // and 0 if the prediction is a negative angle.
            // This is so that it is easier to see the movement of the servo.
            // You are welcome to pass the angle values directly to the steering function.
            // If the servo function is written correctly, it should still work,
            // only the movements of the servo will be more subtle
            if(angle>0){
                steering(gpio, 180);
            }
            else {
                steering(gpio,0);
            }
            
            // Uncomment the line below to see the actual angles on the servo.
            // Remember to comment out the if-else statement above!
            //steering(gpio, angle);
        }

    }
    return 0;
}
