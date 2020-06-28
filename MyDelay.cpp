#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "MyDelay.h"

myDelay::myDelay(void) {
  preMills = 0;
  setDelay(0);
}

myDelay::myDelay(unsigned long dtime) {
  preMills = 0;
  setDelay(dtime);
}

myDelay::myDelay(unsigned long dtime, funTocall funcall) {
  preMills = 0;
  setDelay(dtime);
  _funcall = funcall;
  useFunction = true;
}

myDelay::myDelay(unsigned long dtime, funTocall funcall, int repeatCount) {
  preMills = 0;
  setDelay(dtime);
  setRepeat(repeatCount);
  _funcall = funcall;
  useFunction = true;
}

void myDelay::setDelay(unsigned long dtime) {
  delaytime = dtime;
}

void myDelay::setRepeat(int repeatCount) {
  initialRepeatCount = repeatCount;
  currentRepeatCount = repeatCount;
  repeating = true;
}

void myDelay::start() {
  preMills = millis();
  currentRepeatCount = initialRepeatCount;
  running = true;
}

void myDelay::stop() {
  preMills = 0;
  running = false;
}

bool myDelay::update() {
  if (running) {
    curMills = millis();
    if (curMills - preMills >= delaytime) {
      if (repeating) {
        if (initialRepeatCount != MYDELAY_REPEAT_FOREVER) {
          currentRepeatCount--;
          if (currentRepeatCount == 0) {
            stop();
          }
        }
      }
      preMills = curMills;
      if (useFunction == true) {
        _funcall();
        return true;
      } else {
        return true;
      }
    } else  {
      return false;
    }
  } else {
    return false;
  }
}

bool myDelay::isRunning() {
  return running;
}
