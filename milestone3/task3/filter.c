/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#ifndef FILTER_H_
#define FILTER_H_

#include "queue.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define IIR_A_COEFFICIENT_COUNT 10
#define IIR_B_COEFFICIENT_COUNT 11

#define FILTER_FREQUENCY_COUNT 10

#define FILTER_IIR_FILTER_COUNT 10
#define Z_QUEUE_SIZE 10
#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE 11
#define OUTPUT_QUEUE_SIZE 10

#define OUTPUT_INSIDE_QUEUE_SIZE 2000

#define FIR_FILTER_TAP_COUNT 81
#define POWER_VALUES 10

#define DECIMATION_VALUE 10

static queue_t zQueue[FILTER_IIR_FILTER_COUNT];
static queue_t outputQueue[OUTPUT_QUEUE_SIZE];
static queue_t xQueue;
static queue_t yQueue;

static double currentPowerValue[POWER_VALUES];

const static double firCoefficients[FIR_FILTER_TAP_COUNT] = {
    -2.3389030995116655e-22, -3.7469606409466652e-07, -2.1170053091600836e-06,
    -6.6213586689584756e-06, -1.5062557928299490e-05, -2.7821799444662355e-05,
    -4.3589392664419411e-05, -5.8296747733876897e-05, -6.4211582624672668e-05,
    -4.9674081802547686e-05, 8.4730027622448665e-20,  1.0002105298756125e-04,
    2.6187524582957898e-04,  4.8759997325237709e-04,  7.6340388191615323e-04,
    1.0539924532807440e-03,  1.2993641415646770e-03,  1.4159590001115261e-03,
    1.3037485479782381e-03,  8.6009332261973608e-04,  -8.4773542841798861e-19,
    -1.3190683664248679e-03, -3.0711854500643948e-03, -5.1393355525019791e-03,
    -7.3013471519152344e-03, -9.2292791621008830e-03, -1.0505847437361372e-02,
    -1.0659210338765657e-02, -9.2144665378527044e-03, -5.7570090201892916e-03,
    2.7122082622102036e-18,  8.1537260542085509e-03,  1.8565568558236156e-02,
    3.0839965394623067e-02,  4.4334356306044619e-02,  5.8202384985928925e-02,
    7.1467499081530605e-02,  8.3119244151165658e-02,  9.2220586044455516e-02,
    9.8012268898890739e-02,  1.0000000000000001e-01,  9.8012268898890753e-02,
    9.2220586044455530e-02,  8.3119244151165672e-02,  7.1467499081530619e-02,
    5.8202384985928932e-02,  4.4334356306044626e-02,  3.0839965394623064e-02,
    1.8565568558236156e-02,  8.1537260542085578e-03,  2.7122082622102039e-18,
    -5.7570090201892960e-03, -9.2144665378527078e-03, -1.0659210338765653e-02,
    -1.0505847437361373e-02, -9.2292791621008795e-03, -7.3013471519152379e-03,
    -5.1393355525019852e-03, -3.0711854500643969e-03, -1.3190683664248695e-03,
    -8.4773542841798919e-19, 8.6009332261973554e-04,  1.3037485479782391e-03,
    1.4159590001115254e-03,  1.2993641415646772e-03,  1.0539924532807462e-03,
    7.6340388191615367e-04,  4.8759997325237796e-04,  2.6187524582957930e-04,
    1.0002105298756111e-04,  8.4730027622448810e-20,  -4.9674081802547638e-05,
    -6.4211582624672668e-05, -5.8296747733877493e-05, -4.3589392664419662e-05,
    -2.7821799444662457e-05, -1.5062557928299011e-05, -6.6213586689583579e-06,
    -2.1170053091602801e-06, -3.7469606409465350e-07, -2.3389030995116655e-22};

