#include "stm32f0xx_hal.h"
#include "gesture.h"

extern I2C_HandleTypeDef hi2c1;

extern UART_HandleTypeDef huart2;

uint8_t sensitivity = 30;
// _gestureIn = 0;
int _gestureDirectionX = 0;
int _gestureDirectionY = 0;
int _gestureDirInX = 0;
int _gestureDirInY = 0;
int _gestureSensitivity ;
// _detectedGesture = GESTURE_NONE;

uint8_t init_gesture()
{
	uint8_t temp;
	// verify device active
	temp = read8(APDS9960_ID);
	if (temp != APDS9960_ID_1)
	{
		return false;
	}
	// Disable everything
	if (!(write8(APDS9960_ENABLE, 0x00)))
	{
		return false;
	}

	proximityEnabled = 0;
	gesture_enable = 0;
	// Upon power up, the wait time register is set to 0xFF.
	if (!(write8(APDS9960_WTIME, 0xFF)))
	{
		return false;
	}
	/* The Gesture Pulse Count Register sets Pulse Width Modified current during a Gesture Pulse. The Gesture pulse count
  register bits set the number of pulses to be output on the LDR pin. The Gesture Length register bits set the amount of
  time the LDR pin is sinking current during a gesture pulse
  2- 16μs
  Gesture Pulses - 8
  */
	// 16us, 16 pulses // default is: 0x40 = 8us, 1 pulse
	if (!(write8(APDS9960_GPULSE, 0x8F)))
	{
		return false;
	}

	/* The proximity pulse count register bits set the number of pulses to be output on the LDR pin.	*/

	// 16us, 16 pulses // default is: 0x40 = 8us, 1 pulse
	if (!(write8(APDS9960_PPULSE, 0x8F)))
	{
		return false;
	}

	// setGestureIntEnable

	temp = read8(APDS9960_GCONF4); // READ CONF4
	temp |= 0x02;
	write8(APDS9960_GCONF4, temp); // // wite CONF4

	// setGestureMode
	temp = read8(APDS9960_GCONF4); // READ CONF4
	temp |= 0x01;
	write8(APDS9960_GCONF4, temp); // // wite CONF4
	gesture_enable = 1;

	// enablePower
	temp = read8(APDS9960_ENABLE); // READ APDS9960_ENABLE
	temp |= 0x01;
	write8(APDS9960_ENABLE, temp); // // wite APDS9960_ENABLE

	// enableWait
	temp = read8(APDS9960_ENABLE); // READ APDS9960_ENABLE
	temp |= 0x68;
	write8(APDS9960_ENABLE, temp); // // wite APDS9960_ENABLE
	temp = read8(APDS9960_ENABLE); // READ APDS9960_ENABLE
	// ADC Integration Time Register (0x81)
	// set ADC integration time to 10 ms
	temp = 256 - (10 / 2.78);
	write8(APDS9960_ATIME, temp); // // wite APDS9960_ATIME

	// set ADC gain 4x (0x00 => 1x, 0x01 => 4x, 0x02 => 16x, 0x03 => 64x)
	write8(APDS9960_ATIME, 0x00); // // wite APDS9960_ATIME

	if (sensitivity > 100)
	{
		sensitivity = 100;
	}
	_gestureSensitivity = 100 - sensitivity;

	HAL_Delay(10);

	// enablePower
	temp = read8(APDS9960_ENABLE); // READ APDS9960_ENABLE
	temp |= 0x01;
	write8(APDS9960_ENABLE, temp); // // wite APDS9960_ENABLE

	return true;
}

uint8_t gestureFIFOAvailable()
{
	uint8_t _bytes;
	_bytes = read8(APDS9960_GSTATUS); // operational condition of the gesture state machine.
	if ((_bytes & 0x01) == 0x00)
	{
		return false;
	}

	_bytes = read8(APDS9960_GFLVL); // number of datasets that are currently available in the FIFO for read.
	if (_bytes == 0x00)
	{
		return false;
	}

	return _bytes;
}

uint8_t readGesture()
{
	uint8_t gesture = _detectedGesture;

	_detectedGesture = GESTURE_NONE;

	return gesture;
}

