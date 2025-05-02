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
  if (state == LEFT)  { a = 0; b = -100; }
  if (state == RIGHT) { a = 100; b = 0; }

  // Center the servos
  // NOTE: the servo should be manually calibrated before hand
  if (state == CENTER) a = b = 0;

  servo_speed(26, a);
  servo_speed(27, b);
}

int speed = 50;
int max_speed = 100;

void gradually_speed_up(MoveStates state, int step) {
  servo_speed(26, speed);
  servo_speed(27, -speed);
  if (speed < max_speed)
    speed += step;
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

// Write the IR sensor values for each sensor (p4, p5, p6, p7)
// into the sensors array
// The values should range from 0 to 8, where 0 means the
// obstacle is very close, and 8 means that the obstacle
// wasn't detected at all.
void read_infrared_sensors(int* sensors) {
  // Accumulate the left and right receiver outputs
  // The more we iterate, the more the sensors get nearsighted,
  // so it's seeing closer and closer.
  for (int i = 0; i <= 140; i += 20) {
    da_out(0, i);
    pause(2);
    for (int j = 0; j < 4; j++) {
      // Write to the ir led, read from the ir receiver
      // For example: write to p5, read from p1
      freqout(4 + j, 1, 38000);
      sensors[j] += input(j);
    }
  }
}

// Randomly return true or false
unsigned short lfsr = 0xACE1u;
unsigned bit;
unsigned random(){
  bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
  int value = lfsr =  (lfsr >> 1) | (bit << 15);
  return value < 10000;
}

// I don't think there's a way with the hardware we're allowed to detect
// whether we're being pushed or not. Which means that we're forced to be
// aggressive, rather than offensive.
// Are there more sensors???? -- what if we put sensors on the sides too?
void attack_opponent()
{
  int sensors[4] = {0, 0, 0, 0};
  read_infrared_sensors(sensors);

  for (int i = 0; i < 4; i++) {
    print("Sensor: #%d: %d\n ", i, sensors[i]);
  }

  int leftIR = sensors[1], rightIR = sensors[3];
  if (leftIR < 7) {
     move(FORWARD);
  }
  else if (leftIR >= 7)
    move(RIGHT);
  else { // TODO: might be better to spin around the circle
    move(CENTER);
  }
  /*
  if (leftIR < 7 && rightIR < 7) {
     move(FORWARD);
  }
  else if (leftIR < 7 && rightIR >= 7)
    move(LEFT);
  else if (leftIR >= 7 && rightIR < 7)
    move(RIGHT);
  else {
    // Move around looking for the opponent
    move(FORWARD);
    pause(100);
    move(RIGHT);
    if (random())
      move(LEFT);
    else
      move(RIGHT);
  }
  */
}

void navigate()
{
  int leftQTI, rightQTI;
  read_qti_sensors(&leftQTI, &rightQTI);
  print("Left QTI sensor: %d | Right QTI sensor: %d\n", leftQTI, rightQTI);

  // TODO: what if we decrease the pause times???

  // Stay inside the ring
  if (white(leftQTI) && white(rightQTI))
    move(BACKWARD);

  if (white(leftQTI) && !white(rightQTI)) {
    move(BACKWARD);
    pause(500);
    move(RIGHT);
  }

  if (!white(leftQTI) && white(rightQTI)) {
    move(BACKWARD);
    pause(500);
    move(LEFT);
  }

  if (!white(leftQTI) && !white(rightQTI))
    attack_opponent();
}

int main() {
  da_init(23, 23);
  move(CENTER);
  //pause(5000);
  while (1) {
    navigate();
  }
}
