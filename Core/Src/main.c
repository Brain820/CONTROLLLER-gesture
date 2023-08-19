/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint32_t timer_intrupt = 0;
uint8_t depth_press = 0;
uint8_t focus_upd = 0;
#define on 1
#define off 0

void page_3_print(void);
void page_2_print(void);
void send_cmd(int8_t x, int8_t mode);

uint8_t last_pg, current_pg;
uint8_t pg2 = 0;
uint8_t pg2_fc = 0;

uint8_t pg3_sm, pg3_md, pg3_wd = 0;

uint32_t tt_cnt = 0;
uint8_t last_ps;

enum pos
{
	intensity = 1,
	color,
	sensor,
	lamp,
	endo,
	depth
};

enum key
{
	_prv,
	_nxt,
	_depth,
	_neg,
	_pos
};

struct key_status_reg
{
	volatile uint8_t prv : 1;
	volatile uint8_t nxt : 1;
	volatile uint8_t depth : 1;
	volatile uint8_t pos : 1;
	volatile uint8_t neg : 1;
};

struct gesture_status_reg
{
	uint8_t up : 1;
	uint8_t down : 1;
	uint8_t left : 1;
	uint8_t right : 1;
};

struct interrupt_flag_reg
{
	volatile uint8_t key_flag : 1;
	volatile uint8_t gesture_flag : 1;
	volatile uint8_t screen_flag : 1;
	volatile uint8_t prv_long_press : 1;
	volatile uint8_t depth_long_press : 1;
	volatile uint8_t update_data : 1;
};

struct pos_status_reg
{
	int8_t position_cursor;
	uint16_t key_number : 3;
	uint16_t gesture_direction : 2;
	uint16_t screen_number : 3;
};

struct screen_data_reg
{
	uint8_t intensity : 4;
	uint8_t sensor : 1;
	uint8_t lamp : 1;
	uint8_t endo : 1;
	uint8_t depth : 1;
	int8_t color;
};

enum pos cursor;
struct key_status_reg key_pressed;
struct gesture_status_reg ges_dir;
struct interrupt_flag_reg interrupt_reg;
struct pos_status_reg current_pos = {1, 0, 0, 1};
struct screen_data_reg data_reg = {1, 0, 1, 0, 0, 0};

int8_t last_inten;
int8_t last_focus;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void init_gesture();
uint8_t readGesture();
uint8_t gestureAvailable();
void clr_select();
void clr_data(uint8_t);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void home_page(void);
void lcd_clear(void);
void lcd_init(void);
void lcd_puts(uint8_t x, uint8_t y, int8_t *string);

uint8_t but_state = 0;
// Callback: timer has rolled over
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
// 	// Check which version of the timer triggered this callback and toggle LED
// 	if (htim == &htim6)
// 	{
// 		//    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
// 		tt_cnt++;
// 	}
// }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim == &htim6)
	{
		//    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		tt_cnt++;
	}
	/* Prevent unused argument(s) compilation warning */
	UNUSED(htim);

	/* NOTE : This function should not be modified, when the callback is needed,
			  the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
	 */

	if (htim == &htim14)
	{
		if (HAL_GPIO_ReadPin(S_PRV_GPIO_Port, S_PRV_Pin) == GPIO_PIN_RESET)
		{

			but_state = true;
			HAL_TIM_Base_Stop_IT(&htim14);
		}
	}
}

void print_int(int _data)
{
	int8_t buffer[8];
	lcd_clear();
	sprintf(buffer, "%02d", _data);
	lcd_puts(3, 2, buffer); // x = 2, y = 3
}

uint8_t update_key_press()
{
	if (key_pressed.prv)
	{
		current_pos.key_number = _prv;
		key_pressed.prv = 0;
		return _prv;
	}
	else if (key_pressed.nxt)
	{
		key_pressed.nxt = 0;
		current_pos.key_number = _nxt;

		return _nxt;
	}
	else if (key_pressed.depth)
	{
		// 		key_pressed.depth = 0;
		// 		current_pos.key_number = _depth;
		// 		data_reg.depth = !data_reg.depth;
		// 		clr_data(depth);
		// 		clr_select();
		// //		lcd_puts(3, 10, (int8_t *)">");
		// 		if (data_reg.depth)
		// 			lcd_puts(3, 17, (int8_t *)"ON");
		// 		else
		// 			lcd_puts(3, 17, (int8_t *)"OFF");

		// 		return _depth;

		key_pressed.depth = 0;
		current_pos.key_number = _depth;
		// data_reg.endo = !data_reg.endo;
		clr_data(endo);
		clr_select();
		//		lcd_puts(3, 10, (int8_t *)">");
		if (pg2_fc == 0)
		{
			if (data_reg.endo)
				lcd_puts(2, 17, (int8_t *)"ON");
			else
				lcd_puts(2, 17, (int8_t *)"OFF");
		}

		return _depth;
	}
	else if (key_pressed.pos)
	{
		key_pressed.pos = 0;
		current_pos.key_number = _pos;
		last_ps = _pos;
		return _pos;
	}
	else if (key_pressed.neg)
	{
		key_pressed.neg = 0;
		current_pos.key_number = _neg;
		last_ps = _neg;
		return _neg;
	}

	return 0;
}

