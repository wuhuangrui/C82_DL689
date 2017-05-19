#ifndef DCSTRUCT_H
#define DCSTRUCT_H
#include "apptypedef.h"
#include "LibDbStruct.h"

typedef void (* TPfnDcValToFmt)(int val, BYTE* pbBuf, WORD wLen);

typedef struct{
	bool fDuoPn;  	//该数据项是否支持双测量点号入库,
					//主要是针对国标交采的测量点号可以配置,固定入测量点0,另外一个根据配置来入
					//其它BANK的扩展的数据项,由于都是自己用的数据项,所以一般都不支持双测量点
	WORD wBn;  		//BANK号
	WORD wID;     	//数据项ID,
					//如果为块ID,则wInnerID配为第一个ID的索引,wSubNum为子ID的个数
	WORD wIdx;		//内部计算的索引
	WORD wSubNum;	//子ID的个数
	WORD wLen;		//单个数据项的长度
	TPfnDcValToFmt pfnDcValToFmt;	//格式转换函数
	
	//以下部分由程序自动初始化
	TDataItem diPn0;
	TDataItem diPn;
}TDcValToDbCtrl;	//交采数据入库控制

#endif