int32_t handleGesture()
{
	const uint8_t gestureThreshold = 80;

	while (1)
	{

		uint8_t available = gestureFIFOAvailable();
		if (available <= 0)
			return 0;
		uint8_t fifo_data[128];

		//		uint8_t ret;
		if ((HAL_I2C_Mem_Read(&hi2c1, APDS9960_I2C_ADDR, APDS9960_GFIFO_U, 0x01, &fifo_data[0], (available * 4), 10)) != HAL_OK)
		{
			return false;
		}

		//	    if (bytes_read == 0)
		//	      return 0;
		for (int i = 0; i + 3 < (available * 4); i += 4)
		{
			uint8_t u, d, l, r;
			u = fifo_data[i];
			d = fifo_data[i + 1];
			l = fifo_data[i + 2];
			r = fifo_data[i + 3];

			//	          debugPrint(&huart2, "u");
			//	          debugPrintln(&huart2, u);
			//	          debugPrint(&huart2, "d");
			//	          debugPrintln(&huart2, d);
			//	          debugPrint(&huart2, "l");
			//	          debugPrintln(&huart2, l);
			//	          debugPrint(&huart2, "r");
			//	          debugPrintln(&huart2, r);

			if (u < gestureThreshold && d < gestureThreshold && l < gestureThreshold && r < gestureThreshold)
			{
				_gestureIn = true;
				if (_gestureDirInX != 0 || _gestureDirInY != 0)
				{
					int totalX = _gestureDirInX - _gestureDirectionX;
					int totalY = _gestureDirInY - _gestureDirectionY;

					//		          debugPrint(&huart2, "out");
					//		          debugPrint(&huart2, totalX);
					//		          debugPrint(&huart2, ", ");
					//		          debugPrintln(&huart2, totalY);

					if (totalX < -_gestureSensitivity)
					{
						_detectedGesture = GESTURE_LEFT;
					}
					if (totalX > _gestureSensitivity)
					{
						_detectedGesture = GESTURE_RIGHT;
					}
					if (totalY < -_gestureSensitivity)
					{
						_detectedGesture = GESTURE_DOWN;
					}
					if (totalY > _gestureSensitivity)
					{
						_detectedGesture = GESTURE_UP;
					}
					_gestureDirectionX = 0;
					_gestureDirectionY = 0;
					_gestureDirInX = 0;
					_gestureDirInY = 0;
				}
				continue;
			}

			_gestureDirectionX = r - l;
			_gestureDirectionY = u - d;
			if (_gestureIn)
			{
				_gestureIn = false;
				_gestureDirInX = _gestureDirectionX;
				_gestureDirInY = _gestureDirectionY;
			}
		}
	}
}

uint8_t gestureAvailable()
{
	uint8_t r;
	// enable gesture

	// setGestureMode
	r = read8(APDS9960_ENABLE); // READ CONF4
	if ((r & 0b01000000) != 0)
	{
		gesture_enable = true;
		// return true;
	}
	else
	{
		r |= 0b01000001;
		write8(APDS9960_ENABLE, r);
		r = read8(APDS9960_ENABLE); // READ CONF4
		if ((r & 0b01000000) != 0)
		{
			gesture_enable = true;
			// return true;
		}
	}

	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) != 0)
	{
		return 0;
	}

	if (!gestureFIFOAvailable())
	{
		return false;
	}

	handleGesture();

	//	 if (proximityEnabled)
	//	  {
	//			r = read8(APDS9960_GCONF4); // READ CONF4
	//			r &= 0xFE;
	//			write8(APDS9960_GCONF4,r); // // wite CONF4
	//			gesture_enable = 1;
	//	  }
	// HAL_Delay(10);

	return (_detectedGesture == GESTURE_NONE) ? 0 : 1;
}

uint8_t write8(uint8_t reg, uint8_t value)
{

	if ((HAL_I2C_Mem_Write(&hi2c1, APDS9960_I2C_ADDR, reg, 0x01, &value, 0x01, 10)) != HAL_OK)
	{
		return false;
	}

	return true;
}

uint8_t read8(uint8_t reg)
{

	uint8_t ret;
	if ((HAL_I2C_Mem_Read(&hi2c1, APDS9960_I2C_ADDR, reg, 0x01, &ret, 0x01, 10)) != HAL_OK)
	{
		return false;
	}

	return ret;
}

void debugPrint(UART_HandleTypeDef *huart, char _out[])
{
	HAL_UART_Transmit(huart, (uint8_t *)_out, strlen(_out), 10);
}

void debugPrintln(UART_HandleTypeDef *huart, char _out[])
{
	HAL_UART_Transmit(huart, (uint8_t *)_out, strlen(_out), 10);
	char newline[2] = "\r\n";
	HAL_UART_Transmit(huart, (uint8_t *)newline, 2, 10);
}