// void long_prv_press()
//{
//	current_pos.position_cursor =  sensor;
//	data_reg.sensor = !data_reg.sensor;
//
// }
void set_data_positive()
{
	if (pg2_fc == 0)
	{
		switch (current_pos.position_cursor)
		{
		case intensity:
			if (!data_reg.depth)
			{
				data_reg.intensity++;
				if (data_reg.intensity >= 10)
				{
					data_reg.intensity = 10;
				}
			}
			else
			{
				//				data_reg.intensity = 10;
				depth_press = 1;
			}

			break;
		case color:
			// if (!data_reg.depth)
			// {
			data_reg.color++;
			if (data_reg.color > 1)
			{
				data_reg.color = 1;
			}
			// }

			break;
		case sensor:
			data_reg.sensor = on;
			break;
		case lamp:
			data_reg.lamp = on;
			break;
		case endo:
			data_reg.endo = on;
			break;
		case depth:
			data_reg.depth = on;
			//			last_inten = data_reg.intensity;
			break;
		case 7:
			pg2_fc = on;
			break;
		}
	}

	else
	{
		switch (current_pos.position_cursor)
		{
		case 7:
			pg2_fc = on;
			pg3_sm = off;
			pg3_md = off;
			pg3_wd = off;
			break;
		case 1:
			pg3_sm = on;
			pg3_md = off;
			pg3_wd = off;
			break;
		case 2:
			pg3_sm = off;
			pg3_md = on;
			pg3_wd = off;
			break;
		case 3:
			pg3_sm = off;
			pg3_md = off;
			pg3_wd = on;
			break;
		}
	}
}

void set_data_negative()
{
	if (pg2_fc == 0)
	{

		switch (current_pos.position_cursor)
		{
		case intensity:
			if (!data_reg.depth)
			{
				data_reg.intensity--;
				if (data_reg.intensity <= 1)
				{
					data_reg.intensity = 1;
				}
			}
			else
			{
				depth_press = 1;
			}
			break;
		case color:
			// if (!data_reg.depth)
			// {
			data_reg.color--;
			if (data_reg.color < -1)
			{
				data_reg.color = -1;
			}
			// }
			break;
		case sensor:
			data_reg.sensor = off;
			break;
		case lamp:
			data_reg.lamp = off;
			break;
		case endo:
			data_reg.endo = off;
			data_reg.intensity = data_reg.intensity;
			break;
		case depth:
			data_reg.depth = off;
			break;
		case 7:
			pg2_fc = off;
			break;
		}
	}

	else
	{
		switch (current_pos.position_cursor)
		{
		case 7:
			pg2_fc = off;
			break;
		case 1:
			pg3_sm = off;
			break;
		case 2:
			pg3_md = off;
			break;
		case 3:
			pg3_wd = off;
			break;
		}
	}
}

void update_new_data()
{
	switch (current_pos.key_number)
	{
	case _prv:
		current_pos.position_cursor--;
		if (!data_reg.sensor)
		{

			if (current_pos.position_cursor == 3)
			{
				current_pos.position_cursor = 2;
			}
		}
		// if (pg2_fc == 1)
		// {
		// 	if (current_pos.position_cursor > 3)
		// 	{
		// 		current_pos.position_cursor = 7;
		// 	}
		// }

		if (current_pos.position_cursor <= 0)
		{
			//			lcd_clear();
			current_pos.position_cursor = 7;
		}
		break;
	case _nxt:
		current_pos.position_cursor++;

		if (!data_reg.sensor)
		{

			if (current_pos.position_cursor == 3 && (pg2_fc == 0))
			{
				current_pos.position_cursor = 4;
			}
		}
		//		if (current_pos.position_cursor == 7)
		//		{
		//			page_2_print();
		//		}

		//		if (pg2_fc == 1)
		//		{
		//			if (current_pos.position_cursor > 3)
		//			{
		//				current_pos.position_cursor = 7;
		//			}
		//		}

		if (current_pos.position_cursor > 7)
		{

			current_pos.position_cursor = 1;
		}
		break;
	case _depth:
		//		data_reg.depth = !data_reg.depth;
		// 		if (data_reg.depth)
		// 		{
		// 			current_pos.position_cursor = intensity;
		// 			last_inten = data_reg.intensity;
		// 			data_reg.intensity = 10;
		// 			depth_press = 1;
		// //			data_reg.depth = on;
		// 		}

		data_reg.endo = !data_reg.endo;
		current_pos.position_cursor = endo;
		if (data_reg.endo)
		{

			// last_inten = data_reg.intensity;
			data_reg.endo = 1;
			// depth_press = 1;
			//			data_reg.depth = on;
		}

		else
		{
			// current_pos.position_cursor = endo;
			data_reg.endo = 0;
			// depth_press = 1;
			//			data_reg.depth = off;
		}
		break;
	case _pos:
		set_data_positive();
		break;
	case _neg:
		set_data_negative();
		break;
	}
}

void page_2_print()
{
	lcd_puts(0, 0, (int8_t *)">");
	lcd_puts(0, 1, (int8_t *)"FOCUS  :");
}

