#ifndef RTC8025_H
#define RTC8025_H
#include "ComStruct.h"   //..\cl790dsoft\

typedef struct
{
	unsigned char  second;//
	unsigned char  minute;//
	unsigned char  hour;//
	unsigned char  date;//		日期
	unsigned char  month;//
	unsigned char  year;//
	unsigned char  wday;//		星期
}tm_t;

class RTC8052		//继承iicBus操作类
{
private:
	
public:	
	RTC8052();

	void Read8052Time(void);

	bool setTimer(tm_t &time);		//设置时钟
	void AdjustRx8025saClockPrecision(void);//温度补偿
	bool readTimer(tm_t &time);
	void updateTimer(bool fPowerOff);			//刷新时钟
	void Init(void);
};

extern RTC8052 rtc;
#endif
