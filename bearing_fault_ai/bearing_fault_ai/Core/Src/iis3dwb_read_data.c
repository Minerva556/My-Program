/*
 * iis3dwb_read_data.c
 *
 *  Created on: Mar 26, 2025
 *      Author: John
 */
#include <string.h>
#include <stdio.h>
#include "iis3dwb_reg.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "NanoEdgeAI.h"
#include "knowledge.h"

#define    BOOT_TIME        10 //ms
//片选信号
#define    CS_up_GPIO_Port  GPIOF
#define    CS_up_Pin        GPIO_PIN_5

uint16_t id_class = 0; // Point to id class (see argument of neai_classification fct)
float input_user_buffer[DATA_INPUT_USER * AXIS_NUMBER]; // Buffer of input values
float output_class_buffer[CLASS_NUMBER]; // Buffer of class probabilities
const char *id2class[CLASS_NUMBER + 1] = { // Buffer for mapping class id to class name
	"unknown",
	"type_4",
	"type_3",
	"type_2",
	"type_1",
	"normal",
};

static int16_t data_raw_acceleration[3];
static float acceleration_mg[3];
static int16_t data_raw_temperature;
static float temperature_degC[1];
static uint8_t whoamI, rst;
static int buffer_index = 0; // 静态变量记录缓冲区写入位置
//主要数据记录数组
static float data_all[5];
//数据标志位
const uint8_t begin[] = {0xAA, 0x55};
const uint8_t end[] = {0x55, 0xAA};



static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);

void fill_buffer(float sample_buffer[]);

void iis3dwb_read_data_polling(void)
{
	//分类器初始化
	enum neai_state error_code = neai_classification_init(knowledge);
	//错误处理
	if (error_code != NEAI_OK) {
		unsigned char warning[] = "model init fail...\r\n";
		HAL_UART_Transmit(&huart1, (uint8_t *)warning, sizeof(warning), 0xffff);
		while(1);
	}
  stmdev_ctx_t dev_ctx;
  /* Initialize mems driver interface */
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hspi3;
  /* Init test platform */

  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);
  /* Check device ID */
  iis3dwb_device_id_get(&dev_ctx, &whoamI);

  if (whoamI != IIS3DWB_ID)
      while (1);
  /* Restore default configuration */
  iis3dwb_reset_set(&dev_ctx, PROPERTY_ENABLE);

  do {
     iis3dwb_reset_get(&dev_ctx, &rst);
   } while (rst);

  /* Enable Block Data Update */
  iis3dwb_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  /* Set Output Data Rate */
  iis3dwb_xl_data_rate_set(&dev_ctx, IIS3DWB_XL_ODR_26k7Hz);
  /* Set full scale */
  iis3dwb_xl_full_scale_set(&dev_ctx, IIS3DWB_2g);
  /* Configure filtering chain(No aux interface)
   * Accelerometer low pass filter path
   */
  iis3dwb_xl_filt_path_on_out_set(&dev_ctx, IIS3DWB_LP_ODR_DIV_100);

  /* Read samples in polling mode (no int) */
  while (1) {
    uint8_t reg;
    /* Read output only if new xl value is available */
    iis3dwb_xl_flag_data_ready_get(&dev_ctx, &reg);

    if (reg) {
      /* Read acceleration field data */
      iis3dwb_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
      acceleration_mg[0] =
        iis3dwb_from_fs2g_to_mg(data_raw_acceleration[0]);
      acceleration_mg[1] =
        iis3dwb_from_fs2g_to_mg(data_raw_acceleration[1]);
      acceleration_mg[2] =
        iis3dwb_from_fs2g_to_mg(data_raw_acceleration[2]);
    }
    //数据填充
    fill_buffer(input_user_buffer);
    //结果分类
    neai_classification(input_user_buffer, output_class_buffer, &id_class);
    //温度获取
    iis3dwb_temp_flag_data_ready_get(&dev_ctx, &reg);
    if (reg) {
      /* Read temperature data */
      iis3dwb_temperature_raw_get(&dev_ctx, &data_raw_temperature);
      temperature_degC[0] = iis3dwb_from_lsb_to_celsius(data_raw_temperature);
    }
    //将数据放入到一个float类型数组
    for(int i = 0; i < 5; i++)
    {
    	if(i < 3)
    	{
    		data_all[i] = acceleration_mg[i];
    	}else if(i >= 3 && i < 4)
    	{
    		data_all[i] = temperature_degC[0];
    	}else if(i >= 4 && i < 5)
    	{
    		data_all[i] = (float)id_class;
    	}
    }
    //数据发送
    	HAL_UART_Transmit(&huart1, (uint8_t *)begin, sizeof(begin), 0xffff);
    	HAL_UART_Transmit(&huart1, (float *)data_all, sizeof(data_all), 0xffff);
    	HAL_UART_Transmit(&huart1, (uint8_t *)end, sizeof(end), 0xffff);
    	HAL_Delay(200);
  }
}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 1000);
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
    /* Read command */
    reg |= 0x80;
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Receive(handle, bufp, len, 1000);
    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
  return 0;
}

static void platform_delay(uint32_t ms)
{
  HAL_Delay(ms);
}

void fill_buffer(float sample_buffer[])
{
	for(int axis = 0; axis < AXIS_NUMBER; axis++)
	{ // 遍历加速度三个轴
		sample_buffer[buffer_index] = acceleration_mg[axis] * 0.001f; // 填充当前轴数据
		buffer_index++;
		// 检查是否填满缓冲区，若填满则重置索引循环填充
		if (buffer_index >= DATA_INPUT_USER * AXIS_NUMBER)
		{
			buffer_index = 0;
		}
	}
}
