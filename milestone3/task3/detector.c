
/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "detector.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FILTER_IIR_FILTER_COUNT 10
#define FILTER_FIR_DECIMATION_FACTOR 10
// Student testing functions will return these status values.
typedef uint32_t detector_status_t;   // Used to return status from tests.
#define DETECTOR_STATUS_OK 0          // Everything is A-OK.
#define DETECTOR_STATUS_FAILED_SORT 1 // Something wrong with the sort.
#define DETECTOR_STATUS_FAILED_IGNORE_USER_FREQUENCY                           \
  2 // Didn't properly ignore a specific frequency.
#define DETECTOR_STATUS_FAILED_FIND_MAX_POWER                                  \
  3 // Didn't find the max power frequency number.
#define DETECTOR_STATUS_FAILED_ADC_SCALING                                     \
  4                                      // Didn't properly scale the ADC value.
#define DETECTOR_STATUS_FAILED_GENERAL 5 // Non-specific failure.

#define FREQ_SIZE 10
#define POWER_SIZE 10

#define FUDGE_FACTOR 1000
#define MEDIAN_INDEX 5
typedef uint16_t detector_hitCount_t;

// typedef detector_status_t (*sortTestFunctionPtr)(bool, uint32_t, uint32_t,
//                                                  double[], double[], bool) {

//                                                  }

static bool testOne, testTwo;
static uint32_t hitTestValues[POWER_SIZE] = {8, 7, 6, 5, 3, 1, 0, 8000, 3, 10};
static uint32_t missTestValues[POWER_SIZE] = {0, 0, 0, 0, 0, 0, 0, 1, 3, 4};
static uint32_t fudgeFactor;
static double powerArray[POWER_SIZE];
static uint32_t detector_hitArray[POWER_SIZE];
static bool hitDetectedFlag;
static bool ignoreHitsFlag;
static uint32_t lastFrequency;
static bool ignoreFrequency[FREQ_SIZE];
static uint32_t sampleCount;
static bool ignoreFreq[POWER_SIZE] = {false, false, false, false, false,
                                      false, false, false, false, false};