void page_3_print()
{
	char buf[3];
	lcd_puts(0, 0, (int8_t *)">");
	lcd_puts(0, 1, (int8_t *)"FOCUS  :");
	lcd_puts(1, 1, (int8_t *)"SMALL  : ");
	lcd_puts(2, 1, (int8_t *)"Medium : ");
	lcd_puts(3, 1, (int8_t *)"Wide   : ");

	if (pg3_sm)
	{
		clr_data(8);
		send_cmd(pg3_sm, 8);
		lcd_puts(1, 10, (int8_t *)"ON");
		lcd_puts(3, 10, (int8_t *)"OFF");
		lcd_puts(2, 10, (int8_t *)"OFF");
	}
	// else
	// {
	// 	lcd_puts(1, 10, (int8_t *)"OFF");
	// }

	else if (pg3_md)
	{
		clr_data(9);
		send_cmd(pg3_md, 9);
		lcd_puts(2, 10, (int8_t *)"ON");
		lcd_puts(3, 10, (int8_t *)"OFF");
		lcd_puts(1, 10, (int8_t *)"OFF");
	}

	// else
	// {
	// 	lcd_puts(2, 10, (int8_t *)"OFF");
	// }

	else if (pg3_wd)
	{
		clr_data(10);
		send_cmd(pg3_wd, 10);
		lcd_puts(3, 10, (int8_t *)"ON");
		lcd_puts(1, 10, (int8_t *)"OFF");
		lcd_puts(2, 10, (int8_t *)"OFF");
	}

	else
	{
		lcd_puts(3, 10, (int8_t *)"OFF");
		lcd_puts(1, 10, (int8_t *)"OFF");
		lcd_puts(2, 10, (int8_t *)"OFF");
	}
}

void update_screen_data_2()
{
	focus_upd = 1;
	lcd_puts(0, 0, (int8_t *)">");
	lcd_puts(0, 10, (int8_t *)"Disable");

	// lcd_puts(2, 10, (int8_t *)"OFF");
	// send_cmd(!pg3_md, 9);

	// lcd_puts(1, 10, (int8_t *)"OFF");
	// send_cmd(!pg3_sm, 8);

	// lcd_puts(3, 10, (int8_t *)"OFF");
	// send_cmd(!pg3_wd, 10);
	// send_cmd(1, 11);
}

void update_screen_data_3()
{

	// if (current_pos.key_number == _pos || current_pos.key_number == _neg)

	switch (current_pos.position_cursor)
	{

	case 7:
		lcd_puts(0, 0, (int8_t *)">");
		lcd_puts(1, 0, (int8_t *)" ");
		lcd_puts(2, 0, (int8_t *)" ");
		lcd_puts(3, 0, (int8_t *)" ");
		if (pg2_fc)
		{

			lcd_puts(0, 15, (int8_t *)"  ");
			lcd_puts(0, 10, (int8_t *)"Enable");
			if (focus_upd)
			{
				lcd_puts(3, 10, (int8_t *)"OFF");
				lcd_puts(1, 10, (int8_t *)"OFF");
				lcd_puts(2, 10, (int8_t *)"OFF");
				pg3_sm = 0;
				pg3_md = 0;
				pg3_wd = 0;
			}

			last_focus = data_reg.intensity;

			// if (pg3_md)
			// {
			// 	lcd_puts(2, 10, (int8_t *)"   ");
			// 	lcd_puts(1, 10, (int8_t *)"OFF");
			// 	lcd_puts(2, 10, (int8_t *)"ON");
			// 	lcd_puts(3, 10, (int8_t *)"OFF");
			// 	send_cmd(pg3_md, 9);
			// }
			// send_cmd(0, 11);
		}
		else
		{
			// focus_upd=1;
			if (focus_upd == 1)
			{
				lcd_puts(2, 10, (int8_t *)"OFF");
				lcd_puts(1, 10, (int8_t *)"OFF");
				lcd_puts(3, 10, (int8_t *)"OFF");
			}
			lcd_puts(0, 10, (int8_t *)"Disable");

			// lcd_puts(2, 10, (int8_t *)"OFF");
			send_cmd(!pg3_md, 9);

			// lcd_puts(1, 10, (int8_t *)"OFF");
			send_cmd(!pg3_sm, 8);

			// lcd_puts(3, 10, (int8_t *)"OFF");
			send_cmd(!pg3_wd, 10);

			// send_cmd(1, 11);
		}
		break;

	case 1:
		lcd_puts(0, 0, (int8_t *)" ");
		lcd_puts(1, 0, (int8_t *)">");
		lcd_puts(2, 0, (int8_t *)" ");
		lcd_puts(3, 0, (int8_t *)" ");

		if (pg3_sm)
		{
			focus_upd = 0;
			lcd_puts(1, 10, (int8_t *)"   ");
			lcd_puts(1, 10, (int8_t *)"ON");
			lcd_puts(2, 10, (int8_t *)"OFF");
			lcd_puts(3, 10, (int8_t *)"OFF");
			send_cmd(pg3_sm, 8);

			// send_cmd(pg3_sm, 8);
		}
		// else
		// {
		// 	lcd_puts(1, 10, (int8_t *)"OFF");
		// 	// lcd_puts(3, 10, (int8_t *)"ON");
		// 	send_cmd(!pg3_sm, 8);
		// }

		// // send_cmd(0, 9);
		// // send_cmd(0, 10);
		// // send_cmd(pg3_sm, 8);
		// if (last_ps == _pos || last_ps == _neg)
		// {
		// 	// send_cmd(0, 9);
		// 	// send_cmd(0, 10);
		// send_cmd(pg3_sm, 8);
		// }

		break;
	case 2:
		lcd_puts(0, 0, (int8_t *)" ");
		lcd_puts(1, 0, (int8_t *)" ");
		lcd_puts(2, 0, (int8_t *)">");
		lcd_puts(3, 0, (int8_t *)" ");
		if (pg3_md)
		{
			lcd_puts(2, 10, (int8_t *)"   ");
			lcd_puts(1, 10, (int8_t *)"OFF");
			lcd_puts(2, 10, (int8_t *)"ON");
			lcd_puts(3, 10, (int8_t *)"OFF");
			send_cmd(pg3_md, 9);
		}
		// else
		// {
		// 	lcd_puts(2, 10, (int8_t *)"OFF");
		// 	send_cmd(!pg3_md, 9);
		// }
		// if (last_ps == _pos || last_ps == _neg)
		// {
		// 	// send_cmd(0, 10);
		// 	// 	send_cmd(0, 8);
		// 	send_cmd(pg3_md, 9);
		// }

		break;
	case 3:
		lcd_puts(0, 0, (int8_t *)" ");
		lcd_puts(1, 0, (int8_t *)" ");
		lcd_puts(2, 0, (int8_t *)" ");
		lcd_puts(3, 0, (int8_t *)">");
		if (pg3_wd)
		{

			lcd_puts(3, 10, (int8_t *)"   ");
			lcd_puts(1, 10, (int8_t *)"OFF");
			lcd_puts(2, 10, (int8_t *)"OFF");
			lcd_puts(3, 10, (int8_t *)"ON");
			if (current_pos.key_number == _pos)
			{
				send_cmd(pg3_wd, 10);
			}
		}
		// else
		// {
		// 	lcd_puts(3, 10, (int8_t *)"OFF");
		// 	lcd_puts(1, 10, (int8_t *)"OFF");
		// 	lcd_puts(2, 10, (int8_t *)"OFF");
		// 	send_cmd(pg3_sm, 8);
		// 	send_cmd(pg3_md, 9);

		// 	send_cmd(!pg3_wd, 10);

		// 	// send_cmd(!pg3_wd, 10);
		// }
		// if (last_ps == _pos || last_ps == _neg)
		// {
		// 	// send_cmd(0, 8);
		// 	// send_cmd(0, 9);
		// 	send_cmd(pg3_wd, 10);
		// }
		break;
	}
	// }
}

