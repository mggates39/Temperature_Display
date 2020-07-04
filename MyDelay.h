#ifndef _MyDelay_h_
#define _MyDelay_h_
#define MYDELAY_REPEAT_FOREVER -1

typedef void (*funTocall)(void);

class myDelay {
  public:
    myDelay(void);
    myDelay(unsigned long dtime);
    myDelay(unsigned long dtime, funTocall funcall);
    myDelay(unsigned long dtime, funTocall funcall, int repeatCount);


    void setDelay(unsigned long);
    void setCallback(funTocall);
    void setRepeat(int);
    bool update();
    bool isRunning();
    void start();
    void stop();

  private:
    unsigned long _preMills, _curMills, _delaytime;
    int _initialRepeatCount = MYDELAY_REPEAT_FOREVER;
    int _currentRepeatCount = 0;
    funTocall _funcall;
    bool _useFunction = false;
    bool _running = false;
    bool _repeating = true;
};

#endif
