#include "i2c.h"
#include "OLED.h"
#include "Max30102.h"
#include "algorithm.h"

//дData��max30102��
HAL_StatusTypeDef Max30102_WriteData(uint8_t MemAddress,uint8_t Command,uint16_t SendCount)
{
	HAL_StatusTypeDef status=HAL_OK;
	status=HAL_I2C_Mem_Write(&hi2c1,Max30102_Write_Address,MemAddress,I2C_MEMADD_SIZE_8BIT,&Command,SendCount,100);
	return status;
}

//������
HAL_StatusTypeDef Max30102_ReadData(uint8_t DatAddress,uint8_t *Data,uint16_t ReceiveCount)
{
	HAL_StatusTypeDef status=HAL_OK;
	status=HAL_I2C_Mem_Read(&hi2c1,Max30102_Read_Address,DatAddress,I2C_MEMADD_SIZE_8BIT,Data,ReceiveCount,100);
	return status;
}

void Max30102_FIFO_ReadData(uint8_t DatAddress,uint8_t SixData[6],uint16_t Size)
{
	uint8_t temp;
	Max30102_ReadData(REG_INTR_STATUS_1,&temp,1);
	Max30102_ReadData(REG_INTR_STATUS_2,&temp,1);
	Max30102_ReadData(DatAddress,SixData,Size);
}

uint8_t TempData[6];
uint32_t red_buffer[500];  //�������red�����ڼ�������
uint32_t ir_buffer[500];  //�������� ir�����ڼ���Ѫ��
int32_t ir_buffer_length=500;   //����ǰ500�������õ�������
int32_t pn_SpO2_value;   //Ѫ��ʵ��ֵ
int8_t SpO2_valid;        //Ѫ��ֵ��Ч��־
int32_t pn_hr_value;    //����ʵ��ֵ
int8_t hr_valid;         //������Ч��־
uint32_t red_max=0,red_min=0x3FFFF;  //���ȡֵ��Χ
uint32_t prev_data;      //ǰһ�ε�ֵ
float f_temp;           //��ʱ����
int32_t n_brightness;    //��ȷ���� 

void Max30102_Safety(void)
{
	for(int i=0;i<ir_buffer_length;i++)
	{
		while(Max30102_INT==GPIO_PIN_SET); //�ȴ��ж�������Ӧ��Ĭ��Ϊ��,�������������
		Max30102_FIFO_ReadData(REG_FIFO_DATA,TempData,6);
		red_buffer[i]=((TempData[0]&0x03)<<16) | (TempData[1]<<8) | (TempData[2]); //ǰ��λ�������HR
		ir_buffer[i]=((TempData[3]&0x03)<<16) | (TempData[4]<<8) | (TempData[5]); //����λ�������BO
		if(red_min>red_buffer[i]) red_min=red_buffer[i];  //���µ�ǰ��Сֵ
		if(red_max<red_buffer[i]) red_max=red_buffer[i];  //���µ�ǰ���ֵ
	}
	maxim_heart_rate_and_oxygen_saturation(ir_buffer,ir_buffer_length,red_buffer,&pn_SpO2_value,&SpO2_valid,&pn_hr_value,&hr_valid);
	//����500��������ͨ���㷨�ó�ʵ������Ѫ��ֵ
}

void Max30102_Calculate_HR_BO_Value(int32_t* HR_Value,int8_t* HR_Valid,int32_t* BO_Value,int8_t* BO_Valid)
{
	for(int i=100;i<500;i++)  //�������е�100~500����ֵ��ǰŲ��0~400
	{
		red_buffer[i-100]=red_buffer[i];
		ir_buffer[i-100]=ir_buffer[i];
		if(red_min>red_buffer[i]) red_min=red_buffer[i];  //���µ�ǰ��Сֵ
		if(red_max<red_buffer[i]) red_max=red_buffer[i];  //���µ�ǰ���ֵ
	}
	for(int i=400;i<500;i++)  //ʵ��ֻȡ100������ֵ������
	{
		prev_data=red_buffer[i-1];
		while(Max30102_INT==1); //�ȴ��ж�������Ӧ��Ĭ��Ϊ��,�������������
		Max30102_FIFO_ReadData(REG_FIFO_DATA,TempData,6);
		red_buffer[i]=((TempData[0]&0x03)<<16) | (TempData[1]<<8) | (TempData[2]); //ǰ��λ�������HR
		ir_buffer[i]=((TempData[3]&0x03)<<16) | (TempData[4]<<8) | (TempData[5]); //����λ�������BO
		if(red_buffer[i]>prev_data)
		{    //���ʹ�ʽ��|��һ�ε�ֵ-��ǰֵ| / (���ֵ-��Сֵ) * 255
			f_temp=(float)(red_buffer[i]-prev_data)/(red_max-red_min)*255;
			n_brightness-=(int)f_temp;
			if(n_brightness<0) n_brightness=0;
		}
		else
		{
			f_temp=(float)(prev_data-red_buffer[i])/(red_max-red_min)*255;
			n_brightness+=(int)f_temp;
			if(n_brightness>255) n_brightness=255;
		}
		*HR_Value=pn_hr_value;
		*HR_Valid=hr_valid;
		*BO_Value=pn_SpO2_value;
		*BO_Valid=SpO2_valid;
	}
	maxim_heart_rate_and_oxygen_saturation(ir_buffer,ir_buffer_length,red_buffer,&pn_SpO2_value,&SpO2_valid,&pn_hr_value,&hr_valid);
}