void update_screen_data()
{
	char buffer[3];

	switch (current_pos.position_cursor)
	{
	case intensity:
		if (data_reg.depth == 1)
		{
			//			sprintf(buffer, "%02d", data_reg.intensity);
			//			last_inten = data_reg.intensity;
			//			send_cmd(7,intensity);
			// lcd_puts(1, 7, (int8_t *)"MAX");
			depth_press = 0;
		}
		else
		{
			clr_data(intensity);
			sprintf(buffer, "%02d", data_reg.intensity);
			lcd_puts(1, 7, (int8_t *)buffer);
		}

		clr_select();
		lcd_puts(1, 0, (int8_t *)">");
		send_cmd(data_reg.intensity, intensity);
		break;

	case color:
		if (data_reg.color == 1)
		{
			lcd_puts(2, 7, (int8_t *)"CW");
			send_cmd(5, color);
		}
		else if (data_reg.color == -1)
		{
			lcd_puts(2, 7, (int8_t *)"WW");
			send_cmd(-5, color);
		}

		else
		{
			lcd_puts(2, 7, (int8_t *)"NW");
			send_cmd(0, color);
		}

		// sprintf(buffer, "%02d", data_reg.color);
		// lcd_puts(2, 7, (int8_t *)buffer);
		clr_select();
		lcd_puts(2, 0, (int8_t *)">");
		// send_cmd(data_reg.color, color);
		break;

	case sensor:
		clr_data(sensor);
		clr_select();
		lcd_puts(3, 0, (int8_t *)">");
		send_cmd(data_reg.sensor, sensor);
		if (data_reg.sensor)
			lcd_puts(3, 7, (int8_t *)"ON");
		else
			lcd_puts(3, 7, (int8_t *)"OFF");
		break;

	case lamp:
		clr_data(lamp);
		clr_select();
		lcd_puts(1, 10, (int8_t *)">");
		if (current_pos.key_number == _pos || current_pos.key_number == _neg)
		{
			send_cmd(data_reg.lamp, lamp);
		}
		if (data_reg.lamp)
			lcd_puts(1, 17, (int8_t *)"ON");
		else
			lcd_puts(1, 17, (int8_t *)"OFF");

		break;

	case endo:
		clr_data(endo);
		clr_select();
		lcd_puts(2, 10, (int8_t *)">");

		// if (current_pos.key_number == _pos || current_pos.key_number == _neg)
		// {
		// 	send_cmd(data_reg.endo, endo);
		// }
		send_cmd(data_reg.endo, endo);
		if (data_reg.endo)
			lcd_puts(2, 17, (int8_t *)"ON");
		else
			lcd_puts(2, 17, (int8_t *)"OFF");

		break;

	case depth:
		clr_data(depth);
		clr_select();
		lcd_puts(3, 10, (int8_t *)">");
		if (current_pos.key_number == _pos || current_pos.key_number == _neg)
		{
			send_cmd(data_reg.depth, depth);
		}
		if (data_reg.depth)
		{
			// lcd_puts(1, 7, (int8_t *)"MAX");
			lcd_puts(3, 17, (int8_t *)"ON");
		}
		else
		{

			lcd_puts(3, 17, (int8_t *)"OFF");
			clr_data(intensity);
			sprintf(buffer, "%02d", data_reg.intensity);
			lcd_puts(1, 7, (int8_t *)buffer);
		}

		break;
	}
}

