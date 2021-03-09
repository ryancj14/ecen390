/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/
#include "mio.h"
#define TRANSMITTER_OUTPUT_PIN 13     // JF1 (pg. 25 of ZYBO reference manual).
#define TRANSMITTER_PULSE_WIDTH 20000 // Based on a system tick-rate of 100 kHz.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "filter.h"
#include "utils.h"
#include "buttons.h"
#include "switches.h"

#include "transmitter.h"

// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

#define START_TRANSMITTER_MSG "starting transmitter_runTest()\n\r"
#define COMPLETED_PERIOD_MSG "completed one test period.\n\r"

#define INIT_ST_MSG "Init state\n"
#define WAIT_FOR_STARTFLAG_ST_MSG "Wait for start flag state\n"
#define OUTPUT_LOW_ST_MSG "Output low state\n"
#define OUTPUT_HIGH_ST_MSG "Output high state\n"
#define ERROR_MSG "Transmitter error occurred\n"

#define BTN1_MASK 0x2
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
static bool freqUpdated;
static bool isEnabled;
static bool isContinuous;
static bool debugPrint;
static uint16_t frequencyValue;
static uint16_t storeFreqValue;
static uint32_t tickCounter;
static uint32_t highLowCounter;
static enum transmitter_st_t {
	init_st,                 // Start here, transition out of this state on the first tick.
	wait_for_startFlag_st,
    output_low_st,
    output_high_st
} transmitter_currentState;

// Standard init function.
void transmitter_init() {
    transmitter_currentState = init_st;
    isEnabled = false;
    frequencyValue = 0;
    tickCounter = 0;
    storeFreqValue = 0;
    highLowCounter = 0;
    isContinuous = false;
    freqUpdated = false;
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
  mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.
}

//sets the mio pin to high
void transmitter_set_jf1_to_one() {
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

//sets the mio pin to low
void transmitter_set_jf1_to_zero() {
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '0' to JF-1.
}

// Starts the transmitter.
void transmitter_run() {
    isEnabled = true;
    frequencyValue = storeFreqValue;
}

// Returns true if the transmitter is still running.
bool transmitter_running() {
    return isEnabled;
}

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
  //if it's not enabled or it's in continuous, updated frequency; otherwise, store for later
    if (!isEnabled || isContinuous) 
    {
        frequencyValue = frequencyNumber;
    }
    else {
        storeFreqValue = frequencyNumber;
    }
}

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() {
    return frequencyValue;
}
//prints the state for every tick.
void transmitter_debugStatePrint() {
  static enum transmitter_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != transmitter_currentState - this prevents reprinting the same state name over and over.
  if (previousState != transmitter_currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = transmitter_currentState;     // keep track of the last state that you were in.
    switch(transmitter_currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf(INIT_ST_MSG);
        break;
      case wait_for_startFlag_st:
        printf(WAIT_FOR_STARTFLAG_ST_MSG);
        break;
      case output_low_st:
        printf(OUTPUT_LOW_ST_MSG);
        break;
      case output_high_st:
        printf(OUTPUT_HIGH_ST_MSG);
        break;
      default:
        printf(ERROR_MSG);
        break;
     }
  }
}

// Standard tick function.
void transmitter_tick() {
    
    // Perform state update first.
  switch(transmitter_currentState) {
      case init_st:
        transmitter_currentState = wait_for_startFlag_st;
        break;
      case wait_for_startFlag_st:
      //if it's enabled, then we set it to low.
        if(isEnabled) {
            transmitter_currentState = output_low_st;
            tickCounter = 0;
            highLowCounter = 0;
        }
        else {
            transmitter_currentState = wait_for_startFlag_st;
        }
        break;
      case output_low_st:
// if the counter for frequency hasn't been reached yet, stay in low
        if(highLowCounter < filter_frequencyTickTable[frequencyValue] / 2) {
            transmitter_currentState = output_low_st;
        } 
        else if((tickCounter >= TRANSMITTER_PULSE_WIDTH) && (!isContinuous)) {

            transmitter_currentState = wait_for_startFlag_st;
            tickCounter = 0;
            isEnabled = false;
        }
        else {
            transmitter_set_jf1_to_one();
            transmitter_currentState = output_high_st;
            highLowCounter = 0;
        }
        break;
      case output_high_st:
      //if the counter isn't reach yet, stay in high
        if(highLowCounter < filter_frequencyTickTable[frequencyValue] / 2) {
            transmitter_currentState = output_high_st;
        } 
        else if((tickCounter >= TRANSMITTER_PULSE_WIDTH) && (!isContinuous)) {
            transmitter_currentState = wait_for_startFlag_st;
            tickCounter = 0;
            isEnabled = false;
        }
        else {
            transmitter_set_jf1_to_zero();
            transmitter_currentState = output_low_st;
            highLowCounter = 0;
        }
        break;
    default:
        printf(ERROR_MSG);
      // print an error message here.
      break;
  }
  
  // Perform state action next.
  switch(transmitter_currentState) {
    case init_st:
        break;
      case wait_for_startFlag_st:
        break;
      case output_low_st:
        highLowCounter++;
        tickCounter++;
        break;
      case output_high_st:
        highLowCounter++;
        tickCounter++;
        break;
     default:
      // print an error message here.
      break;
  }  
  //print the debug statements if debug bool set to true
  if(debugPrint)
        transmitter_debugStatePrint(); 
}

//enable test mode
void transmitter_enableTestMode() {
    debugPrint = true;
}

//disable test mode
void transmitter_disableTestMode() {
    debugPrint = false;
}

#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise,
// transmits one pulse-width and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is in
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until the last 200 ms pulse is complete. NOTE: while running continuously,
// the transmitter will change frequencies at the end of each 200 ms pulse.
void transmitter_setContinuousMode(bool continuousModeFlag) {
  isContinuous = continuousModeFlag;
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN1 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
void transmitter_runNoncontinuousTest() {
  printf(START_TRANSMITTER_MSG);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  isContinuous = false;                                    // init the transmitter.
  transmitter_enableTestMode();                       // Prints diagnostics to stdio.
  while (!(buttons_read() & BTN1_MASK)) {                 // Run continuously until btn1 is pressed.
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    transmitter_run();                                    // Start the transmitter.
    while (transmitter_running()) {                       // Keep ticking until it is done.
      // transmitter_tick();
     utils_msDelay(50);
    }
    printf(COMPLETED_PERIOD_MSG);
    utils_msDelay(300);  
  }
  transmitter_disableTestMode();
  printf("exiting transmitter_runTest()\n\r");
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes to the changes in the slide switches.
// Test runs until BTN1 is pressed.
void transmitter_runContinuousTest() {
  printf(START_TRANSMITTER_MSG);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  transmitter_init(); 
  transmitter_setContinuousMode(true);                                 
  transmitter_enableTestMode();                       // Prints diagnostics to stdio.
  while (!(buttons_read() & BTN1_MASK)) {                 // Run continuously until btn1 is pressed.
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    printf(START_TRANSMITTER_MSG);
    transmitter_run();                                    // Start the transmitter.
    printf("Started transmitter run\n");
    while (transmitter_running()) {   
      switchValue = switches_read() % FILTER_FREQUENCY_COUNT;
      transmitter_setFrequencyNumber(switchValue);                            // Keep ticking until it is done.                
      printf("Switch value: %d\n", switchValue);
      utils_msDelay(100);
    }
    printf("completed one test period.\n\r");
    utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
  }
  transmitter_disableTestMode();
  printf("exiting transmitter_runTest()\n\r");
}
