/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Info.h
 * 摘    要：本文件实现应用层各线程间消息通信的机制
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备注：通知消息的原理
 * 		1.用来在参数或系统状态发生改变时,通知某个线程做相应更新
 × 		3.发起者多次把消息置成true,对接收者来说相当于只收到一个消息
 × 		4.当一个消息要发给多个线程时,应该每个线程都分配一个消息的标识
 *********************************************************************************************************/
#ifndef INFO_H
#define INFO_H

#include "apptypedef.h"

//通知消息:
void InitInfo();
void SetInfo(WORD wID, bool fInfo=true);
bool GetInfo(WORD wID);
void SetDelayInfo(WORD wID);

#endif //INFO_H
