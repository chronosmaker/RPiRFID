#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "MFRC522.H"	

//M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反 
unsigned char data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 	//默认密钥
unsigned char g_ucTempbuf[20];		//接口缓存

int main(void)
{
	wiringPiSetup();					    //wiringPi初始化
	wiringPiSPISetup(channel, speed);		//设置SPI通道 0，速率 1000000
	printf ("Raspberry Pi RFID\n") ;

	unsigned char status, i, j;

	delay(500);
	PcdReset();		//RC522复位
	PcdAntennaOff();	//关闭天线
	delay(10);		//每次启动或关闭天线之间应至少有1ms的间隔
	PcdAntennaOn();  	//开启天线 

	while(1)
	{	
		status = PcdRequest(PICC_REQALL, g_ucTempbuf);		//寻卡
		if (status != MI_OK)
		{    
			continue;
		}
		printf("\ncard type:");
		switch(g_ucTempbuf[0])
		{
			case 0x02:    printf("Mifare_One(S70)\n"); break;
			case 0x04:    printf("Mifare_One(S50)\n"); break;
			case 0x08:    printf("Mifare_Pro(X)\n"); break;
			case 0x44:    if(g_ucTempbuf[1] == 0x00)       { printf("Mifare_UltraLight\n"); break; }
				      else if(g_ucTempbuf[1] == 0x03)  { printf("Mifare_DESFire\n");    break; }
		}
		status = PcdAnticoll(g_ucTempbuf);			//防冲撞
		if (status != MI_OK)
		{    
			continue;
		}
		printf("\ncard SN:");	
		for(i=0;i<4;i++)
		{
			if(g_ucTempbuf[i] < 0x10)    printf("0%X",g_ucTempbuf[i]);
			else    printf("%X",g_ucTempbuf[i]);		
		}
		printf("\n");
		status = PcdSelect(g_ucTempbuf);			//选定卡片
		if (status != MI_OK)
		{
			continue;
		}
		status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);	//验证卡片密码
		if (status != MI_OK)
		{
			continue;
		}
		status = PcdWrite(1, data1);						//写块1
		if (status != MI_OK)
		{
			continue;
		}
		status = PcdBakValue(1, 2);						//块备份1 --> 2
		if (status != MI_OK)
		{
			continue;
		}
		for(i=0;i<4;i++)
		{
			status = PcdRead(i, g_ucTempbuf);				//读块

			if (status != MI_OK)
			{
				continue;
			}
			printf("\nread block %d:", i);
			int j;
			for(j=0;j<16;j++)
			{
				if(g_ucTempbuf[j] < 0x10)    printf("0%X",g_ucTempbuf[j]);
				else    printf("%X",g_ucTempbuf[j]);	
			}
		}
		printf("\n");
		PcdHalt();			//卡片进入休眠状态
		delay(1000);
	}
}
