/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

// This file is provided for use in Milestone 3, Task 2.
// Modify as you see fit.

#include "interrupts.h"
#include "lockoutTimer.h"
#include "ledTimer.h"
#include "transmitter.h"
#include "trigger.h"
#include <stdio.h>

int main() {
  interrupts_initAll(true);           // main interrupt init function.
  interrupts_enableTimerGlobalInts(); // enable global interrupts.
  interrupts_startArmPrivateTimer();  // start the main timer.
  interrupts_enableArmInts(); // now the ARM processor can see interrupts.
  trigger_init();
  transmitter_init();
  ledTimer_init();
  lockoutTimer_init();
  // printf("LED timer starting\n");
  // ledTimer_runTest();
  // printf("Starting Lockout Timer\n");
  // lockoutTimer_runTest();
  // printf("Starting trigger Test\n");
  // trigger_runTest();
  printf("Starting noncontinuous test\n");
    transmitter_runNoncontinuousTest();
  //   printf("Starting transmitter continuous test\n");
  //   //transmitter_runContinuousTest();
  //   ledTimer_runTest();
}