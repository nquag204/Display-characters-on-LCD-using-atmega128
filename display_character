#include <avr/io.h>
#include <util/delay.h>

// Định nghĩa các chân kết nối LCD
#define LCD_RS_PORT PORTB
#define LCD_RS_DDR  DDRB
#define LCD_RS_PIN  PB0

#define LCD_RW_PORT PORTB
#define LCD_RW_DDR  DDRB
#define LCD_RW_PIN  PB1

#define LCD_EN_PORT PORTB
#define LCD_EN_DDR  DDRB
#define LCD_EN_PIN  PB2

#define LCD_DATA_PORT PORTC
#define LCD_DATA_DDR  DDRC
#define LCD_D4_PIN    PC4
#define LCD_D5_PIN    PC5
#define LCD_D6_PIN    PC6
#define LCD_D7_PIN    PC7

// Các hàm cho LCD
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_String(const char* str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);

// Hàm tạo xung Enable
void LCD_Enable(void)
{
	LCD_EN_PORT |= (1 << LCD_EN_PIN);   // EN = 1
	_delay_us(1);
	LCD_EN_PORT &= ~(1 << LCD_EN_PIN);  // EN = 0
	_delay_us(100);
}

// Gửi lệnh đến LCD (4-bit mode)
void LCD_Command(unsigned char cmd)
{
	//Gửi 4 bit cao trước
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (cmd & 0xF0);
	LCD_RS_PORT &= ~(1 << LCD_RS_PIN);  // RS = 0 (command mode)
	LCD_RW_PORT &= ~(1 << LCD_RW_PIN);  // RW = 0 (write mode)
	LCD_Enable();
	
	// Gửi 4 bit thấp
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((cmd << 4) & 0xF0);
	LCD_Enable();
	
	_delay_ms(2);
}

// Gửi dữ liệu đến LCD (4-bit mode)
void LCD_Data(unsigned char data)
{
	// Gửi 4 bit cao trước
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (data & 0xF0);
	LCD_RS_PORT |= (1 << Lgì
	}
	
	return 0;
}
