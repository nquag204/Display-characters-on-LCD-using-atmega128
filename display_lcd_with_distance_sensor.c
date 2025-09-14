#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

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

// Định nghĩa các chân cho cảm biến siêu âm HC-SR04
#define TRIG_PORT PORTA
#define TRIG_DDR  DDRA
#define TRIG_PIN  PA0

#define ECHO_PORT PORTA
#define ECHO_DDR  DDRA
#define ECHO_PIN  PA1
#define ECHO_PIN_INPUT PINA

// Biến toàn cục
volatile unsigned int timer_overflow = 0;
volatile unsigned int echo_start_time = 0;
volatile unsigned int echo_end_time = 0;
volatile unsigned char echo_received = 0;

// Các hàm cho LCD
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_String(const char* str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);

// Các hàm cho cảm biến siêu âm
void Ultrasonic_Init(void);
unsigned int Measure_Distance(void);
void Timer1_Init(void);

// Hàm tiện ích
void Int_To_String(unsigned int value, char* buffer);

// Hàm tạo xung Enable cho LCD
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
	// Gửi 4 bit cao trước
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
	LCD_RS_PORT |= (1 << LCD_RS_PIN);   // RS = 1 (data mode)
	LCD_RW_PORT &= ~(1 << LCD_RW_PIN);  // RW = 0 (write mode)
	LCD_Enable();
	
	// Gửi 4 bit thấp
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((data << 4) & 0xF0);
	LCD_Enable();
	
	_delay_ms(2);
}

// Khởi tạo LCD ở chế độ 4-bit
void LCD_Init(void)
{
	// Cấu hình các chân làm đầu ra
	LCD_RS_DDR |= (1 << LCD_RS_PIN);
	LCD_RW_DDR |= (1 << LCD_RW_PIN);
	LCD_EN_DDR |= (1 << LCD_EN_PIN);
	LCD_DATA_DDR |= (1 << LCD_D4_PIN) | (1 << LCD_D5_PIN) | (1 << LCD_D6_PIN) | (1 << LCD_D7_PIN);
	
	// Khởi tạo trạng thái ban đầu
	LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
	LCD_RW_PORT &= ~(1 << LCD_RW_PIN);
	LCD_EN_PORT &= ~(1 << LCD_EN_PIN);
	
	_delay_ms(20);  // Chờ LCD khởi động
	
	// Khởi tạo LCD ở chế độ 4-bit
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | 0x30;  // Function set: 8-bit
	LCD_Enable();
	_delay_ms(5);
	
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | 0x30;  // Function set: 8-bit
	LCD_Enable();
	_delay_us(100);
	
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | 0x30;  // Function set: 8-bit
	LCD_Enable();
	_delay_ms(1);
	
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | 0x20;  // Function set: 4-bit
	LCD_Enable();
	_delay_ms(1);
	
	// Cấu hình LCD: 4-bit, 2 dòng, 5x7 dots
	LCD_Command(0x28);  // Function set: 4-bit, 2 lines, 5x7
	LCD_Command(0x0C);  // Display on, cursor off, blink off
	LCD_Command(0x06);  // Entry mode: increment cursor
	LCD_Command(0x01);  // Clear display
	_delay_ms(2);
}

// Xóa màn hình LCD
void LCD_Clear(void)
{
	LCD_Command(0x01);  // Clear display
	_delay_ms(2);
}

// Đặt con trỏ tại vị trí (row, col)
void LCD_SetCursor(unsigned char row, unsigned char col)
{
	unsigned char address;
	
	switch(row)
	{
		case 0: address = 0x80 + col; break;  // Dòng 1
		case 1: address = 0xC0 + col; break;  // Dòng 2
		case 2: address = 0x94 + col; break;  // Dòng 3 (cho LCD 20x4)
		case 3: address = 0xD4 + col; break;  // Dòng 4 (cho LCD 20x4)
		default: address = 0x80; break;
	}
	
	LCD_Command(address);
}

// Hiển thị chuỗi ký tự
void LCD_String(const char* str)
{
	while(*str)
	{
		LCD_Data(*str++);
	}
}

// Khởi tạo Timer1 để đo thời gian
void Timer1_Init(void)
{
	// Cấu hình Timer1: Normal mode, prescaler = 8
	// Với f_cpu = 16MHz, Timer1 sẽ có độ phân giải 0.5μs
	TCCR1A = 0x00;
	TCCR1B = 0x02;  // Prescaler = 8
	TIMSK |= (1 << TOIE1);  // Enable Timer1 overflow interrupt
	TCNT1 = 0;
	timer_overflow = 0;
}

