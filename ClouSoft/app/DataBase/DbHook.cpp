/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbHook.cpp
 * 摘    要：本文件主要用来定义系统库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *********************************************************************************************************/
#include "stdafx.h"
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "MeterAPI.h"
#include "DataManager.h"
#ifdef SYS_LINUX
#include "AcSample.h"
#include "Sample.h"
#endif
#define CMB_TO_SUB_ID_MAX	45
static WORD g_wCmbToSubID[][CMB_TO_SUB_ID_MAX] =		//DLMS BANK0内部ID到645ID的映射
{
	{0x7000, 0x109f, 0x10Af, 0x110f, 0x111f}, //当前总加有/无功功率及剩余电量	
	{0x7001, 0x8910, 0xb63f, 0xb64f, 0xb65f, 0xb61f, 0xb62f, 0xb6a0, 0xb67f,  0}, //当前三相及总有/无功功率、功率因数，三相电压、电流、零序电流，视在功率
	{0x7002, 0x8910, 0xb31f, 0xb32f, 0xb33f, 0xb34f, 0},		 //A、B、C三相断相统计数据及最近一次断相记录
//	{0x7003, 0x8910, 0xc701, 0xc700, 0xc870, 0xc810, 0xc811, 0xc820, 0xc821, 0xc830, 0xc831, 0xc840, 0xc841, 0xc850, 0xc851, 0}, //电能表日历时钟及电能表状态信息
	{0x7003, 0x8910, 0xc01f, 0xc870, 0xc810, 0xc811, 0xc820, 0xc821, 0xc830, 0xc831, 0xc840, 0xc841, 0xc850, 0xc851, 0}, //电能表日历时钟及电能表状态信息

	{0x7006, 0x8910, 0x901f, 0x902f, 0x911f, 0x912f, 0x913f, 0x915f, 0x916f, 0x914f, 0}, //当前正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）	
//	{0x7007, 0x8910, 0x941f, 0x942f, 0x951f, 0x952f, 0x953f, 0x955f, 0x956f, 0x954f, 0xc117, 0}, //上一计算日正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）	
	{0x7007, 0x8910, 0x941f, 0x942f, 0x951f, 0x952f, 0x953f, 0x955f, 0x956f, 0x954f, 0}, //上一计算日正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）	
	{0x7008, 0x8910, 0xa010, 0xb010, 0xa011, 0xb011, 0xa012, 0xb012, 0xa013, 0xb013, 0xa014, 0xb014,
	                 0xa020, 0xb020, 0xa021, 0xb021, 0xa022, 0xb022, 0xa023, 0xb023, 0xa024, 0xb024, 
					 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 
					 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0}, //当月正、反向有/无功最大需量及发生时间（总、费率1~M）
	{0x7009, 0x8910, 0xa410, 0xb410, 0xa411, 0xb411, 0xa412, 0xb412, 0xa413, 0xb413, 0xa414, 0xb414,
	                 0xa420, 0xb420, 0xa421, 0xb421, 0xa422, 0xb422, 0xa423, 0xb423, 0xa424, 0xb424, 
					 0xa510, 0xb510, 0xa511, 0xb511, 0xa512, 0xb512, 0xa513, 0xb513, 0xa514, 0xb514, 
					 0xa520, 0xb520, 0xa521, 0xb521, 0xa522, 0xb522, 0xa523, 0xb523, 0xa524, 0xb524, 0xc117, 0}, //上一计算日正/反向有/无功最大需量及发生时间（总、费率1~M）

//	{0x700a, 0x901f, 0x902f, 0x911f, 0x912f, 0x913f, 0x915f, 0x916f, 0x914f, 0},//日冻结正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）	
//	{0x700a, 0x9a00, 0x9a1f, 0x9a2f, 0x9b1f, 0x9b2f, 0x9b3f, 0x9b5f, 0x9b6f, 0x9b4f, 0}, //日冻结正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）	
	{0x700a, 0x9a1f, 0x9a2f, 0x9b1f, 0x9b2f, 0x9b3f, 0x9b5f, 0x9b6f, 0x9b4f, 0}, //日冻结正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）	
	{0x700b, 0x8910, 0x941f, 0x942f, 0x951f, 0x952f, 0x953f, 0x955f, 0x956f, 0x954f, 0xc117, 0}, //上一计算日正/反向有/无功电能示值及一/四象限无功电能示值（总、费率1~M）----当前值，多了结算时间0xc117	

	{0x7010, 0x33Af, 0x33Bf, 0x328F, 0x329F},		//总加组上一15分钟曲线点	
//	{0x7011, 0xb630, 0xb631, 0xb632, 0xb633, 0xb640, 0xb641, 0xb642, 0xb643, 0xb650, 0xb651, 0xb652, 0xb653, 0xb611, 0xb612, 0xb613, 0xb621, 0xb622, 0xb623, 0xb6a0, 0},//测量点上一15分钟瞬时量曲线点	
	{0x7011, 0xb63f, 0xb64f, 0xb65f, 0xb61f, 0xb62f, 0xb6a0, 0},//测量点上一15分钟瞬时量曲线点	
	{0x7012, 0x9010, 0x9020, 0x9110, 0x9120, 0},	//测量点上一15分钟电能示值曲线点
	{0x7013, 0x32af, 0x32cf, 0x32bf, 0x32df, 0},	//测量点上一15分钟电量曲线点	

	{0x7221, 0x8910, 0xc9c1, 0xc9b0, 0xc9c5, 0xc990, 0xc991, 0xc9a5, 0xc9d0, 0xc9d1, 0xc9d2, 0},	//F167电能表购、用电信息	

	{0x7020, 0x10BF, 0x10CF, 0},					//总加组当日电能
	{0x7030, 0x10DF, 0x10EF, 0},					//总加组当月电能
	{0x7120, 0x321f, 0x322f, 0},					//总加组上一日电能	
	{0x7130, 0x324f, 0x325f, 0},					//总加组上一月电能	
	{0x7021, 0x11DF, 0x11EF, 0x120F, 0x121F, 0},	//测量点当日电能
	{0x7031, 0x122F, 0x123F, 0x124F, 0x125F, 0},	//测量点当月电能
	{0x7121, 0x300f, 0x302f, 0x301f, 0x303f, 0},	//测量点上一日电能,Delat量
	{0x7131, 0x304f, 0x306f, 0x305f, 0x307f, 0},	//测量点上一月电能,Delat量
	{0x7220, 0x371f, 0},							//直流模拟量的采集
	{0x5512, 0x1500, 0x1501, 0},					//当日当月通信流量
	//{0x5514, 0x5510, 0x5511, 0x5513, 0x1500, 0x1501, 0},	//终端状态
};


