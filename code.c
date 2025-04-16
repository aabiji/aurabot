#include "simpletools.h"
#include "servo.h".
#include "abvolts.h"

typedef enum { FORWARD, BACKWARD, LEFT, RIGHT, CENTER } MoveStates;

// At what distance should we respond to the opponent? (far off or super close (what if we're too late??))
// What happens when the opponent tips the robot forwards?
//  - The opponent disappears
//  - We should be doing that to our opponents
// Scanleft and right for an opponent if we can't see one

void move(MoveStates state) {
  int a, b; // a is left and b is right
  
  // The right wheel needs to move clockwise,
  // the left has to move counter clockwise
  if (state == FORWARD) { a = 100; b = -100; }
  
  // Opposite of moving forwards
  if (state == BACKWARD) { a = -100; b = 100; }

  // Turning
  if (state == LEFT)  { a = 0; b = -100; }
  if (state == RIGHT) { a = 100; b = 0; }

  // Center the servos
  if (state == CENTER) a = b = 0;

  servo_speed(26, a);
  servo_speed(27, b);
}

// determines how long to wait before deciding
// whether the surface is white or black
// below this it's white, above this it's black
const int COLOR_TIME_THRESHOLD = 230;

// detect the surface color using the left and right
// qti sensors and return the state of both sensors
int detect_surface_color() {
  // delay time expressed in clock cycles
  int dtqti = ((((CLKFREQ) / 1000000)) * COLOR_TIME_THRESHOLD);

  // set pin 13 and 14 high and wait
  set_outputs(14, 13, 0b11);
  set_directions(14, 13, 0b11);
  waitcnt(CNT + dtqti);
 
  // set pin 13 and 14 low and wait
  set_directions(14, 13, 0b00);
  waitcnt(CNT + dtqti);

  // read pins 13 and 14.
  // we've now waited at least `COLOR_TIME_THRESHOLD` microseconds
  // so if the pins are still high (1), it must be taking a long to discharge
  // which means that the sensor must be detecing black
  // if the pins low (0), it must have taken less time to discharge,
  // which means that the sensor must be detecting white
  int item = (get_states(14, 13));

  // store the pin state for the left sensor in the first bit,
  // store the pin state for the right sensor in the second bit
  // ex: 10 means that left is black, right is white
  int qtis = ((item << 1) | (item >> 1)) & 3;
  return qtis;
}

void debug_qti_sensors() {
  high(13);
  pause(1);
  int left = rc_time(13, 1);
  
  high(14);
  pause(1);
  int right = rc_time(14, 1);

  // black should be in the thousands (or high like that)
  // white should be < 100 or something
  print("Left QTI sensor: %d | Right QTI sensor: %d\n", left, right);
}

// Use the IR sensors to detect an obstacle in front.
// Writes a value into leftValue and write a value into rightValue
// The values should range from 0 to 8, where 0 means the
// obstacle is very close, and 8 means that the obstacle
// wasn't detected at all.
void read_infrared_sensors(int* leftValue, int* rightValue) {
  int left = 0;
  int right = 0;

  // Accumulate the left and right receiver outputs
  // The more we iterate, the more the sensors get nearsighted,
  // so it's seeing closer and closer.
  for (int i = 0; i <= 140; i += 20) {
    da_out(0, i);
    pause(2);
    // Tally the amount of times the left sensor DID NOT detect something
    freqout(5, 1, 38000);
    left += input(1);
    // Tally the amount of times the right sensor DID NOT detect something
    freqout(6, 1, 38000);
    right += input(2);
  }

  *leftValue = left;
  *rightValue = right;
}

void navigate()
{
  int left, right; // ir sensors
  read_infrared_sensors(&left, &right);
  print("Left IR sensor: %d | Right IR sensor: %d\n", left, right);

  if (left < 7 && right < 7)
    move(FORWARD);
  else if (left < 7 && right >= 7)
    move(LEFT);
  else if (left >= 7 && right < 7)
    move(RIGHT);
  else
    move(CENTER);
}

/*
// This might actually be working...
// probably not anymore
void move_robot_inside_ring() {
  int value = detect_surface_color();

  switch (value) {
    case 0b11: // left & right see black
      move(TURN_LEFT);
      break;
    case 0b10: // left sees black, right sees white
      move(TURN_RIGHT);
      pause(600);
      move(BACKWARDS);
      pause(600);
      break;
    case 0b01: // left sees white, right sees black
      move(TURN_RIGHT);
      pause(600);
      move(FORWARDS);
      pause(600);
      break;
   case 0b00: // left & right see white
      move(TURN_RIGHT);
      pause(600);
      move(BACKWARDS);
      pause(600);
      break;
  }
}
*/

int main() {
  pause(1000);
  da_init(23, 23);
  move(CENTER);
  while (1) {
    navigate(); 
  }
}
