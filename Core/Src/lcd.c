/*
 * lcd.c
 *
 *  Created on: Sep 13, 2021
 *      Author: Bheem
 */

#include <lcd.h>

/*--------------- Initialize LCD ------------------*/
void lcd_init(void)
{

	HAL_Delay(30);

	PIN_LOW(D4_PORT, D4_PIN);
	PIN_HIGH(D5_PORT, D5_PIN);
	PIN_LOW(D6_PORT, D6_PIN);
	PIN_LOW(D7_PORT, D7_PIN);
	PIN_LOW(RS_PORT, RS_PIN);

	PIN_HIGH(EN_PORT, EN_PIN);
	PIN_LOW(EN_PORT, EN_PIN);

	HAL_Delay(50);

	//	lcd_write (0,0x30);
	//	HAL_Delay(5);  // wait for >4.1ms
	//	lcd_write (0, 0x30);
	//	HAL_Delay(1);  // wait for >100us
	//	lcd_write (0, 0x30);
	//	HAL_Delay(10);
	//	lcd_write (0, 0x20);  // 4bit mode
	//	HAL_Delay(10);
	//	lcd_write (0, 0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	//	HAL_Delay(1);
	//
	//	lcd_write (0,cube ); //Display on/off control --> D=0,C=0, B=0  ---> display off
	//	HAL_Delay(1);
	//	lcd_write (0,0x01);  // clear display
	//	HAL_Delay(1);
	//	lcd_write (0,0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	//	HAL_Delay(1);
	lcd_write(0, 0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_write(0, 0x0c);
	HAL_Delay(1);
	lcd_write(0, 0x06); // Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_write(0, 0x01); // clear display
	HAL_Delay(1);
	lcd_write(0, 0x80); // clear display
	HAL_Delay(1);
}

/*--------------- Write To LCD ---------------*/
void lcd_write(uint8_t type, uint8_t data)
{
	HAL_Delay(2);
	if (type)
	{
		PIN_HIGH(RS_PORT, RS_PIN);
	}
	else
	{
		PIN_LOW(RS_PORT, RS_PIN);
	}

	// Send High Nibble
	if (data & 0x80)
	{
		PIN_HIGH(D7_PORT, D7_PIN);
	}
	else
	{
		PIN_LOW(D7_PORT, D7_PIN);
	}

	if (data & 0x40)
	{
		PIN_HIGH(D6_PORT, D6_PIN);
	}
	else
	{
		PIN_LOW(D6_PORT, D6_PIN);
	}

	if (data & 0x20)
	{
		PIN_HIGH(D5_PORT, D5_PIN);
	}
	else
	{
		PIN_LOW(D5_PORT, D5_PIN);
	}

	if (data & 0x10)
	{
		PIN_HIGH(D4_PORT, D4_PIN);
	}
	else
	{
		PIN_LOW(D4_PORT, D4_PIN);
	}
	PIN_HIGH(EN_PORT, EN_PIN);
	PIN_LOW(EN_PORT, EN_PIN);

	// Send Low Nibble
	if (data & 0x08)
	{
		PIN_HIGH(D7_PORT, D7_PIN);
	}
	else
	{
		PIN_LOW(D7_PORT, D7_PIN);
	}

	if (data & 0x04)
	{
		PIN_HIGH(D6_PORT, D6_PIN);
	}
	else
	{
		PIN_LOW(D6_PORT, D6_PIN);
	}

	if (data & 0x02)
	{
		PIN_HIGH(D5_PORT, D5_PIN);
	}
	else
	{
		PIN_LOW(D5_PORT, D5_PIN);
	}

	if (data & 0x01)
	{
		PIN_HIGH(D4_PORT, D4_PIN);
	}
	else
	{
		PIN_LOW(D4_PORT, D4_PIN);
	}
	PIN_HIGH(EN_PORT, EN_PIN);
	PIN_LOW(EN_PORT, EN_PIN);
}

void lcd_puts(uint8_t x, uint8_t y, int8_t *string)
{
// Set Cursor Position
#ifdef LCD16xN // For LCD16x2 or LCD16x4
	switch (x)
	{
	case 0: // Row 0
		lcd_write(0, 0x80 + 0x00 + y);
		break;
	case 1: // Row 1
		lcd_write(0, 0x80 + 0x40 + y);
		break;
	case 2: // Row 2
		lcd_write(0, 0x80 + 0x10 + y);
		break;
	case 3: // Row 3
		lcd_write(0, 0x80 + 0x50 + y);
		break;
	}
#endif

#ifdef LCD20xN // For LCD20x4
	switch (x)
	{

	case 0: // Row 0
		lcd_write(0, 0x80 + 0x00 + y);
		break;
	case 1: // Row 1
		lcd_write(0, 0x80 + 0x40 + y);
		break;
	case 2: // Row 2
		lcd_write(0, 0x80 + 0x14 + y);
		break;
	case 3: // Row 3
		lcd_write(0, 0x80 + 0x54 + y);
		break;
	}
#endif

	while (*string)
	{
		lcd_write(1, *string);
		string++;
	}
}

void clr_data(uint8_t pos)
{

	switch (pos)
	{
	case _intensity:
		lcd_puts(1, 7, (int8_t *)"  ");
		lcd_puts(1, 8, (int8_t *)"  ");
		lcd_puts(1, 9, (int8_t *)"  ");
		break;
	case _color:
		lcd_puts(2, 7, (int8_t *)"  ");
		break;
	case _sensor:
		lcd_puts(3, 7, (int8_t *)"  ");
		lcd_puts(3, 8, (int8_t *)"  ");
		lcd_puts(3, 9, (int8_t *)"  ");
		break;

	case _lamp:
		lcd_puts(1, 17, (int8_t *)"  ");
		lcd_puts(1, 18, (int8_t *)"  ");
		lcd_puts(1, 19, (int8_t *)"  ");
		break;
	case _endo:
		lcd_puts(2, 17, (int8_t *)"  ");
		lcd_puts(2, 18, (int8_t *)"  ");
		lcd_puts(2, 19, (int8_t *)"  ");
		break;
	case _depth:
		lcd_puts(3, 17, (int8_t *)" ");
		lcd_puts(3, 18, (int8_t *)" ");
		lcd_puts(3, 19, (int8_t *)" ");
		break;

	case _focus:
		lcd_puts(0, 10, (int8_t *)" ");
		lcd_puts(0, 11, (int8_t *)" ");
		lcd_puts(0, 12, (int8_t *)" ");
		lcd_puts(0, 13, (int8_t *)" ");
		lcd_puts(0, 14, (int8_t *)" ");
		lcd_puts(0, 15, (int8_t *)" ");
		break;


	case _small:
		lcd_puts(1, 10, (int8_t *)" ");
		lcd_puts(1, 10, (int8_t *)" ");
		lcd_puts(1, 10, (int8_t *)" ");
		break;



	case _medium:
		lcd_puts(2, 10, (int8_t *)" ");
		lcd_puts(2, 10, (int8_t *)" ");
		lcd_puts(2, 10, (int8_t *)" ");
		break;


	case _wide:
		lcd_puts(3, 10, (int8_t *)" ");
		lcd_puts(3, 10, (int8_t *)" ");
		lcd_puts(3, 10, (int8_t *)" ");
		break;
	}
}
void clr_select()
{
	lcd_puts(1, 0, (int8_t *)" ");
	lcd_puts(2, 0, (int8_t *)" ");
	lcd_puts(3, 0, (int8_t *)" ");
	lcd_puts(1, 10, (int8_t *)" ");
	lcd_puts(2, 10, (int8_t *)" ");
	lcd_puts(3, 10, (int8_t *)" ");
}
void lcd_clear(void)
{
	lcd_write(0, 0x01);
}

void default_lcd_print(void)
{
	char buffer[3];
	lcd_puts(0, 0, (int8_t *)"______ COGNATE _____");

	// INTENSITY PRINT
	lcd_puts(1, 1, (int8_t *)"INTEN");
	sprintf(buffer, "%02d", lcd.intensity);
	lcd_puts(1, 7, (int8_t *)buffer);

	// COLOR PRINT
	lcd_puts(2, 1, (int8_t *)"COLOR");
	sprintf(buffer, "%02d", lcd.color);
	lcd_puts(2, 7, (int8_t *)buffer);

	// SENSOR PRINT
	lcd_puts(3, 1, (int8_t *)"SENSR");
	if (lcd.sensor)
		lcd_puts(3, 7, (int8_t *)"ON");
	else
	{
		clr_data(_sensor);
		lcd_puts(3, 7, (int8_t *)"OFF");
	}

	// LAMP PRINT
	lcd_puts(1, 11, (int8_t *)"LAMP");
	if (lcd.lamp)
		lcd_puts(1, 17, (int8_t *)"ON");
	else
	{
		clr_data(_lamp);
		lcd_puts(1, 17, (int8_t *)"OFF");
	}

	// ENDO PRINT
	lcd_puts(2, 11, (int8_t *)"ENDO");
	if (lcd.endo)
		lcd_puts(2, 17, (int8_t *)"ON");
	else
	{
		clr_data(_endo);
		lcd_puts(2, 17, (int8_t *)"OFF");
	}

	// DEPTH PRINT
	lcd_puts(3, 11, (int8_t *)"DEPTH");
	if (lcd.depth)
		lcd_puts(3, 17, (int8_t *)"ON");
	else
	{
		clr_data(_depth);
		lcd_puts(3, 17, (int8_t *)"OFF");
	}
	// set cursor
	lcd_puts(1, 0, (int8_t *)">");
}

void lcd_update()
{
	char buffer[3];
	sprintf(buffer, "%02d", lcd.intensity);
	lcd_puts(1, 7, (int8_t *)buffer);

	sprintf(buffer, "%02d", lcd.color);
	lcd_puts(2, 7, (int8_t *)buffer);

	if (lcd.sensor)
		lcd_puts(3, 7, (int8_t *)"ON");
	else
	{
		clr_data(_sensor);
		lcd_puts(3, 7, (int8_t *)"OFF");
	}
	if (lcd.lamp)
		lcd_puts(1, 17, (int8_t *)"ON");
	else
	{
		clr_data(_lamp);
		lcd_puts(1, 17, (int8_t *)"OFF");
	}

	if (lcd.endo)
		lcd_puts(2, 17, (int8_t *)"ON");
	else
	{
		clr_data(_endo);
		lcd_puts(2, 17, (int8_t *)"OFF");
	}

	if (lcd.depth)
		lcd_puts(3, 17, (int8_t *)"ON");
	else
	{
		clr_data(_depth);
		lcd_puts(3, 17, (int8_t *)"OFF");
	}
}

void time_calculate(uint32_t tt)
{
	uint32_t current = tt;

	run_time->hour = current / (60 * 60);
	current = current % 3600;
	run_time->minutes = current / 60;
	current = current % 60;
	run_time->seconds = current;
}

void clock_page(void)
{
	lcd_clear();
	lcd_puts(0, 1, (int8_t *)"OTL Total Run Time");
	lcd_puts(1, 3, (int8_t *)"Hours");
	lcd_puts(2, 3, (int8_t *)"Minutes");
	lcd_puts(3, 3, (int8_t *)"Seconds");
}

void Total_Time_Print(uint32_t total)
{
	char buffer[3];
	uint32_t current = total;
	uint8_t h = current / (60 * 60);
	current = current % 3600;
	uint8_t m = current / 60;
	current = current % 60;
	uint8_t s = current;

	// uint8_t h = current / (60*60);
	//		run_time->hour = current / (60*60);
	//		current = current % 3600;
	//		run_time->minutes = current / 60;
	//		current = current %60;
	//		run_time->seconds = current;

	// clock_page();

	sprintf(buffer, "%02d", h); // run_time->hour
	lcd_puts(1, 0, (int8_t *)buffer);
	sprintf(buffer, "%02d", m);
	lcd_puts(2, 0, (int8_t *)buffer);
	sprintf(buffer, "%02d", s);
	lcd_puts(3, 0, (int8_t *)buffer);
}

void home_page()
{
	lcd_clear();
	lcd_puts(0, 3, (int8_t *)"COGNATE INDIA");
	// lcd_puts(1, 3,(int8_t*) "Hours");
	lcd_puts(2, 3, (int8_t *)"VISION SERIES");
	lcd_puts(3, 5, (int8_t *)"LED LIGHT");
}

// void print_data(uint8_t y, uint8_t x, )
//{
//
// }

// void lcd_print_pg1()
//{
//
// }
//
// void lcd_print_pg2(void)
//{
//
// }
