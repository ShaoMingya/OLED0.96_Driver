
#define	_OLED_H_
#ifdef	_OLED_H_

#include <bcm2835.h>

#define RS_CMD	0
#define RS_DAT	1

#define lcd_sclk(a)	bcm2835_gpio_write(39,a)
#define lcd_sid(a)	bcm2835_gpio_write(38,a)
#define lcd_rs(a)	bcm2835_gpio_write(37,a)
#define lcd_cs1(a)	bcm2835_gpio_write(36,a)
#define Rom_OUT(a)	bcm2835_gpio_write(35,a)

#define ROM_OUT	bcm2835_gpio_lev(35)

#define Rom_CS(a)	bcm2835_gpio_write(34, a)

void GBZK_GPIO_Config(void);

void transfer_lcd(int data1,uint8_t rs_mode);
uint8_t initial_lcd(void);
void lcd_address(uint8_t page,uint8_t column);

void display_128x64(const uint8_t *dp);

void send_command_to_ROM( uint8_t datu);
static uint8_t get_data_from_ROM(void);
void get_n_bytes_data_from_ROM(uint32_t fontaddr,uint8_t *pBuff,uint8_t DataLen);

void display_GB2312_string(uint8_t y,uint8_t x,char *text,uint8_t type);
void display_string_5x7(uint8_t y,uint8_t x,char *text);

void OLED_Refresh_Gram(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_Rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);

void OLED_Clear(void);
#endif