const static double iirACoefficientConstants
    [FILTER_FREQUENCY_COUNT][IIR_A_COEFFICIENT_COUNT] = {
        {-5.9637727070164015e+00, 1.9125339333078248e+01,
         -4.0341474540744173e+01, 6.1537466875368821e+01,
         -7.0019717951472188e+01, 6.0298814235238872e+01,
         -3.8733792862566290e+01, 1.7993533279581058e+01,
         -5.4979061224867651e+00, 9.0332828533799547e-01},
        {-4.6377947119071443e+00, 1.3502215749461572e+01,
         -2.6155952405269755e+01, 3.8589668330738348e+01,
         -4.3038990303252632e+01, 3.7812927599537133e+01,
         -2.5113598088113793e+01, 1.2703182701888094e+01,
         -4.2755083391143520e+00, 9.0332828533800291e-01},
        {-3.0591317915750960e+00, 8.6417489609637634e+00,
         -1.4278790253808875e+01, 2.1302268283304372e+01,
         -2.2193853972079314e+01, 2.0873499791105537e+01,
         -1.3709764520609468e+01, 8.1303553577932188e+00,
         -2.8201643879900726e+00, 9.0332828533800769e-01},
        {-1.4071749185996747e+00, 5.6904141470697471e+00,
         -5.7374718273676217e+00, 1.1958028362868873e+01,
         -8.5435280598354382e+00, 1.1717345583835918e+01,
         -5.5088290876998407e+00, 5.3536787286077372e+00,
         -1.2972519209655518e+00, 9.0332828533799414e-01},
        {8.2010906117760141e-01, 5.1673756579268559e+00, 3.2580350909220819e+00,
         1.0392903763919172e+01, 4.8101776408668879e+00, 1.0183724507092480e+01,
         3.1282000712126603e+00, 4.8615933365571822e+00, 7.5604535083144497e-01,
         9.0332828533799658e-01},
        {2.7080869856154512e+00, 7.8319071217995688e+00, 1.2201607990980744e+01,
         1.8651500443681620e+01, 1.8758157568004549e+01, 1.8276088095999022e+01,
         1.1715361303018897e+01, 7.3684394621253499e+00, 2.4965418284511904e+00,
         9.0332828533800436e-01},
        {4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01,
         4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01,
         2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00,
         9.0332828533799958e-01},
        {6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071684e+01,
         6.5958045321253451e+01, 7.5230437667866596e+01, 6.4630411355739852e+01,
         4.1261591079244127e+01, 1.8936128791950534e+01, 5.6881982915180291e+00,
         9.0332828533799803e-01},
        {7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01,
         9.8258255839887312e+01, 1.1359460153696298e+02, 9.6280452143026082e+01,
         5.9124742025776392e+01, 2.5268527576524203e+01, 6.8305064480743081e+00,
         9.0332828533799969e-01},
        {8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037053e+01,
         1.3928510844056814e+02, 1.6305115418161620e+02, 1.3648147221895786e+02,
         8.0686288623299745e+01, 3.2276361903872115e+01, 7.9045143816244696e+00,
         9.0332828533799636e-01}};

