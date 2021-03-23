/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include <stdint.h>

#include "hitLedTimer.h"
#include "interrupts.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "transmitter.h"
#include "trigger.h"
#include <stdio.h>

// Uses interval timer 0 to measure time spent in ISR.
#define ENABLE_INTERVAL_TIMER_0_IN_TIMER_ISR 1

// Keep track of how many times isr_function() is called.
// static uint64_t isr_totalXadcSampleCount = 0;

// This implements a dedicated buffer for storing values from the ADC
// until they are read and processed by detector().
// adcBuffer_t is similar to a queue.
#define ADC_BUFFER_SIZE 100000
typedef struct {
  uint32_t indexIn;               // New values go here.
  uint32_t indexOut;              // Pull old values from here.
  uint32_t data[ADC_BUFFER_SIZE]; // Store values here.
} adcBuffer_t;

static uint32_t queueSize;
static adcBuffer_t queue;
typedef uint32_t
    isr_AdcValue_t; // Used to represent ADC values in the ADC buffer.

// isr provides the isr_function() where you will place functions that require
// accurate timing. A buffer for storing values from the Analog to Digital
// Converter (ADC) is implemented in isr.c Values are added to this buffer by
// the code in isr.c. Values are removed from this queue by code in detector.c

// Init adcBuffer.
void adcBufferInit() {
  queue.indexIn = 0;
  queue.indexOut = 0;
}

// Init everything in isr.
void isr_init() {
  adcBufferInit(); // init the local adcBuffer.
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
  transmitter_tick();
  hitLedTimer_tick();
  lockoutTimer_tick();
  trigger_tick();
  isr_addDataToAdcBuffer(interrupts_getAdcData());
}

// This adds data to the ADC queue. Data are removed from this queue and used by
// the detector.
void isr_addDataToAdcBuffer(uint32_t adcData) {
  if (queueSize < ADC_BUFFER_SIZE) {
    queue.data[queue.indexIn] = adcData;
    queue.indexIn++;
    queueSize++;
    if (queue.indexIn >= ADC_BUFFER_SIZE) {
      queue.indexIn = 0;
    }
  }
}

// This removes a value from the ADC buffer.
uint32_t isr_removeDataFromAdcBuffer() {
  if (queueSize > 0) {
    uint32_t returnValue = queue.data[queue.indexOut];
    queue.indexOut++;
    queueSize--;
    if (queue.indexOut >= ADC_BUFFER_SIZE) {
      queue.indexOut = 0;
    }
    return returnValue;
  } else {
    return 0;
  }
}

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount() { return queueSize; }
