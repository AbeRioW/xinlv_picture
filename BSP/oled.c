#include "oled.h"
#include "stdlib.h"
#include "oledfont.h" 
#include "stdio.h"

uint8_t OLED_GRAM[144][8];

static I2C_HandleTypeDef *hi2c_oled;

//·´ÏÔº¯Êý
void OLED_ColorTurn(uint8_t i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xA6,OLED_CMD);//Õý³£ÏÔÊ¾
		}
	if(i==1)
		{
			OLED_WR_Byte(0xA7,OLED_CMD);//·´É«ÏÔÊ¾
		}
}

//ÆÁÄ»Ðý×ª180¶È
void OLED_DisplayTurn(uint8_t i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xC8,OLED_CMD);//Õý³£ÏÔÊ¾
			OLED_WR_Byte(0xA1,OLED_CMD);
		}
	if(i==1)
		{
			OLED_WR_Byte(0xC0,OLED_CMD);//·´×ªÏÔÊ¾
			OLED_WR_Byte(0xA0,OLED_CMD);
		}
}

void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{	
	uint8_t i;			  
	if(cmd)
	  OLED_DC_Set();
	else 
	  OLED_DC_Clr();		  
	OLED_CS_Clr();
	for(i=0;i<8;i++)
	{			  
		OLED_SCL_Clr();
		if(dat&0x80)
		   OLED_SDA_Set();
		else 
		   OLED_SDA_Clr();
		OLED_SCL_Set();
		dat<<=1;   
	}				 		  
	OLED_CS_Set();
	OLED_DC_Set();   	  
}

//¿ªÆôOLEDÏÔÊ¾ 
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//µçºÉ±ÃÊ¹ÄÜ
	OLED_WR_Byte(0x14,OLED_CMD);//¿ªÆôµçºÉ±Ã
	OLED_WR_Byte(0xAF,OLED_CMD);//µãÁÁÆÁÄ»
}

//¹Ø±ÕOLEDÏÔÊ¾ 
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//µçºÉ±ÃÊ¹ÄÜ
	OLED_WR_Byte(0x10,OLED_CMD);//¹Ø±ÕµçºÉ±Ã
	OLED_WR_Byte(0xAE,OLED_CMD);//¹Ø±ÕÆÁÄ»
}

//¸üÐÂÏÔ´æµ½OLED	
void OLED_Refresh(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	   OLED_WR_Byte(0xb0+i,OLED_CMD); //ÉèÖÃÐÐÆðÊ¼µØÖ·
	   OLED_WR_Byte(0x00,OLED_CMD);   //ÉèÖÃµÍÁÐÆðÊ¼µØÖ·
	   OLED_WR_Byte(0x10,OLED_CMD);   //ÉèÖÃ¸ßÁÐÆðÊ¼µØÖ·
	   for(n=0;n<128;n++)
		 OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);
  }
}
//ÇåÆÁº¯Êý
void OLED_Clear(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	   for(n=0;n<128;n++)
			{
			 OLED_GRAM[n][i]=0;//Çå³ýËùÓÐÊý¾Ý
			}
  }
	OLED_Refresh();//¸üÐÂÏÔÊ¾
}

//»­µã 
//x:0~127
//y:0~63
//t:1 Ìî³ä 0,Çå¿Õ	
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	if(t){OLED_GRAM[x][i]|=n;}
	else
	{
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
		OLED_GRAM[x][i]|=n;
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	}
}

//»­Ïß
//x1,y1:Æðµã×ø±ê
//x2,y2:½áÊø×ø±ê
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t mode)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //¼ÆËã×ø±êÔöÁ¿ 
	delta_y=y2-y1;
	uRow=x1;//»­ÏßÆðµã×ø±ê
	uCol=y1;
	if(delta_x>0)incx=1; //ÉèÖÃµ¥²½·½Ïò 
	else if (delta_x==0)incx=0;//´¹Ö±Ïß 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//Ë®Æ½Ïß 
	else {incy=-1;delta_y=-delta_x;}
	if(delta_x>delta_y)distance=delta_x; //Ñ¡È¡»ù±¾ÔöÁ¿×ø±êÖá 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		OLED_DrawPoint(uRow,uCol,mode);//»­µã
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}
//x,y:Ô²ÐÄ×ø±ê
//r:Ô²µÄ°ë¾¶
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r)
{
	int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)      
    {
        OLED_DrawPoint(x + a, y - b,1);
        OLED_DrawPoint(x - a, y - b,1);
        OLED_DrawPoint(x - a, y + b,1);
        OLED_DrawPoint(x + a, y + b,1);
 
        OLED_DrawPoint(x + b, y + a,1);
        OLED_DrawPoint(x + b, y - a,1);
        OLED_DrawPoint(x - b, y - a,1);
        OLED_DrawPoint(x - b, y + a,1);
        
        a++;
        num = (a * a + b * b) - r*r;//¼ÆËã»­µÄµãÀëÔ²ÐÄµÄ¾àÀë
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}



//ÔÚÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö×Ö·û,°üÀ¨²¿·Ö×Ö·û
//x:0~127
//y:0~63
//size1:Ñ¡Ôñ×ÖÌå 6x8/6x12/8x16/12x24
//mode:0,·´É«ÏÔÊ¾;1,Õý³£ÏÔÊ¾
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode)
{
	uint8_t i,m,temp,size2,chr1;
	uint8_t x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //µÃµ½×ÖÌåÒ»¸ö×Ö·û¶ÔÓ¦µãÕó¼¯ËùÕ¼µÄ×Ö½ÚÊý
	chr1=chr-' ';  //¼ÆËãÆ«ÒÆºóµÄÖµ
	for(i=0;i<size2;i++)
	{
		if(size1==8)
			  {temp=asc2_0806[chr1][i];} //µ÷ÓÃ0806×ÖÌå
		else if(size1==12)
        {temp=asc2_1206[chr1][i];} //µ÷ÓÃ1206×ÖÌå
		else if(size1==16)
        {temp=asc2_1608[chr1][i];} //µ÷ÓÃ1608×ÖÌå
		else if(size1==24)
        {temp=asc2_2412[chr1][i];} //µ÷ÓÃ2412×ÖÌå
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
  }
}


//ÏÔÊ¾×Ö·û´®
//x,y:Æðµã×ø±ê  
//size1:×ÖÌå´óÐ¡ 
//*chr:×Ö·û´®ÆðÊ¼µØÖ· 
//mode:0,·´É«ÏÔÊ¾;1,Õý³£ÏÔÊ¾
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t mode)
{
	while((*chr>=' ')&&(*chr<='~'))//ÅÐ¶ÏÊÇ²»ÊÇ·Ç·¨×Ö·û!
	{
		OLED_ShowChar(x,y,*chr,size1,mode);
		if(size1==8)x+=6;
		else x+=size1/2;
		chr++;
  }
}

//m^n
uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

//ÏÔÊ¾Êý×Ö
//x,y :Æðµã×ø±ê
//num :ÒªÏÔÊ¾µÄÊý×Ö
//len :Êý×ÖµÄÎ»Êý
//size:×ÖÌå´óÐ¡
//mode:0,·´É«ÏÔÊ¾;1,Õý³£ÏÔÊ¾
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t mode)
{
	uint8_t t,temp,m=0;
	if(size1==8)m=2;
	for(t=0;t<len;t++)
	{
		temp=(num/OLED_Pow(10,len-t-1))%10;
			if(temp==0)
			{
				OLED_ShowChar(x+(size1/2+m)*t,y,'0',size1,mode);
      }
			else 
			{
			  OLED_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,mode);
			}
  }
}

//ÏÔÊ¾ºº×Ö
//x,y:Æðµã×ø±ê
//num:ºº×Ö¶ÔÓ¦µÄÐòºÅ
//mode:0,·´É«ÏÔÊ¾;1,Õý³£ÏÔÊ¾
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t mode)
{
	uint8_t m,temp;
	uint8_t x0=x,y0=y;
	uint16_t i,size3=(size1/8+((size1%8)?1:0))*size1;  //µÃµ½×ÖÌåÒ»¸ö×Ö·û¶ÔÓ¦µãÕó¼¯ËùÕ¼µÄ×Ö½ÚÊý
	for(i=0;i<size3;i++)
	{
		if(size1==16)
				{temp=Hzk1[num][i];}//µ÷ÓÃ16*16×ÖÌå
		else if(size1==24)
				{temp=Hzk2[num][i];}//µ÷ÓÃ24*24×ÖÌå
		else if(size1==32)       
				{temp=Hzk3[num][i];}//µ÷ÓÃ32*32×ÖÌå
		else if(size1==64)
				{temp=Hzk4[num][i];}//µ÷ÓÃ64*64×ÖÌå
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((x-x0)==size1)
		{x=x0;y0=y0+8;}
		y=y0;
	}
}

