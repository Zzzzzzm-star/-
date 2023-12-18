//////////////////////////////////////////////////////////////////////////////////	 
//  ��������   : ͨ��DHT11�����ʪ�ȣ�OLED��ʾ�ʹ��ڴ�ӡ



//           ----------------------------------------------------------------

//              Ӳ��ԭ��ͼ

//              GND  ��Դ��
//              VCC  3.3v��Դ
//              SCL   PB8��SCL��
//              SDA   PB9��SDA��
//              RES  PA2��SPIģ��ĳ�IICģ����Ҫ�Ӵ����ţ�IICģ���û�����ԣ�


//						��ʪ��DHT11 		�ź������Ž��� PB11
//			Usart1		DEBUG   ���ڴ�ӡ�ӿ�---- -TX:PA9  RX:PA10
//          Usart2      stm32-esp8266ͨѶ����-----TX:PA2 RX:PA3


//                      BEEP = PA4          PB12
//                      LED0 = PA5   ��Ϊ   PB13
//                      LED1 = PA6          PB14
//                      KEY0 = PB0
//                      KEY1 = PB1
//                      ESP8266��λ����     PC14




 
 
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


u8 alarmFlag = 0;//�Ƿ񱨾��ı�־
u8 alarm_is_free = 10;//�������Ƿ��ֶ�����������ֶ�����������Ϊ0


u8 temp=0;      
u8 humi=0;   

u8 Led_Status = 0;//������ʾLED0��״̬

char PUB_BUF[256];//�ϴ����ݵ�buf
const char *devSubTopic[] = {"/mysmarthome/sub"};
const char devPubTopic[] = "/mysmarthome/pub";
   
   
int main(void)
{

    unsigned short timeCount = 0;//���ͼ����
    
    unsigned char *dataPtr = NULL;
    
    Delay_Init();           //��ʼ����ʱ����
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����2
    
    LED_Init();             //��ʼ����LED���ӵ�Ӳ���ӿ�
    KEY_Init();         	//��ʼ���밴�����ӵ�Ӳ���ӿ�
    EXTIX_Init();           //�ⲿ�жϳ�ʼ��
    BEEP_Init();            //��ʼ��������
    DHT11_Init();           //��ʼ��DHT11
    
    
    
       
    Usart1_Init(115200);//debug����
    Usart2_Init(115200);//stm32-esp8266ͨѶ����
    
    
	OLED_Init();
    TIM2_Int_Init(4999,7199);
    TIM3_Int_Init(2999,7199);
    
    ESP8266_Init();					//��ʼ��ESP8266
    UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
   

//	OLED_ColorTurn(0);//0������ʾ��1 ��ɫ��ʾ
//  OLED_DisplayTurn(0);//0������ʾ 1 ��Ļ��ת��ʾ


	while(OneNet_DevLink())			//����OneNET
		delay_ms(500);
	BEEP = 0;//������ʾ����ɹ�
    delay_ms(250);
    BEEP = 1;
    OneNet_Subscribe(devSubTopic, 1);

	while(1)
	{
    
        
        if(timeCount % 40 == 0 )//1000ms /25 = 40 һ��ִ��һ�Σ�Լֵ������ϵͳʱ�����л�����
        {
        
      /*************��ʪ�ȴ�������ȡ�¶�***************/
        DHT11_Read_Data(&temp,&humi);//��ȡ��ʪ��ֵ    
        UsartPrintf(USART1,"�¶ȣ�%d�� ʪ�ȣ�%d%%",temp,humi);
//      delay_ms(1000);
//        printf("�¶�: %d  C",temp);      //���ڴ�ӡ��ʪ��
//        printf("ʪ��: %d %%\r\n",humi);
                
            
            if(alarm_is_free == 10)//����������Ȩ�Ƿ����  alarm_is_free == 10,ִ�����³��򣬳�ʼֵΪ10
            {
                if(temp < 35 && humi < 80)alarmFlag = 0;
                else alarmFlag =1;
            }
            if(alarm_is_free < 10)alarm_is_free++;
//          UsartPrintf(USART_DEBUG, "alarm_is_free = %d\r\n",alarm_is_free);
//          UsartPrintf(USART_DEBUG, "alarmFlag = %d\r\n",alarmFlag);
            
        }
        	if(++timeCount >= 200)				//  5000ms / 25 = 200  ���ͼ��5s
		{
            Led_Status = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);//��ȡLED0��״̬
            
			UsartPrintf(USART_DEBUG, "OneNet_Publish\r\n");
			
			sprintf(PUB_BUF,"{\"Temp\":%d,\"Hum\":%d,\"Led\":%d,\"Beep\":%d}",temp,humi,Led_Status?0:1,alarmFlag);//LED�ǵ͵�ƽ�������Ƶ�״̬���ƽ�Ƿ������� 
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
    
		
//		OLED_ShowString(1,1,"Temp:");    //1��1����ʾTemp:
//		OLED_ShowString(1,9,"C");        //1��9����ʾC���޷�ʶ��棬��  C���
//		OLED_ShowString(2,1,"Humi:");    //2��1����ʾHumi��
//		OLED_ShowString(2,9,"%");        //2��9����ʾ%
//		OLED_ShowSignedNum(1,6,temp,2);  //��ʾ2λ���ַ���  �¶�
//		OLED_ShowSignedNum(2,6,humi,2);  //��ʾ2λ���ַ���  ʪ��
		
//		delay_ms(1000);
//		
//		OLED_ShowChinese(0,0,0,16,1);    //�� x,y
//		OLED_ShowChinese(18,0,1,16,1);  //��
//		OLED_ShowNum(50,0,temp,2,16,1); //��ʾ�¶�ֵ
//		OLED_ShowString(36,0,":",16,1);
//		
//		OLED_ShowChinese(0,16,2,16,1);  //ʪ
//		OLED_ShowChinese(18,16,1,16,1); //��	
//		OLED_ShowNum(50,16,humi,2,16,1);//��ʾʪ��ֵ
//		OLED_ShowString(36,16,":",16,1);  
//		OLED_ShowString(68,16,"%",16,1);   
//		delay_ms(1000);
// if(timeCount % 100 == 0)
//        {
//            if(humi > 80 || temp > 30)alarmFlag = 1;
//            else alarmFlag = 0;
//            
//            t=KEY_Scan(0);	//�õ���ֵ
//                                       
//                switch(t)
//                {				 
//                    case KEY0_PRES:	//ͬʱ����LED0,LED1��ת 
//                        LED0=!LED0;
//                        break;
//                    
//                    case KEY1_PRES:	//����LED1��ת	 
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


	

