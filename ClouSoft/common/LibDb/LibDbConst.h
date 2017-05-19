/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LibDbConst.h
 * 摘    要：本文件主要实现标准系统库的常量定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 *********************************************************************************************************/
#ifndef LIBDBCONST_H
#define LIBDBCONST_H

#define BANK0        0
#define BANK1        1
#define BANK2        2
#define BANK3		 3
#define BANK4        4
#define BANK5		 5
#define BANK6		 6
#define BANK7        7
#define BANK8		 8
#define BANK9		 9
#define BANK10       10
#define BANK11		 11
#define BANK12		 12
#define BANK13		 13
#define BANK14		 14
#define BANK15		 15
#define BANK16		 16
#define BANK17		 17
#define BANK18		 18
#define BANK19		 19
#define BANK20		 20
#define BANK21		 21
#define BANK22		 22
#define BANK23		 23
#define BANK24		 24
#define BANK25		 25

#define BN0     0
#define BN1     1
#define BN2     2
#define BN3		3
#define BN4     4
#define BN5		5
#define BN6		6
#define BN7     7
#define BN8		8
#define BN9		9
#define BN10    10
#define BN11	11
#define BN12	12
#define BN13	13
#define BN14	14
#define BN15	15
#define BN16	16
#define BN17	17
#define BN18	18
#define BN19	19
#define BN20	20
#define BN21	21
#define BN22	22
#define BN23	23
#define BN24	24
#define BN25	25
#define BN26	26
#define BN27	27
#define BN28	28
#define BN29	29
#define BN30	30

#define POINT0       0
#define POINT1       1

#define PN0       0
#define PN1       1
#define PN2       2
#define PN3       3
#define PN4       4
#define PN5       5
#define PN6       6
#define PN7       7
#define PN8       8
#define PN9       9
#define PN10      10
#define PN11      11
#define PN12      12
#define PN13      13
#define PN14      14
#define PN15      15
#define PN16      16
#define PN17      17
#define PN18      18
#define PN19      19
#define PN20      20
#define PN21      21
#define PN22      22
#define PN23      23
#define PN24      24
#define PN25      25
#define PN26      26
#define PN27      27
#define PN28      28
#define PN29      29
#define PN30      30
#define PN31      31
#define PN32      32
#define PN33      33
#define PN34      34
#define PN35      35
#define PN36      36
#define PN37      37
#define PN38      38
#define PN39      39
#define PN40      40
#define PN41      41
#define PN42      42
#define PN43      43
#define PN44      44
#define PN45      45
#define PN46      46
#define PN47      47
#define PN48      48
#define PN49      49


#define IMG0		0

#define SECT0	0
#define SECT1	1
#define SECT2	2
#define SECT3	3
#define SECT4	4
#define SECT5	5
#define SECT6	6
#define SECT7	7
#define SECT8	8
#define SECT9	9
#define SECT10	10
#define SECT11	11
#define SECT12	12
#define SECT13	13
#define SECT14	14
#define SECT15	15
#define SECT16	16
#define SECT17	17

#define BN_VER_LEN		20	//BANK数据版本控制保存的字节长度

//测量点动态映射
#define PNUNMAP			0	//测量点不进行映射
#define PNMAP1			1	//测量点映射方案1

//#define PNMAP_PATHNAME	USER_PARA_PATH"PNMAP%d.cfg"
#define UPG_LOG_LEN		32

//一些最大配置,一般情况下不用改
#define SECT_MAX		    32
#define BANK_MAX		    32
#define BANK_FILE_MAX		200							//一个BANK最大支持的文件数量128
#define BANK_FILE_FLG_SIZE	((BANK_FILE_MAX+7)/8)		//一个BANK文件标志位数组的大小
#define PNMAP_MAX			32							//最多支持的映射方案数

#endif //LIBDBCONST_H