// Always have to init things.
// bool array is indexed by frequency number, array location set for true to
// ignore, false otherwise. This way you can ignore multiple frequencies.
void detector_init(bool ignoredFrequencies[]) {
  fudgeFactor = FUDGE_FACTOR;
  lastFrequency = 0;
  ignoreHitsFlag = false;
  sampleCount = 0;
  filter_init();
  testOne = false;
  testTwo = false;
  hitDetectedFlag = false;
  for (uint32_t i = 0; i < FREQ_SIZE; i++) {
    ignoreFrequency[i] = ignoredFrequencies[i];
  }
}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue) {
  return (adcValue / (2047.5)) - 1;
}
// Returns true if a hit was detected.
bool detector_hitDetected() {
  if (!ignoreHitsFlag) {
    double arrayIndices[POWER_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    detector_sort(&lastFrequency, powerArray, arrayIndices);
    double median = powerArray[(uint32_t)(arrayIndices[MEDIAN_INDEX])];
    double threshold = median * FUDGE_FACTOR;
    if (powerArray[(uint32_t)(arrayIndices[9])] > threshold) {
      lastFrequency = arrayIndices[9];
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}
// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit() { return lastFrequency; }

// Runs the entire detector: decimating fir-filter, iir-filters,
// power-computation, hit-detection. if interruptsNotEnabled = true, interrupts
// are not running. If interruptsNotEnabled = true you can pop values from the
// ADC queue without disabling interrupts. If interruptsNotEnabled = false, do
// the following:
// 1. disable interrupts.
// 2. pop the value from the ADC queue.
// 3. re-enable interrupts if interruptsNotEnabled was true.
// if ignoreSelf == true, ignore hits that are detected on your frequency.
// Your frequency is simply the frequency indicated by the slide switches
void detector(bool interruptsCurrentlyEnabled) {
  uint32_t elementCount = isr_adcBufferElementCount();
  for (uint32_t i = 0; i < elementCount; i++) {
    if (interruptsCurrentlyEnabled) {
      interrupts_disableArmInts();
    }
    uint32_t rawAdcValue = isr_removeDataFromAdcBuffer();
    if (interruptsCurrentlyEnabled) {
      interrupts_enableArmInts();
    }
    double scaledAdcValue = detector_getScaledAdcValue(rawAdcValue); // TEST
    filter_addNewInput(
        scaledAdcValue); // add a new input to the FIR history queue.
    sampleCount++;       // Keep track of how many samples you have acquired.
    if (sampleCount ==
        FILTER_FIR_DECIMATION_FACTOR) { // Only invoke the filters after every
                                        // DECIMATION_FACTOR times.
      sampleCount = 0;    // Reset the sample count when you run the filters.
      filter_firFilter(); // Runs the FIR filter on the accumulated input and
                          // places the output in the y-queue.
      for (uint32_t filterNumber = 0; filterNumber < FILTER_IIR_FILTER_COUNT;
           filterNumber++) {
        filter_iirFilter(filterNumber); // Run each of the IIR filters.
        if (testOne) {
          powerArray[filterNumber] = hitTestValues[filterNumber];
        } else if (testTwo) {
          powerArray[filterNumber] = missTestValues[filterNumber];
        } else {
          powerArray[filterNumber] = filter_computePower(
              filterNumber, false,
              false); // Compute the power for each of the IIR filters.
        }
      }
      if (!lockoutTimer_running()) {
        if (detector_hitDetected() &&
            !ignoreFrequency[detector_getFrequencyNumberOfLastHit()]) {
          lockoutTimer_start();
          hitLedTimer_start();
          detector_hitArray[detector_getFrequencyNumberOfLastHit()]++;
          hitDetectedFlag = true;
        }
      }
    }
  }
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit() { hitDetectedFlag = false; }

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) { ignoreHitsFlag = true; }

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
  for (uint32_t i = 0; i < POWER_SIZE; i++) {
    detector_hitArray[i] = hitArray[i];
  }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t setFudgeFactor) {
  fudgeFactor = setFudgeFactor;
}

// This function sorts the inputs in the unsortedArray and
// copies the sorted results into the sortedArray. It also
// finds the maximum power value and assigns the frequency
// number for that value to the maxPowerFreqNo argument.
// This function also ignores a single frequency as noted below.
// if ignoreFrequency is true, you must ignore any power from frequencyNumber.
// maxPowerFreqNo is the frequency number with the highest value contained in
// the unsortedValues. unsortedValues contains the unsorted values. sortedValues
// contains the sorted values. Note: it is assumed that the size of both of the
// array arguments is 10.
detector_status_t detector_sort(uint32_t *maxPowerFreqNo,
                                double unsortedValues[],
                                double sortedValues[]) {

  double tempPower[POWER_SIZE];
  for (uint32_t i = 0; i < POWER_SIZE; i++) {
    tempPower[i] = unsortedValues[i];
  }
  // for-loop: start at 1, and then
  uint32_t i, j;
  double key;
  for (i = 1; i < POWER_SIZE; i++) {
    key = tempPower[i];
    j = i - 1;

    /* Move elements of unsortedValues[0..i-1], that are
      greater than key, to one position ahead
      of their current position */
    while (j >= 0 && tempPower[j] > key) {
      unsortedValues[j + 1] = tempPower[j];
      sortedValues[j + 1] = sortedValues[j];
      j = j - 1;
    }
    tempPower[j + 1] = key;
  }
  return DETECTOR_STATUS_OK;
}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest() {
  printf("Running detector run test\n");
  detector_init(ignoreFreq);
  lockoutTimer_init();
  testOne = true;
  detector(true);
  if (detector_hitDetected()) {
    printf("Test one success\n");
  } else {
    printf("Test one failed\n");
  }
  detector_clearHit();
  testOne = false;
  testTwo = true;
  detector(true);
  if (!detector_hitDetected()) {
    printf("Test two success\n");
  } else {
    printf("Test two failed\n");
  }
}

// Returns 0 if passes, non-zero otherwise.
// if printTestMessages is true, print out detailed status messages.
detector_status_t detector_testSort(sortTestFunctionPtr testSortFunction,
                                    bool printTestMessages) {
  // test sort
}

detector_status_t detector_testAdcScaling() {}