//num ÏÔÊ¾ºº×ÖµÄ¸öÊý
//space Ã¿Ò»±éÏÔÊ¾µÄ¼ä¸ô
//mode:0,·´É«ÏÔÊ¾;1,Õý³£ÏÔÊ¾
void OLED_ScrollDisplay(uint8_t num,uint8_t space,uint8_t mode)
{
	uint8_t i,n,t=0,m=0,r;
	while(1)
	{
		if(m==0)
		{
	    OLED_ShowChinese(128,24,t,16,mode); //Ð´ÈëÒ»¸öºº×Ö±£´æÔÚOLED_GRAM[][]Êý×éÖÐ
			t++;
		}
		if(t==num)
			{
				for(r=0;r<16*space;r++)      //ÏÔÊ¾¼ä¸ô
				 {
					for(i=1;i<144;i++)
						{
							for(n=0;n<8;n++)
							{
								OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
							}
						}
           OLED_Refresh();
				 }
        t=0;
      }
		m++;
		if(m==16){m=0;}
		for(i=1;i<144;i++)   //ÊµÏÖ×óÒÆ
		{
			for(n=0;n<8;n++)
			{
				OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
			}
		}
		OLED_Refresh();
	}
}

//x,y£ºÆðµã×ø±ê
//sizex,sizey,Í¼Æ¬³¤¿í
//BMP[]£ºÒªÐ´ÈëµÄÍ¼Æ¬Êý×é
//mode:0,·´É«ÏÔÊ¾;1,Õý³£ÏÔÊ¾
void OLED_ShowPicture(uint8_t x,uint8_t y,uint8_t sizex,uint8_t sizey,uint8_t BMP[],uint8_t mode)
{
	uint16_t j=0;
	uint8_t i,n,temp,m;
	uint8_t x0=x,y0=y;
	sizey=sizey/8+((sizey%8)?1:0);
	for(n=0;n<sizey;n++)
	{
		 for(i=0;i<sizex;i++)
		 {
				temp=BMP[j];
				j++;
				for(m=0;m<8;m++)
				{
					if(temp&0x01)OLED_DrawPoint(x,y,mode);
					else OLED_DrawPoint(x,y,!mode);
					temp>>=1;
					y++;
				}
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y0=y0+8;
				}
				y=y0;
     }
	 }
}
//OLEDµÄ³õÊ¼»¯
void OLED_Init(void)
{
//	GPIO_InitTypeDef  GPIO_InitStructure;
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);	 //Ê¹ÄÜA¶Ë¿ÚÊ±ÖÓ
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_15;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ÍÆÍìÊä³ö
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//ËÙ¶È50MHz
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);	  //³õÊ¼»¯GPIOA
// 	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_15);
	
	OLED_RES_Clr();
//	delay_ms(200);
	HAL_Delay(200);
	OLED_RES_Set();
	
	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0×óÓÒ·´ÖÃ 0xa1Õý³£
	OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0ÉÏÏÂ·´ÖÃ 0xc8Õý³£
	OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//
	OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
	OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
	OLED_Clear();
	OLED_WR_Byte(0xAF,OLED_CMD);
}


void oled_showFnum(uint8_t x,uint8_t y,float fnum,uint8_t size1,uint8_t mode)
{
		uint8_t Data[]=" ";
		sprintf(Data,"%.03f C",fnum);
		OLED_ShowString(x,y,(uint8_t*)Data,size1,mode);
}

void OLED_ShowWaveform(uint8_t *data, uint8_t length) {
    static uint8_t prevData[128] = {0};
    uint8_t i;
    
    // æ¸…é™¤ä¸Šä¸€å¸§æ³¢å½¢
    for(i = 0; i < length; i++) {
        if(prevData[i] > 0) {
            OLED_DrawPoint(i, prevData[i], 0);
        }
    }
    
    // ç»˜åˆ¶æ–°æ³¢å½¢
    for(i = 0; i < length; i++) {
        if(data[i] < 64) {
            OLED_DrawPoint(i, data[i], 1);
            prevData[i] = data[i];
        }
    }
    
    OLED_Refresh();
}
