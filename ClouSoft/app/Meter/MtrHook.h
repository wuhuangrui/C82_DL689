/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrHook.h
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年4月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *			 $在这里不要定义成inline,方便和库文件一起编译时重定位
 *********************************************************************************************************/
#ifndef MTRHOOK_H
#define MTRHOOK_H
#include "apptypedef.h"
#include "Comm.h"

//在故障的发生/恢复时候的回调函数,用来生成告警事件
void On485ErrEstb(WORD wPort);		//485故障确认(所有测量点,分端口)
void On485ErrRecv(WORD wPort);		//485故障恢复(所有测量点,分端口)
void OnMtrErrEstb(WORD wPn);	//抄表故障确认(单个测量点)
void OnMtrErrRecv(WORD wPn);	//抄表故障恢复(单个测量点)

bool IsMtrErr(WORD wPn);			//用来查询测量点故障状态	
void DoMtrAnd485ErrErc();

//描述:获取485测量点信息,不包括通过载波采集器采集的485表
void Get485PnMask(BYTE* pbNodeMask);
const BYTE* Get485PnMask();

void GetPlcPnMask(BYTE* pbNodeMask);
const BYTE* GetPlcPnMask();

void Set485PnMask(WORD wPn);
void Clr485PnMask(WORD wPn);

void SetPlcPnMask(WORD wPn);
void ClrPlcPnMask(WORD wPn);

WORD MtrAddrToPn(const BYTE* pbTsa, BYTE bAddrLen);

#endif //MTRHOOK_H