const static double
    iirBCoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_B_COEFFICIENT_COUNT] =
        {{9.0928532700696364e-10, -0.0000000000000000e+00,
          -4.5464266350348181e-09, -0.0000000000000000e+00,
          9.0928532700696362e-09, -0.0000000000000000e+00,
          -9.0928532700696362e-09, -0.0000000000000000e+00,
          4.5464266350348181e-09, -0.0000000000000000e+00,
          -9.0928532700696364e-10},
         {9.0928698045638776e-10, 0.0000000000000000e+00,
          -4.5464349022819393e-09, 0.0000000000000000e+00,
          9.0928698045638787e-09, 0.0000000000000000e+00,
          -9.0928698045638787e-09, 0.0000000000000000e+00,
          4.5464349022819393e-09, 0.0000000000000000e+00,
          -9.0928698045638776e-10},
         {9.0928689298097020e-10, 0.0000000000000000e+00,
          -4.5464344649048510e-09, 0.0000000000000000e+00,
          9.0928689298097020e-09, 0.0000000000000000e+00,
          -9.0928689298097020e-09, 0.0000000000000000e+00,
          4.5464344649048510e-09, 0.0000000000000000e+00,
          -9.0928689298097020e-10},
         {9.0928696329154157e-10, 0.0000000000000000e+00,
          -4.5464348164577090e-09, 0.0000000000000000e+00,
          9.0928696329154180e-09, 0.0000000000000000e+00,
          -9.0928696329154180e-09, 0.0000000000000000e+00,
          4.5464348164577090e-09, 0.0000000000000000e+00,
          -9.0928696329154157e-10},
         {9.0928648497937087e-10, 0.0000000000000000e+00,
          -4.5464324248968538e-09, 0.0000000000000000e+00,
          9.0928648497937076e-09, 0.0000000000000000e+00,
          -9.0928648497937076e-09, 0.0000000000000000e+00,
          4.5464324248968538e-09, 0.0000000000000000e+00,
          -9.0928648497937087e-10},
         {9.0928645119506948e-10, -0.0000000000000000e+00,
          -4.5464322559753479e-09, -0.0000000000000000e+00,
          9.0928645119506958e-09, -0.0000000000000000e+00,
          -9.0928645119506958e-09, -0.0000000000000000e+00,
          4.5464322559753479e-09, -0.0000000000000000e+00,
          -9.0928645119506948e-10},
         {9.0928343368482748e-10, -0.0000000000000000e+00,
          -4.5464171684241375e-09, -0.0000000000000000e+00,
          9.0928343368482750e-09, -0.0000000000000000e+00,
          -9.0928343368482750e-09, -0.0000000000000000e+00,
          4.5464171684241375e-09, -0.0000000000000000e+00,
          -9.0928343368482748e-10},
         {9.0929508683806034e-10, 0.0000000000000000e+00,
          -4.5464754341903021e-09, 0.0000000000000000e+00,
          9.0929508683806042e-09, 0.0000000000000000e+00,
          -9.0929508683806042e-09, 0.0000000000000000e+00,
          4.5464754341903021e-09, 0.0000000000000000e+00,
          -9.0929508683806034e-10},
         {9.0926783827278939e-10, 0.0000000000000000e+00,
          -4.5463391913639461e-09, 0.0000000000000000e+00,
          9.0926783827278922e-09, 0.0000000000000000e+00,
          -9.0926783827278922e-09, 0.0000000000000000e+00,
          4.5463391913639461e-09, 0.0000000000000000e+00,
          -9.0926783827278939e-10},
         {9.0906302220838671e-10, 0.0000000000000000e+00,
          -4.5453151110419338e-09, 0.0000000000000000e+00,
          9.0906302220838675e-09, 0.0000000000000000e+00,
          -9.0906302220838675e-09, 0.0000000000000000e+00,
          4.5453151110419338e-09, 0.0000000000000000e+00,
          -9.0906302220838671e-10}};
#define INITIAL_VALUE 0.0
#define FILTER_SAMPLE_FREQUENCY_IN_KHZ 100
#define FILTER_FIR_DECIMATION_FACTOR                                           \
  10 // FIR-filter needs this many new inputs to compute a new output.
#define FILTER_INPUT_PULSE_WIDTH                                               \
  2000 // This is the width of the pulse you are looking for, in terms of
       // decimated sample count.
// These are the tick counts that are used to generate the user frequencies.
// Not used in filter.h but are used to TEST the filter code.
// Placed here for general access as they are essentially constant throughout
// the code. The transmitter will also use these.
static const uint16_t filter_frequencyTickTable[FILTER_FREQUENCY_COUNT] = {
    68, 58, 50, 44, 38, 34, 30, 28, 26, 24};

// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.

// 1. First filter is a decimating FIR filter with a configurable number of taps
// and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10
// IIR filters. The characteristics of the IIR filter are fixed.

/*********************************************************************************************************
****************************************** Main Filter Functions
******************************************
**********************************************************************************************************/

/***
initializes the xQueue.
***/

