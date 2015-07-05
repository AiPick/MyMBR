/*
	Mode:Ring 3
	OS:Windows 7 x64
	Development: VS2013
	Language: C/C++
*/

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////


/*
	大部分资料都大致认为 BOOT 区代码占用 446 字节
	
	446	=	440		+	4			+	2
	       bootcode		Signature		reserved
*/

#define BOOTRECORDSIZE 440

typedef struct _BOOTRECORD
{
	unsigned char BootRecord[BOOTRECORDSIZE];
}BOOTRECORD, *PBOOTRECORD;

/*
	sizeof (DPT) = 16
	Total DPT	 = 16 * 4
*/

#define DPTSIZE 64
#define DPTNUMBER 4

typedef struct _DPT
{
	unsigned char Dpt[DPTSIZE];
}DPT, *PDPT;


typedef struct _DP
{
	unsigned char BootSign;         // 引导标志
	unsigned char StartHsc[3];
	unsigned char PartitionType;    // 分区类型
	unsigned char EndHsc[3];
	ULONG         SectorsPreceding;     // 本分区之前使用的扇区数
	ULONG         SectorsInPartition;   // 分区的总扇区数
}DP, *PDP;

/*
	MBR	=	BOOTCODE	+	DPT		+	SIGNATURE
	SIZEOF(MBR)	=	512B
*/

typedef struct _MBR
{
	BOOTRECORD BootRecord;                  // 引导程序
	unsigned char ulSigned[4];              // Windows磁盘签名
	unsigned char sReserve[2];              // 保留位
	DPT        Dpt;                         // 分区表
	unsigned char EndSign[2];               // 结束标志
}MBR, *PMBR;


//////////////////////////////////////////////////////////////////////////

int
ReadMBRfromHarddisk0(
	IN OUT	PMBR	pMbr
)

/*++

	Routine Description:

		This function read MBR information from disk .

	Arguments:

		pMbr -	Supplies a pointer to MBR

	Return Value:
		
		0	-	success
		-1	-	failure

--*/
{
	int status = 0;
	DWORD dwRead = 0;

	HANDLE hDevice = CreateFile(_T("\\\\.\\PhysicalDrive0"),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile Error %d \r\n", GetLastError());
		status = -1 ;
	}

	ReadFile(hDevice, (LPVOID)pMbr, sizeof(MBR), &dwRead, NULL);

	CloseHandle(hDevice);

	return status;
}





VOID
ParseMbr(
	IN MBR Mbr
	)

/*++

	Routine Description:

		This function parse MBR information. 

	Arguments:

		Mbr	-	Supplies a struct to current disk object, Harddisk0.

	Return Value:

		VOID

--*/

{
	PMBR pMbr = &Mbr;

	for (int i = 0; i < 512; i++)
	{
		printf("%02X ", ((BYTE *)pMbr)[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\r\n");
		}
	}

	printf("引导记录: \r\n");
	int i = 0;

	for (int i = 0; i < BOOTRECORDSIZE; i++)
	{
		printf("%02X ", Mbr.BootRecord.BootRecord[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\r\n");
		}
	}

	printf("\r\n");

	printf("磁盘签名: \r\n");
	for (i = 0; i < 4; i++)
	{
		printf("%02X ", Mbr.ulSigned[i]);
	}

	printf("\r\n");

	printf("解析分区表: \r\n");
	for (i = 0; i < DPTSIZE; i++)
	{
		printf("%02X ", Mbr.Dpt.Dpt[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\r\n");
		}
	}

	printf("\r\n");

	PDP pDp = (PDP)&(Mbr.Dpt.Dpt);
	for (i = 0; i < DPTNUMBER; i++)
	{
		printf("引导标志: %02X ", pDp[i].BootSign);
		printf("分区类型: %02X", pDp[i].PartitionType);
		printf("\r\n");
		printf("本分区之前扇区数: %d ", pDp[i].SectorsInPartition);
		printf("本分区的总扇区数: %d", pDp[i].SectorsInPartition);
		printf("\r\n");
		printf("该分区的大小: %f \r\n", (double)pDp[i].SectorsInPartition / 1024 * 512 / 1024 / 1024);

		printf("\r\n \r\n");
	}

	printf("结束标志: \r\n");
	for (i = 0; i < 2; i++)
	{
		printf("%02X ", Mbr.EndSign[i]);
	}

	printf("\r\n");
}



//
// 打开物理硬盘设备,通过读取PhysicalDrive0设备文件获取MBR引导分区数据
//	PhysicalDrive0
//

int main(int argc, char* argv[])
{

	MBR Mbr = { 0 };

	if (0 == ReadMBRfromHarddisk0(&Mbr))
	{
		ParseMbr(Mbr);
	}
	else
	{
		printf("Error to read MBR.\n");
	}

	
	getchar();

	return 0;
}
