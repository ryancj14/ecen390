/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef LOCKOUTTIMER_H_
#define LOCKOUTTIMER_H_
#include <stdbool.h>
#include <time.h>
#include "utils.h"

#include "lockoutTimer.h"

#define LED_CYCLES 50000 //half a second
uint32_t static cycleCounter;

#define LOCKOUT_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.

enum lockoutTimer_st_t {
    init_st,
	wait_st,
    run_st
} currentState;

volatile static bool timerStartFlag = false;

void lockoutTimer_debugStatePrint() {
  static enum lockoutTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf(INIT_ST_MSG);
        break;
      case wait_st:
        printf(WAIT_ST_MSG);
        break;
      case run_st:
        printf(RUN_ST_MSG);
        break;
     }
  }
}
// Calling this starts the timer.
void lockoutTimer_start() {
    timerStartFlag = true;
}

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
    timerStartFlag = false;
    cycleCounter = 0;
    currentState = init_st;
}

// Returns true if the timer is running.
bool lockoutTimer_running() {
    return timerStartFlag;
}

// Standard tick function.
void lockoutTimer_tick() {
switch(currentState) {
      case init_st:
        currentState = wait_st;
        break;
      case wait_st:
        if(timerStartFlag) {
            cycleCounter = 0;
            currentState = run_st;
        }
        else {
            currentState = wait_st;
        }
        break;
      case run_st:
        if(cycleCounter > LED_CYCLES) {
            cycleCounter = 0;
            currentState = wait_st;
        }
        else {
            currentState = run_st;
        }
        break;
    default:
        printf(ERROR_MSG);
      // print an error message here.
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
      case init_st:
        break;
      case wait_st:
        break;
      case run_st:

        cycleCounter++;
        break;
    default:
        printf(ERROR_MSG);
      // print an error message here.
      break;
  }
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
    lockoutTimer_init();
    clock_t before = clock();
    lockoutTimer_start();

        while(lockoutTimer_isRunning()) {
            
        }
    printf("Time difference: %d\n", clock() - before);

}

#endif /* LOCKOUTTIMER_H_ */