void initXQueue() {
  queue_init(&(xQueue), X_QUEUE_SIZE, "x_queue");
  // pushes 0 value for initialization for size of queue
  for (uint32_t j = 0; j < X_QUEUE_SIZE; j++)
    queue_overwritePush(&(xQueue), INITIAL_VALUE);
}
/***
initializes the yQueue.
***/
void initYQueue() {
  queue_init(&(yQueue), Y_QUEUE_SIZE, "y_queue");
  // pushes 0 value for initialization for size of queue
  for (uint32_t i = 0; i < Y_QUEUE_SIZE; i++)
    queue_overwritePush(&(yQueue), INITIAL_VALUE);
}

/***
initializes the output queues
***/
void initOutputQueues() {
  // initializes each queue
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(outputQueue[i]), OUTPUT_INSIDE_QUEUE_SIZE, "output");
    // pushes 0 value for initialization for size of queue
    for (uint32_t j = 0; j < OUTPUT_INSIDE_QUEUE_SIZE; j++)
      queue_overwritePush(&(outputQueue[i]), INITIAL_VALUE);
  }
}

/***
initializes the z queues
***/
void initZQueues() {
  // initializes each queue
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(zQueue[i]), Z_QUEUE_SIZE, "z_output");
    // pushes 0 value for initialization for size of queue
    for (uint32_t j = 0; j < Z_QUEUE_SIZE; j++)
      queue_overwritePush(&(zQueue[i]), INITIAL_VALUE);
  }
}

/***
initializes the current power queue
***/
void initCurrentPowerQueue() {
  // pushes 0 value for initialization for size of queue
  for (uint32_t j = 0; j < POWER_VALUES; j++)
    currentPowerValue[j] = 0.0;
}

