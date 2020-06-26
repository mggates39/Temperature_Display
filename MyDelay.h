
#ifndef _MyDelay_h_
#define _MyDelay_h_
#define MYDELAY_REPEAT_FOREVER -1
typedef void (*funTocall)(void);
class myDelay{
	public:
	myDelay(void);
	myDelay(unsigned long dtime);
	myDelay(unsigned long dtime, funTocall funcall);
    myDelay(unsigned long dtime, funTocall funcall, int repeatCount);
	
	
	void setdelay(unsigned long);
	bool update();
	bool isRunnint();
	void start();
    void stop();
	
	private:
	unsigned long preMills, curMills, delaytime;
    int initialRepeatCount = MYDELAY_REPEAT_FOREVER;
    int repeatCount;
	funTocall _funcall;
	bool use_function = false;
    bool running = false;
    bool repeating = true;
};

#endif