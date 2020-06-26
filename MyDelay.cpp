#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "MyDelay.h"

myDelay::myDelay(void)
{

    preMills = 0;
    setdelay(0);
}
myDelay::myDelay(unsigned long dtime)
{
    preMills = 0;
    setdelay(dtime);
}

myDelay::myDelay(unsigned long dtime, funTocall funcall)
{
    preMills = 0;
    setdelay(dtime);
    _funcall = funcall;
    use_function = true;
}

myDelay::myDelay(unsigned long dtime, funTocall funcall, int repeatCount)
{
    preMills = 0;
    setdelay(dtime);
    _funcall = funcall;
    use_function = true;
    initialRepeatCount = repeatCount;
    repeating = true;
}

void myDelay::setdelay(unsigned long dtime)
{
    delaytime = dtime;
}

void myDelay::start()
{
    preMills = millis();
    running = true;
}

void myDelay::stop()
{
    preMills = 0;
    running = false;
}

bool myDelay::update()
{
    if (running)
    {
        curMills = millis();
        if (curMills - preMills >= delaytime)
        {
            preMills = curMills;
            if (use_function == true)
            {
                _funcall();
                return true;
            }
            else
            {
                return true;
            }
        }

        else
            return false;
    }
    else
        return false;
}