//描述:测量点参数是否配置有效
//配置格式如下：
//02 04 
//		12 00 04 
//		02 07 
//			85 06 10 00 00 00 22 24 
//			16 03 
//			16 03 
bool IsPnValid(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN+1];
	if (ReadItemEx(BN0, wPn, 0x6000,bBuf) > 0)
	{
		WORD wSn = OoOiToWord(&bBuf[4]);//((WORD )bBuf[4]<<8)+bBuf[3];
		BYTE bAddrLen = bBuf[9];
		if (wSn>0 && !IsAllAByte(&bBuf[9], 0x00, bAddrLen))
			return true;
	}
	
	return false;
}

//描述:取组合ID到子ID的映射数组
WORD* CmbToSubID(WORD wBn, WORD wID)
{
	if (wBn!=BN0
#ifdef EN_CCT	//允许集抄功能
		&& wBn!=CCT_BN_SPM && wBn!=CCT_BN_MFM	//集抄用到的BANK
#endif //EN_CCT
		)
		return NULL;
		
	WORD wNum = sizeof(g_wCmbToSubID) / (sizeof(WORD)*CMB_TO_SUB_ID_MAX);
	for (WORD i=0; i<wNum; i++)
	{
		if (wID == g_wCmbToSubID[i][0])
		{
			return &g_wCmbToSubID[i][1];
		}
	}
	
	return NULL;
}


