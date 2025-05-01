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

// TODO: test this -- for example with FORWARD, 50, 100, we should gradually speed up as we go forwards
// could we use servo_setramp instead???
void speed_up(MoveStates state, int min_speed, int max_speed, int step) {
  for (int i = min_speed; i <= max_speed; i += step) {
    if (state == FORWARD) {
      servo_speed(26,  i);
      servo_speed(27, -i);
      pause(50);
    }
  }
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

// Use the IR sensors to detect an obstacle in front.
// Writes a value into left and write a value into right
// The values should range from 0 to 8, where 0 means the
// obstacle is very close, and 8 means that the obstacle
// wasn't detected at all.
void read_infrared_sensors(int* left, int* right) {
  *left = *right = 0;

  // Accumulate the left and right receiver outputs
  // The more we iterate, the more the sensors get nearsighted,
  // so it's seeing closer and closer.
  for (int i = 0; i <= 140; i += 20) {
    da_out(0, i);
    pause(2);
    // Tally the amount of times the left sensor DID NOT detect something
    freqout(5, 1, 38000);
    *left += input(1);
    // Tally the amount of times the right sensor DID NOT detect something
    freqout(6, 1, 38000);
    *right += input(2);
  }
}

// I don't think there's a way with the hardware we're allowed to detect
// whether we're being pushed or not. Which means that we're forced to be
// aggressive, rather than offensive.
// Are there more sensors???? -- what if we put sensors on the sides too?
void attack_opponent()
{
  int leftIR, rightIR;
  read_infrared_sensors(&leftIR, &rightIR);

  print("Left IR sensor: %d | Right IR sensor: %d\n", leftIR, rightIR);

  // TODO: just using the left for now
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
    // TODO: Spin around in a circle until it finds the robot. When it finds it, it follows it
    move(LEFT);
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
    attackOpponent();
}

int main() {
  da_init(23, 23);
  move(CENTER);
  pause(5000);
  while (1) {
    navigate();
  }
}