// Must call this prior to using any filter functions.
// initializes all the filter queues
void filter_init() {
  // Init queues and fill them with 0s.
  initXQueue();       // Call queue_init() on xQueue and fill it with zeros.
  initYQueue();       // Call queue_init() on yQueue and fill it with zeros.
  initZQueues();      // Call queue_init() on all of the zQueues and fill each z
                      // queue with zeros.
  initOutputQueues(); // Call queue_init() all of the outputQueues and fill each
                      // outputQueue with zeros.
  initCurrentPowerQueue();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
/***
adds new inputs
x   double value to add as an input
***/
void filter_addNewInput(double x) { queue_overwritePush(&xQueue, x); }

// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
/***
*q  queue to fill
fillValue   value to fill the entire queue with
***/
void filter_fillQueue(queue_t *q, double fillValue) {
  // pushes 0 value for initialization for size of queues
  for (uint32_t i = 0; i < queue_size(q); i++)
    queue_overwritePush(q, fillValue);
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
/***
returns double with fir filter value
***/
double filter_firFilter() {
  double y = 0.0;
  // adds fir value multiplied by coefficient for each item in queue
  for (uint32_t i = 0; i < X_QUEUE_SIZE; i++) {
    y +=
        queue_readElementAt(&xQueue, X_QUEUE_SIZE - i - 1) * firCoefficients[i];
  }
  queue_overwritePush(&yQueue, y);
  return y;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also ed onto zQueue[filterNumber].
/***
filterNumber    filter value to use a single iir filter from
returns double value that is result of the iir filter
***/
double filter_iirFilter(uint16_t filterNumber) {
  double y_values = 0.0;
  double z_values = 0.0;
  // computes the y_value
  for (uint32_t i = 0; i < Y_QUEUE_SIZE; i++) {
    y_values += queue_readElementAt(&yQueue, Y_QUEUE_SIZE - i - 1) *
                iirBCoefficientConstants[filterNumber][i];
  }
  // compute the z_value sum
  for (uint32_t i = 0; i < Z_QUEUE_SIZE; i++) {
    z_values +=
        queue_readElementAt(&(zQueue[filterNumber]), Z_QUEUE_SIZE - i - 1) *
        iirACoefficientConstants[filterNumber][i];
  }
  queue_overwritePush(&(outputQueue[filterNumber]), y_values - z_values);
  queue_overwritePush(&(zQueue[filterNumber]), y_values - z_values);
  return y_values - z_values;
}

/***
initializes the power of the output queues
***/
void initOutputQueuesPower() {
  // initializes each queue
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(outputQueue[i]), Z_QUEUE_SIZE, "z_queue");
    // initializes each value to 0 in the queue
    for (uint32_t j = 0; j < Z_QUEUE_SIZE; j++)
      queue_overwritePush(&(outputQueue[i]), INITIAL_VALUE);
  }
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.
/***
filterNumber    filter value to work with
forceComputeFromScratch     if this is true, then it recomputes all power values
from scratch debugPrint  if true, prints debug statements returns double value
of the power value based on the outputQueue
***/
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint) {
  static double oldValues[POWER_VALUES] = {INITIAL_VALUE};
  // if forcecomputefromscratch is true, then recompute all value sfrom scratch
  if (forceComputeFromScratch) {
    currentPowerValue[filterNumber] = 0;
    // compute each value from scratch in the queue
    for (uint32_t i = 0; i < OUTPUT_INSIDE_QUEUE_SIZE; i++) {
      double value = queue_readElementAt(&(outputQueue[filterNumber]), i);
      currentPowerValue[filterNumber] += (value * value);
    }
  } else {
    double newValue = queue_readElementAt(&(outputQueue[filterNumber]),
                                          OUTPUT_INSIDE_QUEUE_SIZE - 1);
    // printf("New value: %f\n", newValue);
    currentPowerValue[filterNumber] +=
        ((newValue * newValue) -
         (oldValues[filterNumber] * oldValues[filterNumber]));
  }

  oldValues[filterNumber] =
      queue_readElementAt(&(outputQueue[filterNumber]), 0);
  // printf("Current power value: %f\n", currentPowerValue[filterNumber]);
  return currentPowerValue[filterNumber];
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
/***
filterNumber    filter value to work with
returns double that is the last-computed output power value
***/
double filter_getCurrentPowerValue(uint16_t filterNumber) {
  return currentPowerValue[filterNumber];
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
/***
powerValues[]   computed power values to copy into our current power values
array
***/
void filter_getCurrentPowerValues(double powerValues[]) {
  // sets each value in the array
  for (uint32_t i = 0; i < POWER_VALUES; i++)
    powerValues[i] = currentPowerValue[i];
}

// Using the previously-computed power values that are current stored in
// currentPowerValue[] array, Copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].

/***
normalizedArray[]   array to hold normalized values with
*indexOfMaxValue    the max index
***/
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue) {
  // sets the normalized power value for each item
  for (uint32_t i = 0; i < POWER_VALUES; i++) {
    normalizedArray[i] =
        currentPowerValue[i] / (currentPowerValue[*indexOfMaxValue]);
  }
}

/*********************************************************************************************************
********************************** Verification-assisting functions.
**************************************
********* Test functions access the internal data structures of the filter.c via
*these functions. ********
*********************** These functions are not used by the main filter
*functions. ***********************
**********************************************************************************************************/

// Returns the array of FIR coefficients.
/***
returns double is the fir coeff
***/
const double *filter_getFirCoefficientArray() { return firCoefficients; }

// Returns the number of FIR coefficients.

uint32_t filter_getFirCoefficientCount() { return FIR_FILTER_TAP_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
  return iirACoefficientConstants[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() { return IIR_A_COEFFICIENT_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
  return iirBCoefficientConstants[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() { return IIR_B_COEFFICIENT_COUNT; }

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() { return Y_QUEUE_SIZE; }

// Returns the decimation value.
uint16_t filter_getDecimationValue() { return DECIMATION_VALUE; }

// Returns the address of xQueue.
queue_t *filter_getXQueue() { return &xQueue; }

// Returns the address of yQueue.
queue_t *filter_getYQueue() { return &yQueue; }

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {
  return &zQueue[filterNumber];
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
  return &outputQueue[filterNumber];
}

#endif /* FILTER_H_ */