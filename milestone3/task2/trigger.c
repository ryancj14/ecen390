/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "trigger.h"
#include "utils.h"
#include "transmitter.h"
#include "buttons.h"
#include "switches.h"

#define BTN1_MASK 0x0002
// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10

#define TRIGGER_TEST_TICK_PERIOD_IN_MS 50 //???? idk
#define GUN_TRIGGER_PRESSED 1

#define INIT_ST_MSG "Init state\n"
#define WAIT_FOR_PRESS_ST_MSG "Wait for press state\n"
#define DEBOUNCE_PRESS_ST_MSG "Debounce press state\n"
#define WAIT_FOR_RELEASE_ST_MSG "Wait for release state\n"
#define DEBOUNCE_RELEASE_ST_MSG "Debounce release state\n"

#define ADC_MAX_VALUE 5000

static uint32_t adcCounter;
typedef uint16_t trigger_shotsRemaining_t;
static bool ignoreGunInput;
static trigger_shotsRemaining_t shotsRemaining;
static bool isEnabled;
static bool debugPrint;
static enum trigger_st_t {
	init_st,                 // Start here, transition out of this state on the first tick.
	wait_for_press_st,
    debounce_press_st,
    wait_for_release_st,
    debounce_release_st
} currentState;

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion
// in lab web pages). Initializes the mio subsystem.
void trigger_init() {
    debugPrint = false;
    isEnabled = false;

    shotsRemaining = 0;
    adcCounter = 0;
    mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
  // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
  if (trigger_triggerPressed()) {
    ignoreGunInput = true;
  }
  else {
    ignoreGunInput = false;
  }
}


// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() {
    isEnabled = true;
}

bool trigger_isRunning() {
  return isEnabled;
}
// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() {
    isEnabled = false;
}

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
    return shotsRemaining;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
    shotsRemaining = count;
}
void trigger_debugStatePrint() {
  static enum trigger_st_t previousState;
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
      case wait_for_press_st:
        printf(WAIT_FOR_PRESS_ST_MSG);
      case debounce_press_st:
        printf(DEBOUNCE_PRESS_ST_MSG);
        break;
      case wait_for_release_st:
        printf(WAIT_FOR_RELEASE_ST_MSG);
        break;
      case debounce_release_st:
        printf(DEBOUNCE_RELEASE_ST_MSG);
        break;
     }
  }
}
bool trigger_triggerPressed() {
	return ((!ignoreGunInput && (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) || 
                (buttons_read() & BUTTONS_BTN0_MASK));
}

// Standard tick function.
void trigger_tick() {
    trigger_debugStatePrint(); 
    if(!isEnabled) {
        currentState = init_st;
        adcCounter = 0;
    }
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
            currentState = wait_for_press_st;
        break;
      case wait_for_press_st:
      if(trigger_triggerPressed()) {
          currentState = debounce_press_st;
      }
      else {
          currentState = wait_for_press_st;
      }
        break;
      case debounce_press_st:
        if(adcCounter >= ADC_MAX_VALUE && trigger_triggerPressed()) {
            adcCounter = 0;
            currentState = wait_for_release_st;
            transmitter_run();
            if(debugPrint)
            { 
                printf("D\n");
            }
        }
        else {
            currentState = debounce_press_st;
        }
        break;
      case wait_for_release_st:
        if(!trigger_triggerPressed()) {
            currentState = debounce_release_st;
        }
        else {
            currentState = wait_for_release_st;
        }
        break;
      case debounce_release_st:
      if(adcCounter >= ADC_MAX_VALUE && trigger_triggerPressed()) {
            adcCounter = 0;
            shotsRemaining--;
            if(debugPrint)
            { 
                printf("U\n");
            }
            currentState = wait_for_press_st;
        }
        else {
            currentState = debounce_release_st;
        }
        break;
     }

     switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        break;
      case debounce_press_st:
        adcCounter++;
        break;
      case wait_for_press_st:
        break;
      case wait_for_release_st:
        break;
      case debounce_release_st:
        adcCounter++;
        break;
     }
}

void trigger_enableTestMode() {
    debugPrint = true;
}
void trigger_disableTestMode() {
    debugPrint = false;
}

// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest() {
printf("starting trigger_runTest()\n\r");
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  trigger_init();                                     // init the transmitter.
  transmitter_init();
  trigger_debugStatePrint();                           // Prints diagnostics to stdio.
  while (!(buttons_read() & BTN1_MASK)) {                 // Run continuously until btn1 is pressed.
                                     // Start the transmitter (run)
    while (trigger_isRunning()) {                       // Keep ticking until it is done.
      trigger_tick();                                 // tick.
      utils_msDelay(TRIGGER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
    }
    printf("completed one test period.\n\r");
  }
  trigger_disableTestMode();
  printf("exiting trigger_runTest()\n\r");
}

