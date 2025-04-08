#include "simpletools.h"

// below this it's white, above this it's black
const int WHITE_BLACK_THRESHOLD = 230;

int QTIs_Front() {
  // clock cycles???
  int dtqti = ((((CLKFREQ) / 1000000)) * WHITE_BLACK_THRESHOLD);

  // set pin 13 and 14 high and wait
  set_outputs(14, 13, 0b11);
  set_directions(14, 13, 0b11);
  waitcnt(CNT + dtqti);

  // set pin 13 and 14 low and wait
  set_directions(14, 13, 0b00);
  waitcnt(CNT + dtqti);
 
  // read pins 13 and 14 and put the info in 1 number
  // the first bit of the number -> left
  // the second bit of the number -> right
  // if the bit is 0 -> white, if it's 1 -> black
  int item = (get_states(14, 13));
  int qtis = ((item << 1) | (item >> 1)) & 3;
  return qtis;
}

int main() {
  const int WHITE_THRESHOLD = 115;
  const int BLACK_TRESHOLD = 460; 
 
  while (1) {
    term_cmd(CLS);
 
    // set the pin to high, pause for 1 millisecond and
    // then measure the amount of time it takes the pin to
    // go back low. the lower the time, the brighter the surface
    // the higher the time, the darker the surface (reflection)
    high(13);
    pause(1);
    int left = (rc_time(13, 1));

    high(14);
    pause(1);
    int right = (rc_time(14, 1));

    int value = QTIs_Front();
    print("Left: %d | Right: %d | Value: %02b\n", left, right, value);
    pause(1000);
  }
}
