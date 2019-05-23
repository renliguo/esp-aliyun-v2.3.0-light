
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver_control.h"
#include "esp_err.h"
//#include "c_types.h"

#include "iot_import.h"

#define GPIO_OUTPUT_LED    2
#define GPIO_OUTPUT_PIN_SEL  1ULL<<GPIO_OUTPUT_LED

#define AM2320_SENSOR_ADDR                 0xB8             /*!< slave address for MPU6050 sensor */

#define I2C_EXAMPLE_MASTER_SCL_IO           10                /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           9               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */
#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */

uint8_t IIC_TX_Buffer[]={0x03,0x00,0x04}; //读温湿度命令（无CRC校验）
uint8_t IIC_RX_Buffer[10];
int GPIO_flag=0;

void GPIO_init(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO0
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void LED_toggle(void)
{
	if(GPIO_flag==0)
	{
		gpio_set_level(GPIO_OUTPUT_LED, 1);
		GPIO_flag=1;
	}
	else
	{
		gpio_set_level(GPIO_OUTPUT_LED, 0);
		GPIO_flag=0;
	}

}

/**
 * @brief i2c master initialization
 */
 esp_err_t AM2320_i2c_master_init(void)
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = 0;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = 0;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}

 static void AM2320_wakeUp(void)
 {

	my_i2c_master_start(I2C_EXAMPLE_MASTER_NUM);       // 启动I2C
	my_i2c_master_writeByte(I2C_EXAMPLE_MASTER_NUM,0xB8); // 发送器件地址
	my_i2c_master_getAck(I2C_EXAMPLE_MASTER_NUM);// 唤醒指令时 传感器不会回ACK 但是第一定要发检测ACK的时钟 否则会出错
	i2c_master_set_dc(I2C_EXAMPLE_MASTER_NUM,1, 0);
	i2c_master_wait(1000);
	my_i2c_master_stop(I2C_EXAMPLE_MASTER_NUM);

 }

///计算CRC校验码
unsigned int CRC16(unsigned char *ptr, unsigned char len)
{
   unsigned int crc=0xffff;
   unsigned char i;
   while(len--)
   {
       crc ^=*ptr++;
       for(i=0;i<8;i++)
	   {
	       if(crc & 0x1)
		   {
		      crc>>=1;
			  crc^=0xa001;
		   }
		   else
		   {
		      crc>>=1;
		   }
	   }
   }
   return crc;
}
///检测CRC校验码是否正确
unsigned char CheckCRC(unsigned char *ptr,unsigned char len)
{
  unsigned int crc;
	crc=(unsigned int)CRC16(ptr,len-2);
	if(ptr[len-1]==(crc>>8) && ptr[len-2]==(crc & 0x00ff))
	{
	    return 0xff;
	}
	else
	{
	   return 0x0;
	}
}

void Clear_Data (void)
{
	int i;
	for(i=0;i<6;i++)
	{
	IIC_RX_Buffer[i] = 0x00;
	}
}


void AM2320_GetValue(int* hum1,int* temp1)
{
	uint8_t i;
	int a,b;
    int hum,temp;
    char uart_temp[30];
    Clear_Data();                       // 清除收到数据
    AM2320_wakeUp();	                // 唤醒传感器
    WriteNByte(I2C_EXAMPLE_MASTER_NUM,0xB8,IIC_TX_Buffer,3);
    i2c_master_wait(2000);                        //发送读取或写数据命令后，至少等待2MS（给传感器返回数据作时间准备）
    ReadNByte(I2C_EXAMPLE_MASTER_NUM,0xB8,IIC_RX_Buffer,8);    //读返回数据
    i2c_master_wait(2);
    i2c_master_set_dc(I2C_EXAMPLE_MASTER_NUM,1, 1);	                        //确认释放总线

    if(CheckCRC(IIC_RX_Buffer,8))
         {
            a =(( IIC_RX_Buffer[2]<<8 )+ IIC_RX_Buffer[3]);
            b =(( IIC_RX_Buffer[4]<<8)+ IIC_RX_Buffer[5]);

            hum=a/10;
            temp=b/10;
            *hum1=hum;
            *temp1=temp;

            printf("hum: %d  temp: %d \n\r",hum,temp);

         }
         else
         {
            printf("Data: CRC Wrong\n");
         }

//        for(i = 0; i< 8; i++)
//        {
//            printf("%d Byte: %x \n\r", i+1,IIC_RX_Buffer[i]);
//        }
}


