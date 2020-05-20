/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "c:/work/Penta/Pent4silea/src/Pent4silea.ino"
//
// Pentsilea.iso: main application
//
// Author: A.Navatta

// This #include statement was automatically added by the Particle IDE.
#include <application.h>

#include <Led.hpp>
#include <Telemetry.hpp>
#include <CloudAPI.hpp>

void setup();
void loop();
#line 13 "c:/work/Penta/Pent4silea/src/Pent4silea.ino"
Led led;

//int dvalue = 0;
//unsigned long long last = 0;
//unsigned long long current = 0;

#define MAIN_THREAD_SLEEP_TIME  250  // 250 ms

void setup()
{
    // sleep 2 seconds
    delay(2000);

    cloudapi.init(false);

    led.init(true);
}

int counter = 0;

// Last time, we wanted to continously blink the LED on and off
// Since we're waiting for input through the cloud this time,
// we don't actually need to put anything in the loop
void loop()
{
    C_DEBUG("loop in(%d)", counter);
    led.toggle();

    cloudapi.poll();
 
    C_DEBUG("loop out(%d)", counter);
    counter++;
    delay(MAIN_THREAD_SLEEP_TIME);
}