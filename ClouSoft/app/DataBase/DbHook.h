/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbHook.h
 * 摘    要：本文件主要用来定义系统库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *********************************************************************************************************/
#ifndef DBHOOK_H
#define DBHOOK_H
#include "apptypedef.h"

/////////////////////////////////////////////////////////////////////////
//系统库的代码库需要的挂钩/回调函数定义
bool IsPnValid(WORD wPn);
WORD* CmbToSubID(WORD wBn, WORD wID);
int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, BYTE bPerm, BYTE* pbPassword, int nRet);
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet);
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet);
int PostReadItemValHook(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwTime, int nRet);
int PostReadItemVal64Hook(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwTime, int nRet);
bool PswCheck(BYTE bPerm, BYTE* pbPassword);
int PermCheck(TItemDesc* pItemDesc, BYTE bPerm, BYTE* pbPassword);

//描述:数据段是否会存放采集测量点(比如交采,脉冲)的实时数据,
//	   数据库在带时标读取采集测量点的该段内数据时,会自动把时标失效
//	   以实现交采脉冲数据的时标不管,默认当前数据就是最新符合的数据
bool IsSectHaveSampleData(WORD wSect);
BYTE GetInvalidData(BYTE bErr=ERR_OK); 	//获取本系统的无效数据的定义
bool IsInvalidData(BYTE* p, WORD wLen);	//是否是无效数据，无效数据可能存在多种定义
void GetMtrProCfgPath(char* pbCfgPath);//获取本系统的电表配置文件的路径
void DefaultUpgFun(WORD wFrmBn, WORD wFrmId, WORD wToBn, WORD wToId, int iPnNum);
BYTE GetDbInvalidData(); //获取本系统的无效数据的定义

/////////////////////////////////////////////////////////////////////////
//在实现挂钩/回调函数时需要额外定义的函数
WORD CmbToSubIdNum(WORD wBn, WORD wID);


#endif //DBHOOK_H

