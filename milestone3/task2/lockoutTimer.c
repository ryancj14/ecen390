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
#include <stdio.h>
#include "intervalTimer.h"

#include "lockoutTimer.h"

#define START_LOCKOUT_TIMER "completed one test period.\n\r"
#define LOCKOUT_TIMER_VALUE "Lockout Timer Test: %f seconds\n"
#define LOCKOUT_TIMER_FAIL "failed to initialize intervalTimer 0\n"
#define ERROR_MSG "Lockout Timer Error\n"
#define INIT_ST_MSG "Init st\n"
#define WAIT_ST_MSG "Wait state\n"
#define RUN_ST_MSG "Run state\n"

#define INTERVAL_TIMER_INDEX 1
#define INTERVAL_TIMER_STATUS_OK 1

#define LED_CYCLES 50000 //half a second
uint32_t static cycleCounter;
static bool debugPrint;
#define LOCKOUT_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.

enum lockoutTimer_st_t {
    init_st,
	wait_st,
    run_st
} lockout_currentState;

volatile static bool timerStartFlag = false;

void lockoutTimer_debugStatePrint() {
  static enum lockoutTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != lockout_currentState - this prevents reprinting the same state name over and over.
  if (previousState != lockout_currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = lockout_currentState;     // keep track of the last state that you were in.
    switch(lockout_currentState) {            // This prints messages based upon the state that you were in.
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
    lockout_currentState = init_st;
}

// Returns true if the timer is running.
bool lockoutTimer_running() {
    return timerStartFlag;
}

// Standard tick function.
void lockoutTimer_tick() {
  //if the debug print statements are enabled, print 
  if(debugPrint)
    lockoutTimer_debugStatePrint();
    //transitions for the state machien
switch(lockout_currentState) {
      case init_st:
        lockout_currentState = wait_st;
        break;
      case wait_st:
      //if the timerstartflag is enabled, then run
        if(timerStartFlag) {
            cycleCounter = 0;
            lockout_currentState = run_st;
        }
        else {
            lockout_currentState = wait_st;
        }
        break;
      case run_st:
      //if the counter reaches max, then wait again
        if(cycleCounter > LED_CYCLES) {
            lockout_currentState = wait_st;
            timerStartFlag = false;
        }
        else {
            lockout_currentState = run_st;
        }
        break;
    default:
        printf(ERROR_MSG);
      // print an error message here.
      break;
  }
  
  // Perform state action next.
  switch(lockout_currentState) {
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
  printf(START_LOCKOUT_TIMER);
  lockoutTimer_init();

//if the timer index is acceptable
  if (intervalTimer_init(INTERVAL_TIMER_INDEX) == INTERVAL_TIMER_STATUS_OK) {
    intervalTimer_reset(INTERVAL_TIMER_INDEX);
    intervalTimer_start(INTERVAL_TIMER_INDEX);
    lockoutTimer_start();
    //continuous loop to keep test running while enabled
    while (lockoutTimer_running()) {
    }
    intervalTimer_stop(INTERVAL_TIMER_INDEX);
    double intervalTimerValue =
        intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_INDEX);
    printf(LOCKOUT_TIMER_VALUE, intervalTimerValue);
  } else {
    printf(LOCKOUT_TIMER_FAIL);
  }
}

