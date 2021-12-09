/***************************************************************************

EL Lightwire Sequence Generator using Sparkfun EL Sequencer board (COM-12781)

****************************************************************************/

#include "Arduino.h"


#define VERBOSE           0
#define DELAY_INTERVAL    10  // baseDelay is to be multiplied by 10msec
#define RAND_PATT_DELAY   10  // baseDelay for the random pattern case -- 10msec
#define MAX_PATTERNS      32

#define NUM_CHANNELS      8
#define MAX_SEQUENCES     8

// pin definitions
#define CHANNEL_A         2
#define CHANNEL_B         3
#define CHANNEL_C         4
#define CHANNEL_D         5
#define CHANNEL_E         6
#define CHANNEL_F         7
#define CHANNEL_G         8
#define CHANNEL_H         9

#define STATUS_LED        13

#define SELECT_B0         A2
#define SELECT_B1         A3
#define SELECT_B2         A4

#define UNUSED_ANALOG     A6
#define SPEED_POT         A7


// status LED state
unsigned char statusLED = 0;

// A Pattern is a set of (active HIGH) enables and a delay multiplier
// The delay following a Pattern is given by the value of the speed pot added to the
//  base value given by the Pattern's baseDelay value (multipled by DELAY_INTERVAL msecs)
typedef struct {
  unsigned char enables;    // active HIGH for each channel, LSB=channelA, MSB=channelH
  unsigned char baseDelay;  // number of delay intervals to be added to the speed pot value
} Pattern;

// A Sequence is an array of enables/delayMultiplier tuples
// Can have as many Sequences as there are selector switch positions
// A NULL pointer will make the given selection generate a random pattern
// N.B. counting on initialization putting zeros in the remaining slots
Pattern sequences[MAX_SEQUENCES][MAX_PATTERNS] = {
  {
    {0x00, 1},
    {0x01, 1},
    {0x02, 1},
    {0x04, 1},
    {0x08, 1},
    {0x10, 1},
    {0x20, 1},
    {0x40, 1},
    {0x80, 1},
    {0x40, 1},
    {0x20, 1},
    {0x10, 1},
    {0x08, 1},
    {0x04, 1},
    {0x02, 1},
    {0x01, 1},
  },
};


// Read the selector switch and return the current setting
int getSelection() {
  return (((digitalRead(A4) == LOW) << 2) | ((digitalRead(A3) == LOW) << 1) | (digitalRead(A2) == LOW));
}


// Read the speed pot and return the delay value in msec
// Min=0 (no delay) and Max=1024 (~1sec)
int getSpeed() {
  return analogRead(SPEED_POT);
}


void setup() { 
  Serial.begin(9600);
  delay(500);

  pinMode(CHANNEL_A, OUTPUT);
  pinMode(CHANNEL_B, OUTPUT);
  pinMode(CHANNEL_C, OUTPUT);
  pinMode(CHANNEL_D, OUTPUT);
  pinMode(CHANNEL_E, OUTPUT);
  pinMode(CHANNEL_F, OUTPUT);
  pinMode(CHANNEL_G, OUTPUT);
  pinMode(CHANNEL_H, OUTPUT);

  pinMode(STATUS_LED, OUTPUT);

  pinMode(SELECT_B0, INPUT_PULLUP);
  pinMode(SELECT_B1, INPUT_PULLUP);
  pinMode(SELECT_B2, INPUT_PULLUP);

  // use floating input as source of randomness
  randomSeed(analogRead(UNUSED_ANALOG));

  if (VERBOSE) {
    Serial.println("Start");
  }
}


void loop() {
  int i, speed, randEnbs, selection, delayTime;
  Pattern *seqPtr;

  digitalWrite(13, (statusLED & 0x01));
  statusLED++;

  selection = getSelection();
  if (VERBOSE) {
    Serial.print("Selection: ");
    Serial.println(selection, DEC);
  }

  seqPtr = sequences[selection];
  if ((seqPtr->enables == 0) && (seqPtr->baseDelay == 0x00)) {
    randEnbs = random(0, 255);
    delayTime = getSpeed() + RAND_PATT_DELAY;
    if (VERBOSE) {
      Serial.println("Pattern: 0x" + String(randEnbs, HEX) + ", Delay: " + String(delayTime));
    }
    for (i = 0; i < NUM_CHANNELS; i++) {
      digitalWrite(i + 2, ((randEnbs >> i) & 0x01));
    }
    delay(delayTime);
  } else {
    // loop over sequence
    while (!((seqPtr->enables == 0) && (seqPtr->baseDelay == 0x00))) {
      if (getSelection() != selection) {
        break;
      }
      delayTime = (seqPtr->baseDelay * DELAY_INTERVAL) + getSpeed();
      if (VERBOSE) {
        Serial.println("Pattern: 0x" + String(seqPtr->enables, HEX) + ", Delay: " + String(delayTime));
      }
      for (i = 0; i < NUM_CHANNELS; i++) {
        digitalWrite(i + 2, ((seqPtr->enables >> i) & 0x01));
      }
      delay(delayTime);
      seqPtr++;
    }
  }
}
