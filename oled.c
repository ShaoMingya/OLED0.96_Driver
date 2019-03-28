/* blink.c
 * you can build this with something like:
 * gcc -Wall blink.c -o blink -lbcm2835
 * sudo ./blink
*/

#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
#include "oled.h"

uint8_t OLED_GRAM[128][8];

void GBZK_GPIO_Config(void)
{
    bcm2835_gpio_fsel(36, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(37, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(38, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(39, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(35, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(34, BCM2835_GPIO_FSEL_OUTP);
    lcd_sclk(0);
    lcd_sid(0);
    lcd_rs(0);
    lcd_cs1(0);
    Rom_CS(0);
    Rom_OUT(0);
}

void GBZK_ROMOUTSET(void)
{
    bcm2835_gpio_fsel(34, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(34, BCM2835_GPIO_PUD_DOWN);
}

void GBZK_ROMOUTRESET(void)
{
    bcm2835_gpio_fsel(34, BCM2835_GPIO_FSEL_OUTP);
}

void transfer_lcd(int data1,uint8_t rs_mode)
{
    uint8_t i;
    lcd_rs(rs_mode);
    delayMicroseconds(2);
    lcd_cs1(0);
    delayMicroseconds(2);
    for(i=0;i<8;i++)
    {
	lcd_sclk(0);
	delayMicroseconds(2);
	if(data1&0x80)
	{
	    lcd_sid(1);
	}
	else
	{
	    lcd_sid(0);
	}
	delayMicroseconds(2);
	lcd_sclk(1);
	delayMicroseconds(2);
	data1<<=1;
    }
    lcd_rs(1);
    delayMicroseconds(2);
    lcd_cs1(1);
    delayMicroseconds(2);
}

uint8_t initial_lcd()
{
    if(!bcm2835_init())
    {
	printf("bcm2835_init Failed!Maybe 'sudo' OK.\r\n");
	return 1;
    }
    GBZK_GPIO_Config();
    lcd_cs1(0);
    Rom_CS(1);

    transfer_lcd(0xAE,RS_CMD);	//关闭显示

    transfer_lcd(0xD5,RS_CMD);//设置时钟分频因子,震荡频率
    transfer_lcd(0x80,RS_CMD);//--set divide ratio

    transfer_lcd(0xA8,RS_CMD);//--set multiplex ratio(1 to 64)
    transfer_lcd(0x3F,RS_CMD);//默认0X3F(1/64)

    transfer_lcd(0xD3,RS_CMD);//-set display offsettransfer_lcd(0xd3,RS_CMD);//-set display offset
    transfer_lcd(0x00,RS_CMD);//-not offset

    transfer_lcd(0x40,RS_CMD);//--set start line address

    transfer_lcd(0x8D,RS_CMD);//--set DC-DC enable
    transfer_lcd(0x14,RS_CMD);//bit2，开启/关闭

    transfer_lcd(0x20,RS_CMD);	//Set Memory Addressing Mode
    transfer_lcd(0x02,RS_CMD);	//0,Horizontal;1,Vertical;2,Page(RESET);other,Invalid

    transfer_lcd(0xA1,RS_CMD);//TAB9-1,set segment re-map 0 to 127

    //区别C0
    transfer_lcd(0xC8,RS_CMD);	//Set COM Output Scan Direction

    transfer_lcd(0xDA,RS_CMD);//--set com pins hardware configuration
    transfer_lcd(0x12,RS_CMD);//默认值

    transfer_lcd(0x81,RS_CMD);//对比度设置
    transfer_lcd(0x7F,RS_CMD);//1~255;默认0X7F (越大越亮)

    transfer_lcd(0xD9,RS_CMD);//--set pre-charge period
    transfer_lcd(0xF1,RS_CMD); //默认值0x22

    transfer_lcd(0xDB,RS_CMD);//--set vcomh
    transfer_lcd(0x20,RS_CMD);//默认值，0x20,0.77xVcc

    transfer_lcd(0xA4,RS_CMD);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    transfer_lcd(0xA6,RS_CMD);//A6,normal display;A7,反显

    transfer_lcd(0xaf,RS_CMD);//--turn on oled panel

    lcd_cs1(1);
    OLED_Clear();
    return 0;
}

void display_128x64(const uint8_t *dp)
{
    uint32_t i,j;
    lcd_cs1(0);
    for(j=0;j<8;j++)
    {
	lcd_address(0,j);
	for (i=0;i<128;i++)
	{
	    transfer_lcd(*dp,RS_DAT);
	    dp++;
	}
    }
    lcd_cs1(1);
}

void lcd_address(uint8_t page,uint8_t column)
{
    transfer_lcd(0xb0 + column,RS_CMD);
    transfer_lcd(((page & 0xf0) >> 4) | 0x10,RS_CMD);
    transfer_lcd((page & 0x0f) | 0x00,RS_CMD);
}

void send_command_to_ROM( uint8_t datu)
{
    uint8_t i;
    for(i=0;i<8;i++ )
    {
	if(datu&0x80)
	{
	    lcd_sid(1);
	}
	else
	{
	    lcd_sid(0);
	}
	delayMicroseconds(2);
	datu = datu<<1;
	lcd_sclk(0);
	delayMicroseconds(2);

	lcd_sclk(1);
	delayMicroseconds(2);
    }
}

static uint8_t get_data_from_ROM()
{
    uint8_t i;
    uint8_t ret_data=0;

    lcd_sclk(1);
    delayMicroseconds(2);

    GBZK_ROMOUTSET();
    delayMicroseconds(2);
    for(i=0;i<8;i++)
    {
	Rom_OUT(1);
	delayMicroseconds(2);
	lcd_sclk(0);
	delayMicroseconds(2);

	ret_data=ret_data<<1;
	ret_data=ret_data+ROM_OUT;

	delayMicroseconds(2);
	lcd_sclk(1);
	delayMicroseconds(2);
    }
    GBZK_ROMOUTRESET();
    delayMicroseconds(2);
    return(ret_data);
}

void get_n_bytes_data_from_ROM(uint32_t fontaddr,uint8_t *pBuff,uint8_t DataLen)
{
    uint8_t i;
    uint8_t addrHigh,addrMid,addrLow;

    addrHigh = (fontaddr&0xff0000)>>16;
    addrMid = (fontaddr&0xff00)>>8;
    addrLow = fontaddr&0xff;

    Rom_CS(0);
    delayMicroseconds(2);
    lcd_cs1(1);
    delayMicroseconds(2);
    lcd_sclk(0);
    delayMicroseconds(2);
    send_command_to_ROM(0x03);
    send_command_to_ROM(addrHigh);
    send_command_to_ROM(addrMid);
    send_command_to_ROM(addrLow);
    for(i=0;i<DataLen;i++)
    {
	*(pBuff+i) = get_data_from_ROM();
    }
    delayMicroseconds(2);
    Rom_CS(1);
}

void display_GB2312_string(uint8_t y,uint8_t x,char *text,uint8_t type)
{
    uint8_t i= 0,n;
    uint32_t fontaddr=0;
    uint8_t fontbuf[32];
    uint32_t BaseAdd;
    if(type==0)
    {
	BaseAdd = 0x3CF80;
    }
    else if(type==1)
    {
	BaseAdd = 0x3B7C0;
    }

    while((text[i]>0x00))
    {
	if(((text[i]>=0xB0)&&(text[i]<=0xF7))&&((text[i+1]>0xA0)&&(text[i+1]<0xFF)))
	{//汉字
	    fontaddr = (text[i]-0xB0)*94;	//一组94个字
	    fontaddr += (text[i+1]-0xA1)+846;
	    fontaddr = (uint32_t)(fontaddr*32);

	    get_n_bytes_data_from_ROM(fontaddr,fontbuf,32);//15*16汉字占32字节
	    for(n=0;n<32;n++)
	    {
		OLED_GRAM[x+(n%16)][y+n/16] = fontbuf[n];
	    }

	    i+=2;
	    x+=16;
	}
	else if(((text[i]>=0xa1) &&(text[i]<=0xa3))&&(text[i+1]>=0xa1))
	{//特殊字符
	    fontaddr = (text[i]-0xA1)*94;
	    fontaddr += (text[i+1]-0xA1);
	    fontaddr = (uint32_t)(fontaddr*32);

	    get_n_bytes_data_from_ROM(fontaddr,fontbuf,32);
	    for(n=0;n<32;n++)
	    {
		OLED_GRAM[x+(n%16)][y+n/16] = fontbuf[n];
	    }

	    i+=2;
	    x+=16;
	}
	else if((text[i]>=0x20) &&(text[i]<=0x7e))
	{
	    uint8_t fontbuf[16];
	    fontaddr = (text[i]-0x20);
	    fontaddr = (uint32_t)(fontaddr*16);
	    fontaddr = (uint32_t)(fontaddr+BaseAdd);

	    get_n_bytes_data_from_ROM(fontaddr,fontbuf,16);
	    for(n=0;n<16;n++)
	    {
		OLED_GRAM[x+(n%8)][y+n/8] = fontbuf[n];
	    }

	    i+=1;
	    x+=8;
	}
	else
	{
	    i++;
	}
    }
}

void display_string_5x7(uint8_t y,uint8_t x,char *text)
{
    uint8_t i= 0,n;
    uint32_t fontaddr=0;
    while((text[i]>0x00))
    {
	if((text[i]>=0x20) &&(text[i]<=0x7e))
	{
	    uint8_t fontbuf[8];
	    fontaddr = (text[i]- 0x20);
	    fontaddr = (uint32_t)(fontaddr*8);
	    fontaddr = (uint32_t)(fontaddr+0x3bfc0);

	    get_n_bytes_data_from_ROM(fontaddr,fontbuf,8);

	    for(n=0;n<8;n++)
	    {
		OLED_GRAM[x+(n%8)][y+n/8] = fontbuf[n];
	    }

	    i+=1;
	    x+=6;
	}
	else
	{
	    i++;
	}
    }
}

uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)result*=m;
	return result;
}

void OLED_Refresh_Gram(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
	transfer_lcd(0xb0+i,RS_CMD);
	transfer_lcd(0x00,RS_CMD);
	transfer_lcd(0x10,RS_CMD);
	for(n=0;n<128;n++)
	{
	    transfer_lcd(OLED_GRAM[n][i],RS_DAT);
	}
    }
}

//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
	for(n=0;n<128;n++)
	{
	    OLED_GRAM[n][i] = 0x00;
	}
    }
    OLED_Refresh_Gram();//更新显示
}
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)//左上角(0,0)
{
    uint8_t pos,bx,temp=0;
    if(x>129||y>64)	return;
    pos = y/8;	//页地址
    bx = y%8;
    temp = 1<<(bx);
    if(t)	OLED_GRAM[x][pos] |= temp;
    else	OLED_GRAM[x][pos] &= ~temp;
}

//x1,y1,x2,y2 填充区域的对角坐标
//确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63
//dot:0,清空;1,填充
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot)
{
    uint8_t x,y;
    for(x=x1;x<=x2;x++)
    {
	for(y=y1;y<=y2;y++)
	{
	    OLED_DrawPoint(x,y,dot);
	}
    }
    OLED_Refresh_Gram();//更新显示
}

void OLED_Rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot)
{
    uint8_t x,y;
    for(x=x1;x<=x2;x++)
    {
	OLED_DrawPoint(x,y1,dot);
	OLED_DrawPoint(x,y2,dot);
    }
    for(y=y1;y<=y2;y++)
    {
	OLED_DrawPoint(x1,y,dot);
	OLED_DrawPoint(x2,y,dot);
    }
    OLED_Refresh_Gram();//更新显示
}
