#include "simpletools.h"
#include "servo.h"

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
  print("Left sensor: %d | Right sensor: %d\n", left, right);
}

typedef enum { FORWARDS, BACKWARDS, TURN_LEFT, TURN_RIGHT } MoveStates;

void move(MoveStates state) {
  // a is the left servo, b is the right servo 
  // positive to turn the servo clockwise, negative to turn counter-clockwise

  int a, b;
  if (state == FORWARDS)   { a =  100; b =  100; }
  if (state == BACKWARDS)  { a = -100; b = -100; }
  if (state == TURN_LEFT)  { a =  100; b = -100; }
  if (state == TURN_RIGHT) { a = -100; b =  100; }

  servo_speed(26, a);
  servo_speed(27, b);
}

// This might actually be working...
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

int main() {
  while (1) {
    move_robot_inside_ring();
  }
}