void Max30102_Reset(void)
{
	Max30102_WriteData(REG_MODE_CONFIG,0x40,1);
	Max30102_WriteData(REG_MODE_CONFIG,0x40,1);
}

void Max30102_Init(void)
{
	Max30102_Reset();
	
	Max30102_WriteData(REG_INTR_ENABLE_1,0xc0,1);	// INTR setting
	Max30102_WriteData(REG_INTR_ENABLE_2,0x00,1);
	Max30102_WriteData(REG_FIFO_WR_PTR,0x00,1);  	//FIFO_WR_PTR[4:0]
	Max30102_WriteData(REG_OVF_COUNTER,0x00,1);  	//OVF_COUNTER[4:0]
	Max30102_WriteData(REG_FIFO_RD_PTR,0x00,1);  	//FIFO_RD_PTR[4:0]
	Max30102_WriteData(REG_FIFO_CONFIG,0x0f,1);  	//sample avg = 1, fifo rollover=false, fifo almost full = 17
	Max30102_WriteData(REG_MODE_CONFIG,0x03,1);  	//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	Max30102_WriteData(REG_SPO2_CONFIG,0x27,1);  	// SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)  
	Max30102_WriteData(REG_LED1_PA,0x24,1);   	//Choose value for ~ 7mA for LED1
	Max30102_WriteData(REG_LED2_PA,0x24,1);   	// Choose value for ~ 7mA for LED2
	Max30102_WriteData(REG_PILOT_PA,0x7f,1);   	// Choose value for ~ 25mA for Pilot LED
}

void Read(void)
{
	
	static 	uint8_t sampleIndex = 0;
	int32_t HR_Value,BO_Value;
	int8_t HR_Valid,BO_Valid;
	Max30102_Calculate_HR_BO_Value(&HR_Value,&HR_Valid,&BO_Value,&BO_Valid);
	if(HR_Valid==1 && BO_Valid==1)
	{
		

//		OLED_ShowString(1,1,"HR:",8,1);
//		OLED_ShowString(1,20,"BO:",8,1);
//		OLED_ShowNum(20,1,HR_Value,3,8,1);
//		OLED_ShowNum(20,20,BO_Value,3,8,1);
//		OLED_Refresh();
		Draw_HeartRate_Waveform(HR_Value);
	}
	
	HAL_Delay(1000);
}


#define GRAPH_WIDTH 128
#define GRAPH_HEIGHT 64
#define GRAPH_START_X 0
#define GRAPH_START_Y 0

uint16_t heartRateBuffer[GRAPH_WIDTH]; //�洢��ʷ��������
uint8_t bufferIndex = 0;

void Draw_HeartRate_Waveform(uint32_t currentHR) {
    //���»�����
    heartRateBuffer[bufferIndex] = currentHR;
    bufferIndex = (bufferIndex + 1) % GRAPH_WIDTH;
    
    // ����
    OLED_Clear();
    
    // ����������
    OLED_DrawLine(GRAPH_START_X, GRAPH_START_Y, GRAPH_START_X, GRAPH_START_Y + GRAPH_HEIGHT,1);  //���Ʊ��ϵ����ŵ���
    OLED_DrawLine(GRAPH_START_X, GRAPH_START_Y + GRAPH_HEIGHT/2, 
                 GRAPH_START_X + GRAPH_WIDTH, GRAPH_START_Y + GRAPH_HEIGHT/2,1);                //�����м�ĺ���
    
    // �ҵ����ֵ����Сֵ�Ե�����ʾ����
    uint16_t maxHR = 0, minHR = 65535;
    for(int i=0; i<GRAPH_WIDTH; i++) {
        if(heartRateBuffer[i] > maxHR) maxHR = heartRateBuffer[i];
        if(heartRateBuffer[i] < minHR) minHR = heartRateBuffer[i];
    }
    
    //ȷ���к������ʾ��Χ
    if(maxHR - minHR < 20) {
        maxHR = minHR + 20;
//			  if(minHR>10)
//					minHR-=10;
    }
    
    //���Ʋ���
    for(int i=1; i<GRAPH_WIDTH; i++) {
        int prevIndex = (bufferIndex + i - 1) % GRAPH_WIDTH;
        int currIndex = (bufferIndex + i) % GRAPH_WIDTH;
        
        int prevY = GRAPH_HEIGHT - (heartRateBuffer[prevIndex] - minHR) * GRAPH_HEIGHT / (maxHR - minHR);
        int currY = GRAPH_HEIGHT - (heartRateBuffer[currIndex] - minHR) * GRAPH_HEIGHT / (maxHR - minHR);
        
			  if((heartRateBuffer[prevIndex]>300)&&(heartRateBuffer[currIndex]>300))
				{
											  OLED_DrawLine(GRAPH_START_X + i-1, GRAPH_START_Y + GRAPH_HEIGHT, 
              GRAPH_START_X + i, GRAPH_START_Y + GRAPH_HEIGHT,1);
				}
				else
				{
											  OLED_DrawLine(GRAPH_START_X + i-1, GRAPH_START_Y + prevY, 
              GRAPH_START_X + i, GRAPH_START_Y + currY,1);
				}

    }

				OLED_Refresh();
	}