void send_cmd(int8_t x, int8_t mode)
{
	uint8_t data[5];
	data[0] = '@';
	data[4] = '#';
	switch (mode)
	{
	case intensity:
		data[1] = 'I';
		data[2] = '0';
		if (x == 10)
		{
			data[3] = ':'; //@I0:#
			HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		}
		else
		{
			data[3] = 48 + x;
			HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		}
		break;
	case color:
		if (x > 0)
		{
			data[1] = 'C'; //@c-0#
			data[2] = '+';
			data[3] = 48 + x;
		}
		else if (x < 0)
		{
			data[1] = 'C';
			data[2] = '-';
			data[3] = (48 + (x) * (-1));
		}
		else
		{
			data[1] = 'C';
			data[2] = '-'; //@c-0#
			data[3] = '0';
		}
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case sensor:
		// data[1] = 'I';data[2] = '0';data[3] = 48+x;
		// HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;

	case lamp:
		data[1] = 'L'; //@L_1#
		data[2] = '_';
		data[3] = 48 + x;
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case endo:
		data[1] = 'E';
		data[2] = '_';
		data[3] = 48 + x;
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case depth:
		data[1] = 'D';
		data[2] = '_';
		data[3] = 48 + x;
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case 8:

		if (x == 1)
		{
			data[1] = 'F';
			data[2] = '_';
			data[3] = 48 + 1;
		}
		else if (x == 0)
		{
			data[1] = 'R';
			data[2] = 'E';
			data[3] = 'S';
		}

		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case 9:
		if (x == 1)
		{
			data[1] = 'F';
			data[2] = '_';
			data[3] = 48 + 2;
		}
		else if (x == 0)
		{
			data[1] = 'R';
			data[2] = 'E';
			data[3] = 'S';
		}
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case 10:
		if (x == 1)
		{
			data[1] = 'F';
			data[2] = '_';
			data[3] = 48 + 3;
		}
		else if (x == 0)
		{
			data[1] = 'R';
			data[2] = 'E';
			data[3] = 'S';
		}
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	case 11:
		if (x == 0)
		{
			data[1] = 'F';
			data[2] = '_';
			data[3] = 48 + 5;
		}
		else if (x == 1)
		{
			data[1] = 'I';
			data[2] = '_';
			data[3] = 48 + last_focus;
		}
		//		data[1] = 'F';
		//		data[2] = '_';
		//		data[3] = 48 + 4;
		HAL_UART_Transmit(&huart1, &data[0], 5, 100);
		break;
	}
}

void page1_print(void)
{
	char buffer[3];
	lcd_puts(0, 0, (int8_t *)"______ COGNATE _____");
	lcd_puts(1, 1, (int8_t *)"INTEN");
	lcd_puts(2, 1, (int8_t *)"COLOR");
	lcd_puts(3, 1, (int8_t *)"SENSR");
	lcd_puts(1, 11, (int8_t *)"LAMP");
	lcd_puts(2, 11, (int8_t *)"ENDO");
	lcd_puts(3, 11, (int8_t *)"DEPTH");

	//	lcd_puts(1, 0, (int8_t *)">");

	sprintf(buffer, "%02d", data_reg.intensity);
	lcd_puts(1, 7, (int8_t *)buffer);

	sprintf(buffer, "%02d", data_reg.color);
	lcd_puts(2, 7, (int8_t *)"NW");

	clr_data(sensor);
	if (data_reg.sensor)
		lcd_puts(3, 7, (int8_t *)"ON");
	else
		lcd_puts(3, 7, (int8_t *)"OFF");

	clr_data(lamp);
	if (data_reg.lamp)
		lcd_puts(1, 17, (int8_t *)"ON");
	else
		lcd_puts(1, 17, (int8_t *)"OFF");

	clr_data(endo);
	if (data_reg.endo)
		lcd_puts(2, 17, (int8_t *)"ON");
	else
		lcd_puts(2, 17, (int8_t *)"OFF");

	clr_data(depth);
	if (data_reg.depth)
		lcd_puts(3, 17, (int8_t *)"ON");
	else
		lcd_puts(3, 17, (int8_t *)"OFF");

	lcd_puts(1, 0, (int8_t *)">");
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	static uint32_t _time;

	if ((HAL_GetTick() - _time) >= 100)
	{

		// if (GPIO_Pin == S_PRV_Pin)
		// {

		// 	key_pressed.prv = 1;
		// 	interrupt_reg.key_flag = 1;
		// 	// HAL_TIM_Base_Start_IT(&htim6);
		// 	// but_state = false;
		// }

		// if (GPIO_Pin == S_NEXT_Pin)
		// {

		// 	key_pressed.nxt = 1;
		// 	interrupt_reg.key_flag = 1;
		// }

		//  if (GPIO_Pin == DEPTH_Pin)
		// {

		// 	key_pressed.depth = 1;
		// 	interrupt_reg.key_flag = 1;
		// }

		// if (GPIO_Pin == CHANGE_N_Pin)
		// {

		// 	key_pressed.neg = 1;
		// 	interrupt_reg.key_flag = 1;
		// }

		// else if (GPIO_Pin == CHANGE_P_Pin)
		// {

		// 	key_pressed.pos = 1;
		// 	interrupt_reg.key_flag = 1;
		// }

		// else if (GPIO_Pin == I2C_INT_Pin)
		// {
		// 	//			interrupt_reg.key_flag = 1;
		// 	//			key_pressed.prv =1;
		// }

		_time = HAL_GetTick();
	}

	else
	{
		__NOP();
	}
}

