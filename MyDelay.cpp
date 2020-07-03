#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "MyDelay.h"

myDelay::myDelay(void) {
  this->preMills = 0;
  this->setDelay(0);
}

myDelay::myDelay(unsigned long dtime) {
  this->preMills = 0;
  this->setDelay(dtime);
}

myDelay::myDelay(unsigned long dtime, funTocall funcall) {
  this->preMills = 0;
  this->setDelay(dtime);
  this->setCallback(funcall);
}

myDelay::myDelay(unsigned long dtime, funTocall funcall, int repeatCount) {
  this->preMills = 0;
  this->setDelay(dtime);
  this->setCallback(funcall);
  this->setRepeat(repeatCount);
}

void myDelay::setDelay(unsigned long dtime) {
  this->delaytime = dtime;
}

void myDelay::setCallback(funTocall funcall) {
  if (funcall != 0) {
    this->_funcall = funcall;
    this->useFunction = true;
  } else {
    this->useFunction = false;
  }
}

void myDelay::setRepeat(int repeatCount) {
  this->initialRepeatCount = repeatCount;
  this->currentRepeatCount = repeatCount;
  this->repeating = true;
}

void myDelay::start() {
  this->preMills = millis();
  this->currentRepeatCount = initialRepeatCount;
  this->running = true;
}

void myDelay::stop() {
  this->preMills = 0;
  this->running = false;
}

bool myDelay::update() {
  if (this->running) {
    this->curMills = millis();
    if (this->curMills - this->preMills >= this->delaytime) {
      if (this->repeating) {
        if (this->initialRepeatCount != MYDELAY_REPEAT_FOREVER) {
          this->currentRepeatCount--;
          if (this->currentRepeatCount == 0) {
            stop();
          }
        }
      }
      this->preMills = this->curMills;
      if (this->useFunction == true) {
        this->_funcall();
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
  return this->running;
}