//描述:取组合ID到子ID的个数
WORD CmbToSubIdNum(WORD wBn, WORD wID)
{
	WORD* pwSubID = CmbToSubID(wBn, wID);
	if (pwSubID == NULL)
		return 1;
	
	WORD wNum = 0;	
	while (*pwSubID++ != 0)	//把组合ID转换成依次对子ID的读
		wNum++;
	
	return wNum;
}

int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, BYTE bPerm, BYTE* pbPassword, int nRet)
{
	//BYTE bBuf[100], bOldBuf[100], bProp, bOldProp, bProType, bNum, bPn, bOldPn, bPulseProp;
	//WORD wNum, wSn, wPoint, wUsePn;//wPoint为下发的测量点号，wObj为序号，wUsePn为序号对应的已经使用的测量点号
	//BYTE* pbTmp;
	//WORD i, j, n, step, wFindObj;
	//bool fSame, fChg;
	BYTE bTmpBuf[80];

	if (wBank==BN0 && wID==0x4001)	//终端地址
	{		
		if (GetItemLen(BN10, 0xa1d0) > 0)
		{
			if (pbBuf[1] > TSA_LEN-2)	//pbBuf[1]为OCTSTRING长度，TSA_LEN-2=15
				pbBuf[1] = TSA_LEN-2;

			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			memcpy(bTmpBuf, pbBuf, pbBuf[1]+2);		//+2, 类型1Byte + 长度1Byte
			WriteItemEx(BN10, PN0, 0xa1d0, bTmpBuf);
		}
	}
	#ifdef SYS_LINUX
	else if(wBank==BN2 && wID == 0x5039)
	{
		AcSetPulseRatio(pbBuf);//不能调用InitSample();
		TraceBuf(DB_FAPROTO,"\r\nPostWriteItemExHook: wBank=BN2,wID=0x5039,pbBuf= ",pbBuf,1);

	}
	else if((wBank==BN25 && 
				((wID >= 0x5001 && wID <= 0x5005)
				  ||(wID == 0x500f) 
				  )
			)
			||
			(wBank==BN28 && 
				((wID >= 0x0011 && wID <= 0x0019)
				  ||(wID == 0x001f) 
				  )
			)
			
		  )
	{// 交采参数变化
		BYTE bCnt = 0;
		AcSetPulseRatio(&bCnt);//不能调用InitSample();
//			InitSample();
//			DTRACE(DB_FAPROTO,("PostWriteItemExHook: AC_PARA change-> InitSample\r\n"));
	}
	#endif

	return nRet;
}