void beep_sound()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_Delay(35);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
	HAL_Delay(35);
	//	beep_sound_flag = 0;
}

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
// 	if (GPIO_Pin == S_PRV_Pin && state == true)
// 	{
// 		HAL_TIM_Base_Start_IT(&htim6);
// 		but_state = false;
// 	}
// 	else
// 	{
// 		__NOP();
// 	}
// }

// typedef enum
// {
// 	NO_PRESS,
// 	SINGLE_PRESS,
// 	LONG_PRESS,
// 	DOUBLE_PRESS
// } eButtonEvent;

// bool buttonState()
// {
// 	static const uint32_t DEBOUNCE_MILLIS = 200;
// 	static bool buttonstate; // = HAL_GPIO_ReadPin( S_PRV_GPIO_Port, S_PRV_Pin ) == 1 ;
// 	buttonstate = HAL_GPIO_ReadPin(S_PRV_GPIO_Port, S_PRV_Pin) == 1;
// 	static uint32_t buttonstate_ts; // = HAL_GetTick() ;

// 	buttonstate_ts = HAL_GetTick();

// 	uint32_t now = HAL_GetTick();
// 	if (now - buttonstate_ts > DEBOUNCE_MILLIS)
// 	{
// 		if (buttonstate != HAL_GPIO_ReadPin(S_PRV_GPIO_Port, S_PRV_Pin) == 1)
// 		{
// 			buttonstate = !buttonstate;
// 			buttonstate_ts = now;
// 		}
// 	}
// 	return buttonstate;
// }

// eButtonEvent getButtonEvent()
// {
// 	static const uint32_t DOUBLE_GAP_MILLIS_MAX = 1000;
// 	static const uint32_t LONG_MILLIS_MIN = 3000;
// 	static uint32_t button_down_ts = 0;
// 	static uint32_t button_up_ts = 0;
// 	static bool double_pending = false;
// 	static bool long_press_pending = false;
// 	static bool button_down = false;
// 	;

// 	eButtonEvent button_event = NO_PRESS;
// 	uint32_t now = HAL_GetTick();

// 	// If state changed...
// 	if (button_down != buttonState())
// 	{
// 		button_down = !button_down;
// 		if (button_down)
// 		{
// 			// Timestamp button-down
// 			button_down_ts = now;
// 		}
// 		else
// 		{
// 			// Timestamp button-up
// 			button_up_ts = now;

// 			// If double decision pending...
// 			if (double_pending)
// 			{
// 				button_event = DOUBLE_PRESS;
// 				double_pending = false;
// 			}
// 			else
// 			{
// 				double_pending = true;
// 			}

// 			// Cancel any long press pending
// 			long_press_pending = false;
// 		}
// 	}

// 	// If button-up and double-press gap time expired, it was a single press
// 	if (!button_down && double_pending && now - button_up_ts > DOUBLE_GAP_MILLIS_MAX)
// 	{
// 		double_pending = false;
// 		button_event = SINGLE_PRESS;
// 	}
// 	// else if button-down for long-press...
// 	else if (!long_press_pending && button_down && now - button_down_ts > LONG_MILLIS_MIN)
// 	{
// 		button_event = LONG_PRESS;
// 		long_press_pending = false;
// 		double_pending = false;
// 	}

