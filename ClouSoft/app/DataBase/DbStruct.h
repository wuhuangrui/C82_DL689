/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbStruct.h
 * 摘    要：本文件主要用来定义各版本特殊的数据结构
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 *********************************************************************************************************/
#ifndef DBSTRUCT_H
#define DBSTRUCT_H
#include "sysarch.h"
#include "LibDbStruct.h"

typedef struct{
	BYTE  bValid:1;        				//有效的标志  
	BYTE  bNotFix:7;					//长度是否固定的标志
	BYTE  bBank;        				//对应Bank标识    
    WORD  wID;         					//对应ID标识  
	BYTE  bType;						//PN类型  
	WORD  wPNMax;						//PN数量  
}TConvertFNIDDesc;

#define PARSE_MAX	64
typedef struct{
	//数组或结构的具体数据内容
	BYTE* 	pbCfg;				//数组或结构的具体数据内容
								//如普通采集方案：记录列选择  array CSD
								//的具体配置内容
	WORD wCfgLen;				//数组或结构的长度

	//对配置字段解析的结果
	WORD 	wNum;					//解析出来的字段个数
	WORD 	wPos[PARSE_MAX];			//字段的起始位置
	WORD	wLen[PARSE_MAX];			//字段的数据长度
	BYTE	bType[PARSE_MAX];		//字段数据类型

	//配置的字段进一步解析成对应数据项的信息
	WORD  wTotalLen;				//所有数据项的总长度
	WORD  wItemOffset[PARSE_MAX];	//数据项在记录中的偏移，
	WORD  wItemLen[PARSE_MAX];		//数据项在记录中的长度，
	//通过OoGetDataLen()取得
}TFieldParser;	//字段解析器

#endif //DBSTRUCT_H
