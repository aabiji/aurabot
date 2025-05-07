#include "simpletools.h"
#include "servo.h"
#include "abvolts.h"

typedef enum { FORWARD, BACKWARD, LEFT, RIGHT, CENTER } MoveStates;

void move(MoveStates state) {
  int a, b; // a is left and b is right. can range from -200 to 200

  // The right wheel needs to move clockwise,
  // the left has to move counter clockwise
  if (state == FORWARD) { a = 100; b = -100; }

  // Opposite of moving forwards
  if (state == BACKWARD) { a = -100; b = 100; }

  // Turning
  if (state == LEFT)  { a = -100; b = -100; }
  if (state == RIGHT) { a = 100; b = 100; }

  // Center the servos
  // NOTE: the servo should be manually calibrated before hand
  if (state == CENTER) a = b = 0;

  servo_speed(26, a);
  servo_speed(27, b);
}

// Read the left and the right QTI sensor
void read_qti_sensors(int* left, int* right) {
  high(13);
  pause(1);
  *left = rc_time(13, 1);

  high(14);
  pause(1);
  *right = rc_time(14, 1);
}

// Return 1 if the QTI sensor detects white, 0 otherwise
int white(int value) { return value < 115; }

// Write to the sensor at p5, read from p0
// Write to the sensor at p7, read from p2
void read_infrared_sensors(int* left, int* right) {
  // Accumulate the left and right receiver outputs
  // The more we iterate, the more the sensors get nearsighted,
  // so it's seeing closer and closer.
  for (int i = 0; i <= 140; i += 20) {
    da_out(0, i);
    pause(2);
   
    freqout(5, 1, 38000);
    *left += input(1);
    freqout(7, 1, 38000);
    *right += input(2);
  }  
}

int state  = 0;
int counter = 0;

void finderPattern() {
  // forward, left, right, forward, right, left
  int states[]    = { 0, 1, 0, 2, 0, 2, 0, 1 };
  int durations[] = { 8, 6, 8, 6, 8, 6, 8, 6 };

  counter++;
  if (counter > durations[state]) {
    counter = 0;
    state++;
    if (state >= sizeof(states) / sizeof(int)) {
      state = 0; // reset back to 0
    }      
  }      

  if (states[state] == 0)
    move(FORWARD);
  if (states[state] == 1)
    move(LEFT);
  else if (states[state] == 2)
    move(RIGHT); 
}  

void attack_opponent()
{
  int leftIR = 0, rightIR = 0;
  read_infrared_sensors(&leftIR, &rightIR);

  if (leftIR < 7 && rightIR < 7) // left & right detect
     move(FORWARD);
  else if (leftIR < 7 && rightIR >= 7) // left detects
    move(LEFT);
  else if (leftIR >= 7 && rightIR < 7) // right detects
    move(RIGHT);
  else
    finderPattern();
}

void navigate()
{
  int leftQTI, rightQTI;
  read_qti_sensors(&leftQTI, &rightQTI);

  if (!white(leftQTI) && !white(rightQTI)) {
    attack_opponent();
    return;
  }

  // Stay inside the ring
  if (white(leftQTI) && white(rightQTI)) {
    move(BACKWARD);
  }

  if (white(leftQTI) && !white(rightQTI)) {
    move(BACKWARD);
    pause(600);
    move(RIGHT);
  }

  if (!white(leftQTI) && white(rightQTI)) {
    move(BACKWARD);
    pause(600);
    move(LEFT);
  }
}

int main() {
  da_init(23, 23);
  move(CENTER);
  pause(5000);
  while (1) {
    navigate();
  }
}