//描述:读数据项的挂钩,在数据项被读后自动调用,进行一些非标准的操作
//参数:@nRet 在调用ReadItemEx()时的返回值,应该为正数
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet)
{
	BYTE bBuf[90]={0};
	//BYTE* pbTmp = pbBuf;
	WORD j;
	int len;
	DWORD dwIP = 0;
	BYTE  bConnType = 1;
	
	if (wBank==BN0 && wID==0x4010) //SAP要与逻辑设备名相关联
	{
		/*WORD wSap;

		pbBuf[0] = 2;
		wSap = 1;
		memcpy(pbBuf+1, (BYTE*)&wSap, 2);
		ReadItemEx(BN0, PN0, 0x4000, pbBuf+3); //管理逻辑设备名
		wSap = 17;
		memcpy(pbBuf+19, (BYTE*)&wSap, 2);
		ReadItemEx(BN0, PN0, 0x4001, pbBuf+21); //数据逻辑设备名*/
	}
	else if (wBank==BN0 && wID==0x100f) //版本信息里的终端设备名要与管理设备名相关联
	{
		ReadItemEx(BN0, PN0, 0x4000, pbBuf); //替换管理逻辑设备名
	}
	else if (wBank==BN0 && wID==0x4001)	//终端地址
	{
		ReadItemEx(BN10, PN0, 0xa1d0, pbBuf);
	}
	else if (wBank==BN0 && wID==0x4507)	//信号强度
	{
		ReadItemEx(BN2, PN0, 0x6003, pbBuf);
	}
	else if (wBank==BN0 && wID==0x4509)	//GPRS拨号本机IP
	{		
		ReadItemEx(BN1, PN0, 0x2032, &bConnType);
		if(bConnType == 1)
		{
#ifndef SYS_WIN
			dwIP = GetLocalAddr("ppp0");
#endif

			pbBuf[0] = DT_OCT_STR;
			pbBuf[1] = 0x04;
			pbBuf[2] = (BYTE)dwIP&0xff;
			pbBuf[3] = (BYTE)(dwIP>>8)&0xff;
			pbBuf[4] = (BYTE)(dwIP>>16)&0xff;
			pbBuf[5] = (BYTE)(dwIP>>24)&0xff;
		}
		else
		{
			ReadItemEx(BN2, PN0, 0x2055, bBuf);

			pbBuf[0] = DT_OCT_STR;
			pbBuf[1] = 0x04;
			memcpy(&pbBuf[2], bBuf, 4);
			
		}	
	}

	return nRet;
}


//描述:读组合数据项的挂钩,在数据项被读后自动调用,进行一些非标准的操作
//参数:@nRet 在调用ReadItemEx()时的返回值,应该为正数
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet)
{
	//NOTICE:目前所有的wID都出自g_wCmbToSubID[],故暂时不对wID进行判断
	if (wID == 0x126f) //相位角没有抄表时间
		return nRet;	

	WORD* pwSubID = CmbToSubID(wBank, wID);	

	//目前都把终端抄表时间直接替换为读函数带下来的时间
	TTime time;
	SecondsToTime(dwTime, &time);	
	if (*pwSubID==0x8910)
	{
		GetCurTime(&time);
		TimeToDateTime(time, pbBuf);	
	}	

	return nRet;
}

//描述:读组合数据项的挂钩,在数据项被读后自动调用,进行一些非标准的操作
//参数:@nRet 在调用ReadItemEx()时的返回值,应该为正数
int PostReadItemValHook(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwTime, int nRet)
{
	//NOTICE:目前所有的wID都出自g_wCmbToSubID[],故暂时不对wID进行判断
	if (wID == 0x126f) //相位角没有抄表时间
		return nRet;

	*piVal32 = dwTime;
	
	return nRet;
}

//描述:读组合数据项的挂钩,在数据项被读后自动调用,进行一些非标准的操作
//参数:@nRet 在调用ReadItemEx()时的返回值,应该为正数
int PostReadItemVal64Hook(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwTime, int nRet)
{
	//NOTICE:目前所有的wID都出自g_wCmbToSubID[],故暂时不对wID进行判断
	if (wID == 0x126f) //相位角没有抄表时间
		return nRet;

	*piVal64 = dwTime;
	
	return nRet;
}


bool PswCheck(BYTE bPerm, BYTE* pbPassword)
{
	/*
	if (!IsTimeEmpty(g_tmAccessDenied))
	{										//写被冻结24小时
		TTime now;
		GetCurTime(&now);
		if (MinutesPast(g_tmAccessDenied, now) < 24*60)
			return false;
		else								//冻结时间结束
			SetEmptyTime(&g_tmAccessDenied);
	}
	
	BYTE bPassword[3];
	if (bPerm == DI_LOW_PERM)
	{
        g_DataManager.ReadTermnPara(0x8021, bPassword);   //低级权限密码
	}
	else  //DI_HIGH_PERM
	{
		g_DataManager.ReadTermnPara(0x8022, bPassword);   //高级权限密码
	}

    if (pbPassword[0]!=bPassword[0] || pbPassword[1]!=bPassword[1] || pbPassword[2]!=bPassword[2])
	{
		g_wAccessDeniedCnt++;
		if (g_wAccessDeniedCnt >= 3)
		{
			GetCurTime(&g_tmAccessDenied);
			g_wAccessDeniedCnt = 0;
		}
		
	    return false;
	}
	
	g_wAccessDeniedCnt = 0;   //只要输入正确,错误计数归零
	*/

	return true;
}