// 	return button_event;
// }

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	//	current_pos.position_cursor = 1;
	//	data_reg = {1,0,0,1,0,0};
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM6_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	MX_TIM14_Init();
	/* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim6);
	HAL_TIM_Base_Start_IT(&htim14);
	//	lcd_init();

	//	HAL_Delay(1000);
	//	home_page();
	//	HAL_Delay(1000);
	//	lcd_clear();

	HAL_Delay(100);
	lcd_init();
	HAL_Delay(100);
	home_page();
	HAL_Delay(100);
	lcd_clear();
	page1_print();
	HAL_Delay(100);
	//  	data_reg.sensor = 1;
	init_gesture();
	HAL_Delay(100);
	uint8_t sns_status = 0;
	uint8_t page_change_flag = 0;

	data_reg.sensor = 0;

	uint32_t temp_time = HAL_GetTick();
	HAL_Delay(10);

	pg3_sm = off;
	pg3_md = off;
	pg3_wd = off;

	send_cmd(1, intensity); // F1
	send_cmd(1, lamp);		// F2
	send_cmd(0, endo);		// F3
							/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		if (HAL_GPIO_ReadPin(S_PRV_GPIO_Port, S_PRV_Pin) == 0)
		{
			interrupt_reg.key_flag = 1;
			key_pressed.prv = 1;
		}

		if (HAL_GPIO_ReadPin(S_NEXT_GPIO_Port, S_NEXT_Pin) == 0)
		{
			interrupt_reg.key_flag = 1;
			key_pressed.nxt = 1;
		}

		if (HAL_GPIO_ReadPin(DEPTH_GPIO_Port, DEPTH_Pin) == 0)
		{
			interrupt_reg.key_flag = 1;
			key_pressed.depth = 1;
		}
		if (HAL_GPIO_ReadPin(CHANGE_N_GPIO_Port, CHANGE_N_Pin) == 0)
		{

			key_pressed.neg = 1;
			interrupt_reg.key_flag = 1;
		}

		if (HAL_GPIO_ReadPin(CHANGE_P_GPIO_Port, CHANGE_P_Pin) == 0)
		{

			key_pressed.pos = 1;
			interrupt_reg.key_flag = 1;
		}

		if (interrupt_reg.key_flag)
		{

			if (HAL_GPIO_ReadPin(S_PRV_GPIO_Port, S_PRV_Pin) == 0)
			{
				temp_time = HAL_GetTick();
				// temp_time = tt_cnt;
				HAL_Delay(50);
				while ((HAL_GetTick() - temp_time) <= 2000) // while ((tt_cnt - temp_time) >= 5000) // while ((HAL_GetTick() - temp_time) >= 5000)  // error tt_cnt
				{
					if (HAL_GPIO_ReadPin(S_PRV_GPIO_Port, S_PRV_Pin))
					{
						interrupt_reg.key_flag = 1;
						interrupt_reg.prv_long_press = 0;
						break;
					}
					interrupt_reg.key_flag = 0;
					interrupt_reg.prv_long_press = 1;
					HAL_Delay(50);
				}
			}

			if (HAL_GPIO_ReadPin(DEPTH_GPIO_Port, DEPTH_Pin) == 0)
			{
				temp_time = HAL_GetTick();
				HAL_Delay(50);
				while ((HAL_GetTick() - temp_time) <= 5000)
				{
					if (HAL_GPIO_ReadPin(DEPTH_GPIO_Port, DEPTH_Pin))
					{
						interrupt_reg.key_flag = 1;
						interrupt_reg.depth_long_press = 0;
						break;
					}
					interrupt_reg.key_flag = 0;
					interrupt_reg.depth_long_press = 1;
				}
			}

			if (interrupt_reg.key_flag)
			{
				beep_sound();
				// if (!data_reg.sensor)
				// {
				update_key_press();
				interrupt_reg.key_flag = 0;
				interrupt_reg.update_data = 1;
				// }

				interrupt_reg.key_flag = 0;
			}

			if (interrupt_reg.prv_long_press)
			{
				beep_sound();
				interrupt_reg.prv_long_press = 0;
				key_pressed.prv = 0;
				// interrupt_reg.update_data = 0;
				data_reg.sensor = !data_reg.sensor;
				clr_data(sensor);
				if (data_reg.sensor)
					lcd_puts(3, 7, (int8_t *)"ON");
				else
					lcd_puts(3, 7, (int8_t *)"OFF");
				sns_status = 1;
			}

			if (interrupt_reg.depth_long_press)
			{
				//				lcd_puts(0, 0, (int8_t *)"...");
				page_change_flag = 1;
				beep_sound();
				current_pos.key_number = _depth;
				interrupt_reg.depth_long_press = 0;
				key_pressed.depth = 0;
				interrupt_reg.key_flag = 0;
				//				interrupt_reg.update_data = 1;
			}
			//			else if (interrupt_reg.gesture_flag)
			//			{
			//				beep_sound();
			//				interrupt_reg.gesture_flag = 0;
			//				interrupt_reg.update_data = 1;
			//			}
		}

		if (data_reg.sensor)
		{
			HAL_Delay(10);
			//			test[0] = 's';
			//			test[1] = '0';
			if (gestureAvailable())
			{
				//				test[1] = 'L';
				//				HAL_UART_Transmit(&huart1, &test[0], 5, 100);
				uint8_t gesture = readGesture();
				switch (gesture)
				{
				case 0:
					beep_sound();
					// test[1] = 'z';
					// HAL_UART_Transmit(&huart1, &test[0], 5, 100);
					key_pressed.prv = 1;
					update_key_press();
					// lcd_puts(2, 1, (int8_t *)"UUUU");
					interrupt_reg.update_data = 1;
					break;

				case 1:
					beep_sound();
					// test[1] = '1';
					// HAL_UART_Transmit(&huart1, &test[0], 5, 100);
					key_pressed.nxt = 1;
					update_key_press();
					// lcd_puts(2, 1, (int8_t *)"DDD");
					interrupt_reg.update_data = 1;
					break;

				case 2:
					beep_sound();
					// 					test[1] = '2';
					// 					HAL_UART_Transmit(&huart1, &test[0], 5, 100);
					key_pressed.neg = 1;
					update_key_press();
					// lcd_puts(2, 1, (int8_t *)"LLL");
					interrupt_reg.update_data = 1;
					break;

				case 3:
					beep_sound();
					// test[1] = '3';
					// HAL_UART_Transmit(&huart1, &test[0], 5, 100);
					key_pressed.pos = 1;
					update_key_press();
					// lcd_puts(2, 1, (int8_t *)"RRRR");
					interrupt_reg.update_data = 1;
					break;

				default:
					break;
				}
			}
		}

		if (interrupt_reg.update_data)
		{
			//			char buffers[3];
			update_new_data();

			if (current_pos.position_cursor == 7)
			{

				//					if(last_pg != 2)
				//					{
				//						lcd_clear();
				//						page_2_print();
				//					}
				//					update_screen_data_2();

				if (pg2_fc == 0)
				{
					//						lcd_clear();
					if (last_pg != 2)
					{
						lcd_clear();
						page_2_print();
					}

					last_pg = 2;
					update_screen_data_2();
				}
				else if (pg2_fc == 1)
				{
					//						lcd_clear();
					if (last_pg != 3)
					{
						lcd_clear();
						page_3_print();
						last_pg = 3;
					}
					//						lcd_clear();
					//						page_3_print();

					update_screen_data_3();
				}

				//					sprintf(buffers, "%02d", current_pos.position_cursor);
				//					lcd_puts(3, 16, (int8_t *)buffers);
			}
			else if ((current_pos.position_cursor >= 1) || (current_pos.position_cursor <= 6))
			{
				if (pg2_fc == 0)
				{

					if (last_pg != 1)
					{
						lcd_clear();
						page1_print();
					}

					last_pg = 1;
					update_screen_data();

					//					if(current_pos.position_cursor >= 1)
					//					{
					//	//					current_pos.position_cursor = 1;
					//						lcd_clear();
					//						page1_print();
					//					}

					//					sprintf(buffers, "%02d", current_pos.position_cursor);
					//				lcd_puts(2, 16, (int8_t *)buffers);
				}

				else if (pg2_fc == 1)
				{
					if (current_pos.position_cursor > 3)
					{
						current_pos.position_cursor = 7;
					}

					if (last_pg != 3)
					{
						lcd_clear();
						page_3_print();
					}

					last_pg = 3;
					update_screen_data_3();
					//					sprintf(buffers, "%02d", current_pos.position_cursor);
					//					lcd_puts(3, 16, (int8_t *)buffers);
				}
			}

			interrupt_reg.update_data = 0;
		}

		if (page_change_flag)
		{
			clock_page();
			Total_Time_Print(tt_cnt); // tt_cnt timer_intrupt
			HAL_Delay(5000);
			lcd_clear();
			page1_print();
			page_change_flag = 0;
			interrupt_reg.update_data = 0;
			interrupt_reg.key_flag = 0;
		}
		HAL_Delay(50);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x0000020B;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */
}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void)
{

	/* USER CODE BEGIN TIM6_Init 0 */

	/* USER CODE END TIM6_Init 0 */

	/* USER CODE BEGIN TIM6_Init 1 */

	/* USER CODE END TIM6_Init 1 */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 32000 - 1;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 1000 - 1;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM6_Init 2 */

	/* USER CODE END TIM6_Init 2 */
}