// Ngắt Timer1 overflow
ISR(TIMER1_OVF_vect)
{
	timer_overflow++;
}

// Khởi tạo cảm biến siêu âm
void Ultrasonic_Init(void)
{
	// Cấu hình chân TRIG làm đầu ra
	TRIG_DDR |= (1 << TRIG_PIN);
	TRIG_PORT &= ~(1 << TRIG_PIN);  // Đặt TRIG = 0
	
	// Cấu hình chân ECHO làm đầu vào
	ECHO_DDR &= ~(1 << ECHO_PIN);
	ECHO_PORT &= ~(1 << ECHO_PIN);  // Tắt pull-up
	
	// Khởi tạo Timer1
	Timer1_Init();
	sei();  // Enable global interrupts
}

// Hàm đo khoảng cách
unsigned int Measure_Distance(void)
{
	unsigned long pulse_width = 0;
	unsigned int distance = 0;
	
	// Reset các biến
	echo_received = 0;
	timer_overflow = 0;
	
	// Tạo xung TRIG (10μs)
	TRIG_PORT |= (1 << TRIG_PIN);
	_delay_us(10);
	TRIG_PORT &= ~(1 << TRIG_PIN);
	
	// Reset Timer1
	TCNT1 = 0;
	
	// Chờ ECHO lên HIGH
	while(!(ECHO_PIN_INPUT & (1 << ECHO_PIN)));
	echo_start_time = TCNT1;
	
	// Chờ ECHO xuống LOW
	while(ECHO_PIN_INPUT & (1 << ECHO_PIN));
	echo_end_time = TCNT1;
	
	// Tính toán độ rộng xung (đơn vị: timer ticks)
	if(echo_end_time >= echo_start_time)
	{
		pulse_width = echo_end_time - echo_start_time;
	}
	else
	{
		// Trường hợp Timer overflow
		pulse_width = (0xFFFF - echo_start_time) + echo_end_time + 1;
	}
	
	// Chuyển đổi sang khoảng cách (cm)
	// pulse_width * 0.5μs * 340m/s / 2 / 100 = pulse_width * 0.0085
	// Với prescaler = 8, mỗi tick = 0.5μs
	// distance (cm) = pulse_width * 0.5 * 34 / 200 = pulse_width * 0.085
	// Để tránh số thực, ta nhân 10 rồi chia 100: distance = pulse_width * 85 / 1000
	distance = (unsigned int)(pulse_width * 17 / 1000);  // Đã tối ưu công thức
	
	return distance;
}

// Chuyển đổi số nguyên thành chuỗi
void Int_To_String(unsigned int value, char* buffer)
{
	sprintf(buffer, "%u", value);
}

// Hàm main
int main(void)
{
	unsigned int distance = 0;
	char distance_str[10];
	char display_buffer[21];
	
	// Khởi tạo các thiết bị
	LCD_Init();
	Ultrasonic_Init();
	
	// Hiển thị tiêu đề
	LCD_Clear();
	LCD_SetCursor(0, 0);
	LCD_String("DISTANCE METER");
	LCD_SetCursor(1, 0);
	LCD_String("HC-SR04 + ATMEGA128");
	
	_delay_ms(2000);  // Hiển thị tiêu đề trong 2 giây
	
	// Vòng lặp chính
	while(1)
	{
		// Đo khoảng cách
		distance = Measure_Distance();
		
		// Chuyển đổi kết quả thành chuỗi
		Int_To_String(distance, distance_str);
		
		// Tạo chuỗi hiển thị
		strcpy(display_buffer, "Distance: ");
		strcat(display_buffer, distance_str);
		strcat(display_buffer, " cm");
		
		// Hiển thị kết quả lên LCD
		LCD_Clear();
		LCD_SetCursor(0, 0);
		LCD_String("ULTRASONIC SENSOR");
		LCD_SetCursor(1, 0);
		LCD_String(display_buffer);
		
		// Hiển thị trạng thái
		LCD_SetCursor(2, 0);
		if(distance > 0 && distance < 400)  // Phạm vi hoạt động của HC-SR04
		{
			LCD_String("Status: OK");
		}
		else
		{
			LCD_String("Status: Out Range");
		}
		
		LCD_SetCursor(3, 0);
		LCD_String("NGUYEN NHAT QUANG");
		
		_delay_ms(500);  // Cập nhật mỗi 500ms
	}
	
	return 0;
}
