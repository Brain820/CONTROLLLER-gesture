/*
 * lcd.h
 *
 *  Created on: Sep 13, 2021
 *      Author: Bheem
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_


#include "stm32f0xx_hal.h"
#include <stdio.h>

/*------------- Define LCD Use -----------------*/

/*Note: Comment which not use */

//#define LCD16xN //For lcd16x2 or lcd16x4
#define LCD20xN //For lcd20x4

/*------------- Define For Connection -----------------*/

#define RS_PORT		GPIOA
#define RS_PIN		GPIO_PIN_5

#define EN_PORT		GPIOA
#define EN_PIN		GPIO_PIN_4

#define D7_PORT		GPIOA
#define D7_PIN		GPIO_PIN_3

#define D6_PORT		GPIOA
#define D6_PIN		GPIO_PIN_2

#define D5_PORT		GPIOA
#define D5_PIN		GPIO_PIN_1

#define D4_PORT		GPIOA
#define D4_PIN		GPIO_PIN_0

enum cursor
{
_intensity = 1,
_color,
_sensor,
_lamp,
_endo,
_depth,
_focus,
_small,
_medium,
_wide

};

extern  struct lcd_variable
{
	volatile uint8_t position;
	volatile int8_t intensity;
	volatile int8_t color;
	volatile uint8_t sensor;
	volatile uint8_t lamp;
	volatile uint8_t endo;
	volatile uint8_t depth;
};
extern struct lcd_variable lcd;


//struct flag_variable
//{
//	volatile uint8_t _s_prv;
//	volatile uint8_t _s_next;
//	volatile int8_t _color;
//	volatile uint8_t _depth;
//	volatile uint8_t _set_n;
//	volatile uint8_t _set_p;
//};
//
//struct flag_variable flag_status;


//extern struct lcd_variable lcd;

#define MAX 10
#define OFF 0
#define ON 1

typedef struct time_var
{
uint8_t hour;
uint8_t minutes;
uint8_t seconds;

}times;

times* run_time;

#define PIN_LOW(PORT,PIN)	HAL_GPIO_WritePin(PORT,PIN,GPIO_PIN_RESET);
#define PIN_HIGH(PORT,PIN)	HAL_GPIO_WritePin(PORT,PIN,GPIO_PIN_SET);

/*------------ Declaring Function Prototype -------------*/
void default_lcd_print(void);
void lcd_init(void);
void lcd_write(uint8_t type,uint8_t data);
void lcd_puts(uint8_t x, uint8_t y, int8_t *string);
void lcd_clear(void);
void clr_select(void);
void clr_data(uint8_t);

void Total_Time_Print(uint32_t);
void time_calculate(uint32_t);
void clock_page(void);

void lcd_update(void);
//void lcd_print_pg1(void);
//void lcd_print_pg2(void);
void set_cursor(void);
#endif /* INC_LCD_H_ */