/**
 * @brief TIM14 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM14_Init(void)
{

	/* USER CODE BEGIN TIM14_Init 0 */

	/* USER CODE END TIM14_Init 0 */

	/* USER CODE BEGIN TIM14_Init 1 */

	/* USER CODE END TIM14_Init 1 */
	htim14.Instance = TIM14;
	htim14.Init.Prescaler = 32000 - 1;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 3000;
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM14_Init 2 */

	/* USER CODE END TIM14_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_RS485Ex_Init(&huart1, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, D4_Pin | D5_Pin | D6_Pin | D7_Pin | E_Pin | RS_Pin | BUZZER_Pin | PA7_Pin | LED2_Pin | LED1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, PB0_Pin | GPIO_PIN_1 | PB2_Pin | GPIO_PIN_10, GPIO_PIN_RESET);

	/*Configure GPIO pins : D4_Pin D5_Pin D6_Pin D7_Pin
							 E_Pin RS_Pin BUZZER_Pin PA7_Pin
							 LED2_Pin LED1_Pin */
	GPIO_InitStruct.Pin = D4_Pin | D5_Pin | D6_Pin | D7_Pin | E_Pin | RS_Pin | BUZZER_Pin | PA7_Pin | LED2_Pin | LED1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0_Pin PB1 PB2_Pin PB10 */
	GPIO_InitStruct.Pin = PB0_Pin | GPIO_PIN_1 | PB2_Pin | GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : S_PRV_Pin S_NEXT_Pin DEPTH_Pin IR_N_Pin
							 CHANGE_N_Pin */
	GPIO_InitStruct.Pin = S_PRV_Pin | S_NEXT_Pin | DEPTH_Pin | IR_N_Pin | CHANGE_N_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : CHANGE_P_Pin */
	GPIO_InitStruct.Pin = CHANGE_P_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CHANGE_P_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : I2C_INT_Pin */
	GPIO_InitStruct.Pin = I2C_INT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(I2C_INT_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
