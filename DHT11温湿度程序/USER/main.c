//////////////////////////////////////////////////////////////////////////////////	 
//  功能描述   : 通过DHT11检测温湿度，OLED显示和串口打印



//           ----------------------------------------------------------------

//              硬件原理图

//              GND  电源地
//              VCC  3.3v电源
//              SCL   PB8（SCL）
//              SDA   PB9（SDA）
//              RES  PA2（SPI模块改成IIC模块需要接此引脚，IIC模块用户请忽略）


//						温湿度DHT11 		信号线引脚接线 PB11
//			Usart1		DEBUG   串口打印接口---- -TX:PA9  RX:PA10
//          Usart2      stm32-esp8266通讯串口-----TX:PA2 RX:PA3


//                      BEEP = PA4          PB12
//                      LED0 = PA5   改为   PB13
//                      LED1 = PA6          PB14
//                      KEY0 = PB0
//                      KEY1 = PB1
//                      ESP8266复位引脚     PC14




 
 
//              ----------------------------------------------------------------
//******************************************************************************/
#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "timer.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "dht11.h"
#include "usart.h"	 
#include "timer.h"
#include "led.h"
#include "beep.h"
#include "stdio.h"
#include "key.h"
#include "esp8266.h"
#include "onenet.h"
#include "string.h"
#include "exti.h"


u8 alarmFlag = 0;//是否报警的标志
u8 alarm_is_free = 10;//报警器是否被手动操作，如果手动操作即设置为0


u8 temp=0;      
u8 humi=0;   

u8 Led_Status = 0;//用来表示LED0的状态

char PUB_BUF[256];//上传数据的buf
const char *devSubTopic[] = {"/mysmarthome/sub"};
const char devPubTopic[] = "/mysmarthome/pub";
   
   
int main(void)
{

    unsigned short timeCount = 0;//发送间隔量
    
    unsigned char *dataPtr = NULL;
    
    Delay_Init();           //初始化延时函数
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组2
    
    LED_Init();             //初始化与LED连接的硬件接口
    KEY_Init();         	//初始化与按键连接的硬件接口
    EXTIX_Init();           //外部中断初始化
    BEEP_Init();            //初始化蜂鸣器
    DHT11_Init();           //初始化DHT11
    
    
    
       
    Usart1_Init(115200);//debug串口
    Usart2_Init(115200);//stm32-esp8266通讯串口
    
    
	OLED_Init();
    TIM2_Int_Init(4999,7199);
    TIM3_Int_Init(2999,7199);
    
    ESP8266_Init();					//初始化ESP8266
    UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
   

//	OLED_ColorTurn(0);//0正常显示，1 反色显示
//  OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示


	while(OneNet_DevLink())			//接入OneNET
		delay_ms(500);
	BEEP = 0;//鸣叫提示接入成功
    delay_ms(250);
    BEEP = 1;
    OneNet_Subscribe(devSubTopic, 1);

	while(1)
	{
    
        
        if(timeCount % 40 == 0 )//1000ms /25 = 40 一秒执行一次（约值，可能系统时钟运行会有误差）
        {
        
      /*************温湿度传感器获取温度***************/
        DHT11_Read_Data(&temp,&humi);//读取温湿度值    
        UsartPrintf(USART1,"温度：%d℃ 湿度：%d%%",temp,humi);
//      delay_ms(1000);
//        printf("温度: %d  C",temp);      //串口打印温湿度
//        printf("湿度: %d %%\r\n",humi);
                
            
            if(alarm_is_free == 10)//报警器控制权是否空闲  alarm_is_free == 10,执行以下程序，初始值为10
            {
                if(temp < 35 && humi < 80)alarmFlag = 0;
                else alarmFlag =1;
            }
            if(alarm_is_free < 10)alarm_is_free++;
//          UsartPrintf(USART_DEBUG, "alarm_is_free = %d\r\n",alarm_is_free);
//          UsartPrintf(USART_DEBUG, "alarmFlag = %d\r\n",alarmFlag);
            
        }
        	if(++timeCount >= 200)				//  5000ms / 25 = 200  发送间隔5s
		{
            Led_Status = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);//读取LED0的状态
            
			UsartPrintf(USART_DEBUG, "OneNet_Publish\r\n");
			
			sprintf(PUB_BUF,"{\"Temp\":%d,\"Hum\":%d,\"Led\":%d,\"Beep\":%d}",temp,humi,Led_Status?0:1,alarmFlag);//LED是低电平触发，灯的状态与电平是反过来的 
            OneNet_Publish(devPubTopic, PUB_BUF);
			
			timeCount = 0;
			ESP8266_Clear();
		}
		
		dataPtr = ESP8266_GetIPD(3);
		if(dataPtr != NULL)
		OneNet_RevPro(dataPtr);
        delay_ms(10);
    }
}         
//            
//        if(t == KEY0_PRES )LED0=!LED0;
//        else if (t == KEY1_PRES)LED1=!LED1;
//                delay_ms(10);
    
		
//		OLED_ShowString(1,1,"Temp:");    //1行1列显示Temp:
//		OLED_ShowString(1,9,"C");        //1行9列显示C，无法识别℃，用  C替代
//		OLED_ShowString(2,1,"Humi:");    //2行1列显示Humi：
//		OLED_ShowString(2,9,"%");        //2行9列显示%
//		OLED_ShowSignedNum(1,6,temp,2);  //显示2位的字符串  温度
//		OLED_ShowSignedNum(2,6,humi,2);  //显示2位的字符串  湿度
		
//		delay_ms(1000);
//		
//		OLED_ShowChinese(0,0,0,16,1);    //温 x,y
//		OLED_ShowChinese(18,0,1,16,1);  //度
//		OLED_ShowNum(50,0,temp,2,16,1); //显示温度值
//		OLED_ShowString(36,0,":",16,1);
//		
//		OLED_ShowChinese(0,16,2,16,1);  //湿
//		OLED_ShowChinese(18,16,1,16,1); //度	
//		OLED_ShowNum(50,16,humi,2,16,1);//显示湿度值
//		OLED_ShowString(36,16,":",16,1);  
//		OLED_ShowString(68,16,"%",16,1);   
//		delay_ms(1000);
// if(timeCount % 100 == 0)
//        {
//            if(humi > 80 || temp > 30)alarmFlag = 1;
//            else alarmFlag = 0;
//            
//            t=KEY_Scan(0);	//得到键值
//                                       
//                switch(t)
//                {				 
//                    case KEY0_PRES:	//同时控制LED0,LED1翻转 
//                        LED0=!LED0;
//                        break;
//                    
//                    case KEY1_PRES:	//控制LED1翻转	 
//                        LED1=!LED1;
//                        break;
//                    default:
//                          delay_ms(10);
//                    
//          
//                }
//		         
//        }
//		OLED_Refresh();


	

