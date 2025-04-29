#include "simpletools.h"
#include "servo.h"
#include "abvolts.h"

// Potential techniques
// - Move at varying speeds -- use CR servo set ramp
// - Search for the opponent in a pattern, then when we detect them,
//   charge as fast as possible to try to push them out
// - Ensure that we can't leave the ring
// - Attack in angled arcs so we're harder to detect
// - Clean the robot as much as possible so that the opponent's IR sensors
//   have a harder time detecting us
// - Lower the center of gravity of the robot so that it's harder to destabilize it
// - Detect when we're being pushed and try to fight back
// - Try to tip the other robot so that we throw off their sensors and mess them up
// - Detect when we're being tipped
// - If it's drifting, change batteries

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
int isWhite(int value) {
  return value < 115;
}

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

void attackOpponent()
{
  int leftIR, rightIR;
  read_infrared_sensors(&leftIR, &rightIR);
  print("Left IR sensor: %d | Right IR sensor: %d\n", leftIR, rightIR);

  if (leftIR < 7 && rightIR < 7) {
    // TODO: how do we know when we're in a deadlock -- not moving while trying to push the opponent???
    if (leftIR <= 1 && rightIR <= 1) {
      move(BACKWARD);
      pause(100);
   } else {  
     move(FORWARD);
      pause(500);
    }    
  }    
  else if (leftIR < 7 && rightIR >= 7)
    move(LEFT);
  else if (leftIR >= 7 && rightIR < 7)
    move(RIGHT);
  else { // TODO: might be better to spin around the circle
    move(LEFT);
  }
}  

void navigate()
{
  int leftQTI, rightQTI;
  read_qti_sensors(&leftQTI, &rightQTI);
  //print("Left QTI sensor: %d | Right QTI sensor: %d\n", leftQTI, rightQTI);

  // Stay inside the ring
  if (isWhite(leftQTI) && isWhite(rightQTI))
    move(BACKWARD);
 
  if (isWhite(leftQTI) && !isWhite(rightQTI)) {
    move(BACKWARD);
    pause(500);
    move(RIGHT);
  }

  if (!isWhite(leftQTI) && isWhite(rightQTI)) {
    move(BACKWARD);
    pause(500);
    move(LEFT); 
  }

  if (!isWhite(leftQTI) && !isWhite(rightQTI))
    attackOpponent();
}

int main() {
  da_init(23, 23);
  move(CENTER);
  pause(1000);
  while (1) {
    navigate();
  }    
}
