/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/
#include "mio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "trigger.h"
#include "utils.h"
#include "transmitter.h"
#include "buttons.h"
#include "switches.h"


// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10

#define TRIGGER_TEST_TICK_PERIOD_IN_MS 50 //???? idk
#define GUN_TRIGGER_PRESSED 1

#define DOWN_PRINT "D\n"
#define UP_PRINT "U\n"
#define INIT_ST_MSG "Init state\n"
#define WAIT_FOR_PRESS_ST_MSG "Wait for press state\n"
#define DEBOUNCE_PRESS_ST_MSG "Debounce press state\n"
#define WAIT_FOR_RELEASE_ST_MSG "Wait for release state\n"
#define DEBOUNCE_RELEASE_ST_MSG "Debounce release state\n"
#define ERROR_MSG "Trigger error\n"
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
} trigger_currentState;

//return true if trigger is pressed
bool trigger_triggerPressed() {
	return ((!ignoreGunInput && (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) || 
                ((buttons_read() & BUTTONS_BTN0_MASK) != 0));
}

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion
// in lab web pages). Initializes the mio subsystem.
void trigger_init() {
  mio_init(false);
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
//return true if trigger is enabled
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
//print states if debug states enabled
void trigger_debugStatePrint() {
  static enum trigger_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != trigger_currentState - this prevents reprinting the same state name over and over.
  if (previousState != trigger_currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = trigger_currentState;     // keep track of the last state that you were in.
    switch(trigger_currentState) {            // This prints messages based upon the state that you were in.
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
      default:
        printf(ERROR_MSG);
     }
  }
}

// Standard tick function.
void trigger_tick() {
  //if debug prints are enabled print
  if(debugPrint)
    trigger_debugStatePrint(); 
    //if it's not enabled, stay in init state
    if(!isEnabled) {
        trigger_currentState = init_st;
        adcCounter = 0;
    }
    switch(trigger_currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
            trigger_currentState = wait_for_press_st;
        break;
      case wait_for_press_st:
      //if trigger is pressed, go to debounce state
      if(trigger_triggerPressed()) {
          trigger_currentState = debounce_press_st;
      }
      else {
          trigger_currentState = wait_for_press_st;
      }
        break;
      case debounce_press_st:
      //if adc counter has been fulfilled, then change state
        if(adcCounter >= ADC_MAX_VALUE && trigger_triggerPressed()) {
          //if it's pressed, then print D 
          if(debugPrint)
            { 
                printf(DOWN_PRINT);
            }
            adcCounter = 0;
            trigger_currentState = wait_for_release_st;
            transmitter_run();
        }
        else if(trigger_triggerPressed()){
            trigger_currentState = debounce_press_st;
        }
        else {
          adcCounter = 0;
          trigger_currentState = wait_for_press_st;
        }
        break;
      case wait_for_release_st:
      //if the trigger isn't pressed, move to debounce for release
        if(!trigger_triggerPressed()) {
            trigger_currentState = debounce_release_st;
        }
        else {
            trigger_currentState = wait_for_release_st;
        }
        break;
      case debounce_release_st:
      //if the adc max is met, then move on
      if(adcCounter >= ADC_MAX_VALUE && (!trigger_triggerPressed())) {
            adcCounter = 0;
            shotsRemaining--;
            //if debug print is turned on, print a U
            if(debugPrint)
            { 
                printf("U\n");
            }
            trigger_currentState = wait_for_press_st;
        }
        else if(!trigger_triggerPressed()){
            trigger_currentState = debounce_release_st;
        }
        else {
          adcCounter = 0;
          trigger_currentState = wait_for_release_st;
        }
        break;
        default:
        printf("Trigger error\n");
        break;
     }

     switch(trigger_currentState) {            // This prints messages based upon the state that you were in.
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
//enable test mode
void trigger_enableTestMode() {
    debugPrint = true;
}
//disable test mode
void trigger_disableTestMode() {
    debugPrint = false;
}

// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest() {
  debugPrint = true;
  trigger_enable();
printf("starting trigger_runTest()\n\r");
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.                          // Prints diagnostics to stdio.
  while (!(buttons_read() & BUTTONS_BTN1_MASK)) {                 // Run continuously until btn1 is pressed.
                                     // Start the transmitter (run)
    while (trigger_isRunning()) {                       // Keep ticking until it is done.                                 // tick.
 // short delay between ticks.
    }
    //printf("completed one test period.\n\r");
  }
  trigger_disableTestMode();
  printf("exiting trigger_runTest()\n\r");
}

