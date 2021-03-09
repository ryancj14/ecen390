/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef LEDTIMER_H_
#define LEDTIMER_H_

#include "mio.h"
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include <stdio.h>

#include "ledTimer.h"

#define LEDTIMER_HIGH_VALUE 1
#define LEDTIMER_LOW_VALUE 0
#define LEDTIMER_OUTPUT_PIN 11    


#define INIT_ST_MSG "Init state\n"
#define WAIT_ST_MSG "Wait state\n"
#define LIT_ST_MSG "Lit state\n"
#define ERROR_MSG "LED Timer Error occurred\n"

// Provides an led timer with adjustable frequency and adjustable on/off times.
// All time-based numbers are expressed in milliseconds.
// Minimum period is 1 millisecond.

// Controls both hitLed and JF-10. Make sure to disable the hitLedTimer when
// using this timer.
#define LED_TIMER_LED_PIN 15 // This is the MIO pin number.
#define LED_CYCLES 50000 //half a second

static bool debugPrint;
volatile static bool timerStartFlag = false;
uint32_t static cycleCounter;
enum ledTimer_st_t {
    init_st,
	wait_st,
    lit_st
} led_currentState;

// Initialize the ledTimer before you use it.
void ledTimer_init() {
  mio_init(false);
led_currentState = init_st;
cycleCounter = 0;
mio_setPinAsOutput(LEDTIMER_OUTPUT_PIN);
}

void ledTimer_debugStatePrint() {
  static enum ledTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != led_currentState - this prevents reprinting the same state name over and over.
  if (previousState != led_currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = led_currentState;     // keep track of the last state that you were in.
    switch(led_currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf(INIT_ST_MSG);
        break;
      case wait_st:
        printf(WAIT_ST_MSG);
        break;
      case lit_st:
        printf(LIT_ST_MSG);
        break;
     }
  }
}

// Starts the ledTimer running.
void ledTimer_start() {
    timerStartFlag = true;
}

// Returns true if the timer is currently running, false otherwise.
bool ledTimer_isRunning() {
    return timerStartFlag;
}

// Terminates operation of the ledTimer.
void ledTimer_stop() {
    timerStartFlag = false;
}

// Specfies how long the LED is on in milliseconds.
void ledTimer_setOnTimeInMs(uint32_t milliseconds) {

}

// Specfies the period of the ledTimer.
void ledTimer_setPeriodInMs(uint32_t milliseconds) {

}

// Specified the number of ticks per millisecond for the tick function.
// E.G., if ticksPerMillisecond is 10, the tick function will be called 10 times
// each millisecond.
void ledTimer_setTicksPerMs(uint32_t ticksPerMillisecond) {

}

// Invoking this causes the led timer to also control the hit-LED.
// Flag = true means that the ledTimer will control the hitLed.
// Flag = false means that the ledTimer does not control the hitLed.
void ledTimer_controlHitLed(bool flag) {

}

// Standard state-machine tick function. Call this to advance the state machine.
void ledTimer_tick() {
  if(debugPrint)
    ledTimer_debugStatePrint();
switch(led_currentState) {
      case init_st:
        led_currentState = wait_st;
        break;
      case wait_st:
        if(timerStartFlag) {
            cycleCounter = 0;
            led_currentState = lit_st;
            mio_writePin(LEDTIMER_OUTPUT_PIN, LEDTIMER_HIGH_VALUE); // Write a '1' to JF-1.
        }
        else {
            led_currentState = wait_st;
        }
        break;
      case lit_st:
        if(cycleCounter > LED_CYCLES) {
            cycleCounter = 0;
            led_currentState = wait_st;
            mio_writePin(LEDTIMER_OUTPUT_PIN, LEDTIMER_LOW_VALUE); // Write a '1' to JF-1.
        }
        else {
            led_currentState = lit_st;
        }
        break;
    default:
        printf(ERROR_MSG);
        printf("Current state in LED: %d\n", led_currentState);
      // print an error message here.
      break;
  }
  
  // Perform state action next.
  switch(led_currentState) {
      case init_st:
        break;
      case wait_st:
        break;
      case lit_st:

        cycleCounter++;
        break;
    default:
        printf(ERROR_MSG);
      // print an error message here.
      break;
  }
}

// Standard test function.
void ledTimer_runTest() {
  debugPrint = true;
while(true) {
    ledTimer_start();
        while(ledTimer_isRunning()) {
            
        }
    utils_msDelay(300);
    }
}

// Debug function.
void ledTimer_dumpDebugValues() {

}

#endif /* LEDTIMER_H_ */