int PermCheck(TItemDesc* pItemDesc, BYTE bPerm, BYTE* pbPassword)
{
	if (pbPassword == NULL)   //这里把带密码的调用视作外来调用，需要验证密码，本系统的读写则不作任何限制
		return ERR_OK;
		
	/*if (!IsTimeEmpty(g_tmAccessDenied))
	{										//写被冻结24小时
		TTime now;
		GetCurTime(&now);
		if (MinutesPast(g_tmAccessDenied, now) < 24*60)
			return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
		else								//冻结时间结束
			SetEmptyTime(&g_tmAccessDenied);
	}*/
	
	
	if (bPerm==DI_LOW_PERM && pItemDesc->wPerm==DI_HIGH_PERM)   //想使用低级权限密码访问高级权限数据
	{
		return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
	}

	if ((pItemDesc->wRW & DI_WRITE) != DI_WRITE)   //不允许写
	{
		return -(ERR_INVALID + (int )pItemDesc->wLen*0x100); //没别的更合适的错误代码，只能用设置内容非法
	}


	/*BYTE bPassword[3];
	if (bPerm == DI_LOW_PERM)
	{
        g_DataManager.ReadTermnPara(0x8021, bPassword);   //低级权限密码
	}
	else  //DI_HIGH_PERM
	{
		g_DataManager.ReadTermnPara(0x8022, bPassword);   //高级权限密码
	}

    if (pbPassword[0]!=bPassword[0] || pbPassword[1]!=bPassword[1] || pbPassword[2]!=bPassword[2])
	{
		g_wAccessDeniedCnt++;
		if (g_wAccessDeniedCnt >= 3)
		{
			GetCurTime(&g_tmAccessDenied);
			g_wAccessDeniedCnt = 0;
		}
		
	    return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
	}*/
	
	if (!PswCheck(bPerm, pbPassword))
	{
		return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
	}
	
	g_dwWrOpClick = GetClick();
	
	return ERR_OK;
}

bool IsSectHaveSampleData(WORD wSect)
{
#if 0
	if (wSect == SECT_PN_DATA)
		return true;
	else
#endif
		return false;

}


//描述:根据不同的错误定义bErr，获取本系统的无效数据的定义
//备注:例如江苏规定要区分不支持数据项(0xee)和抄表失败数据项(0xef)
BYTE GetInvalidData(BYTE bErr)
{
	return INVALID_DATA;
}

//描述:是否是无效数据，无效数据可能存在多种定义
bool IsInvalidData(BYTE* p, WORD wLen)
{
	if (IsAllAByte(p, INVALID_DATA, wLen))
		return true;

	return false;
}

//描述:获取本系统的电表配置文件的路径
void GetMtrProCfgPath(char* pbCfgPath)
{
	sprintf(pbCfgPath,"%s", USER_CFG_PATH);	
}

//描述:数据库升级用的默认转换函数
//参数:iPnNum 从新版本系统库中取出wToId的测量点个数
void DefaultUpgFun(WORD wFrmBn, WORD wFrmId, WORD wToBn, WORD wToId, int iPnNum)
{
	BYTE bBuf[1024];
	if (iPnNum <= 0)
		return;

	DWORD dwTime;
	for (WORD wPn=0; wPn<iPnNum; wPn++)
	{
		dwTime = 0;
		if (UpgReadItem(wFrmBn, wPn, wFrmId, bBuf, &dwTime) > 0)
		{
			WriteItemEx(wToBn, wPn, wToId, bBuf, dwTime);
		}
	}
}

//描述:获取本系统的无效数据的定义
BYTE GetDbInvalidData()
{
	return INVALID_DATA;	
}
