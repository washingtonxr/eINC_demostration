#include "gdeh0154d27.h"
#include "gpio.h"
#include "freertos.h"
#include "cmsis_os.h"
#include "main.h"
#include "lut_data.h"
#include "picture_rom.h"
#include "number_rom.h"
#include "english_alphabet.h"
#include "spi.h"

static void Epaper_READBUSY(void);
static void Epaper_Spi_WriteByte(uint8_t TxData);
static void Epaper_Write_Command(uint8_t cmd);
static void Epaper_Write_Data(uint8_t data);
static void EPD_select_LUT(const uint8_t * wave_data);
static void EPD_HW_Init(void);
static void EPD_Update(void);
static void EPD_WhiteScreen_ALL(const uint8_t *datas);
static void EPD_DeepSleep(void);
static void EPD_Part_Update(void);
static void EPD_Part_Init(void);
static void EPD_SetRAMValue_BaseMap(const uint8_t *datas);
static void EPD_Part_off(void);
static void EPD_WhiteScreen_BW(void);
static void EPD_WhiteScreen_Black(void);
static void EPD_WhiteScreen_White(void);
static void EPD_Dis_Part(uint32_t x_start, uint32_t y_start,const uint8_t *datas,
                                uint32_t PART_COLUMN, uint32_t PART_LINE);
static void EPD_Dis_Part_myself(uint32_t x_startA, uint32_t y_startA,const uint8_t * datasA,
                                uint32_t x_startB, uint32_t y_startB,const uint8_t * datasB,
                                uint32_t x_startC, uint32_t y_startC,const uint8_t * datasC,
                                uint32_t x_startD, uint32_t y_startD,const uint8_t * datasD,
                                uint32_t x_startE, uint32_t y_startE,const uint8_t * datasE,
                                uint32_t PART_COLUMN, uint32_t PART_LINE);

static void Epaper_READBUSY(void)
{ 
    while(1){	 //=1 BUSY
        osDelay(1);
        if(HAL_GPIO_ReadPin(GPIOE, eINC_BUSY_Pin) == GPIO_PIN_RESET) 
            break;
        osDelay(10);
    }  
}

static void Epaper_Spi_WriteByte(uint8_t TxData)
{				   			 
#if 0
	uint8_t TempData;
	uint8_t scnt;

	TempData = TxData;

    /* Reset clock. */
    HAL_GPIO_WritePin(GPIOD, eINC_CLK_Pin, GPIO_PIN_RESET);
    for(scnt = 0; scnt < 8; scnt++){
        if(TempData & 0x80)
            HAL_GPIO_WritePin(GPIOD, eINC_DAT_Pin, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOD, eINC_DAT_Pin, GPIO_PIN_RESET);
        /* Set clock. */
        HAL_GPIO_WritePin(GPIOD, eINC_CLK_Pin, GPIO_PIN_SET);
        /* Reset clock. */
        HAL_GPIO_WritePin(GPIOD, eINC_CLK_Pin, GPIO_PIN_RESET);
        /* Load next bit. */
        TempData = TempData << 1;
    }
#else
    uint8_t ret;    
    SPI1_Enable();
    ret = HAL_SPI_Transmit_IT(&hspi1, &TxData, 1);
    if(HAL_OK != ret){
        Debug_printf("Error: HAL_SPI_Transmit_IT write failed.\n");
    }
    SPI1_Disable();
#endif
}

static void Epaper_Write_Command(uint8_t cmd)
{
    HAL_GPIO_WritePin(GPIOD, eINC_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, eINC_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOE, eINC_DC_Pin, GPIO_PIN_RESET);
    Epaper_Spi_WriteByte(cmd);
    HAL_GPIO_WritePin(GPIOE, eINC_CS_Pin, GPIO_PIN_SET);
}

static void Epaper_Write_Data(uint8_t data)
{
    HAL_GPIO_WritePin(GPIOD, eINC_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, eINC_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOE, eINC_DC_Pin, GPIO_PIN_SET);    // D/C#   0:command  1:data
    Epaper_Spi_WriteByte(data);
    HAL_GPIO_WritePin(GPIOD, eINC_CS_Pin, GPIO_PIN_SET);
}

static void EPD_select_LUT(const uint8_t * wave_data)
{        
    uint8_t count;
    Epaper_Write_Command(0x32);
    for(count = 0; count < 30; count++)
        Epaper_Write_Data(wave_data[count]);
}


static void EPD_HW_Init(void)
{
    HAL_GPIO_WritePin(GPIOE, eINC_RES_Pin, GPIO_PIN_RESET);
    osDelay(1); 
    HAL_GPIO_WritePin(GPIOE, eINC_RES_Pin, GPIO_PIN_SET);
    osDelay(1);
    
    /* GDEH0154D27, 200^2 */
    Epaper_Write_Command(0x01); //Driver output control      
    Epaper_Write_Data(0xC7);  //Y
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);

    Epaper_Write_Command(0x0C); //softstart  
    Epaper_Write_Data(0xD7);   //is necessary
    Epaper_Write_Data(0xD6);   //is necessary
    Epaper_Write_Data(0x9D);  //is necessary

    Epaper_Write_Command(0x2C);     //VCOM Voltage
    Epaper_Write_Data(0x77);    

    Epaper_Write_Command(0x3A);     //Dummy Line     
    Epaper_Write_Data(0x1A);    
    Epaper_Write_Command(0x3B);     //Gate time 
    Epaper_Write_Data(0X08);  

    Epaper_Write_Command(0x11); //data entry mode       
    Epaper_Write_Data(0x01);

    Epaper_Write_Command(0x3C); //BorderWavefrom
    Epaper_Write_Data(0x33);    

    Epaper_Write_Command(0x44); //set Ram-X address start/end position   
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x18);    //0x18-->(24+1)*8=200

    Epaper_Write_Command(0x45); //set Ram-Y address start/end position          
    Epaper_Write_Data(0xC7);   //0xC7-->(199+1)=200
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00); 

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0xC7);  //Y
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    EPD_select_LUT(LUT_DATA); //LUT
}

static void EPD_Update(void)
{   
    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0xC7);   
    Epaper_Write_Command(0x20); 
    Epaper_READBUSY();  
}

static void EPD_WhiteScreen_ALL(const uint8_t *datas)
{
    uint32_t i;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(i = 0; i < ALLSCREEN_GRAGHBYTES; i++){               
        Epaper_Write_Data(*datas);
        datas++;
    }
    EPD_Update();
}

static void EPD_DeepSleep(void)
{  	
    Epaper_Write_Command(0x10); //enter deep sleep
    Epaper_Write_Data(0x01); 
    osDelay(100);
}

static void EPD_Part_Update(void)
{
	Epaper_Write_Command(0x22); 
	Epaper_Write_Data(0x04);   //different
	Epaper_Write_Command(0x20); 
	Epaper_READBUSY(); 		
}

static void EPD_Part_Init(void)
{
    EPD_HW_Init();
    EPD_select_LUT(LUT_DATA_part);

    //POWER ON
    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0xC0);   
    Epaper_Write_Command(0x20); 
    Epaper_READBUSY();  

    Epaper_Write_Command(0x3C); //BorderWavefrom
    Epaper_Write_Data(0x01);
    osDelay(100);   //is necessary
}

static void EPD_SetRAMValue_BaseMap(const uint8_t *datas)
{
	uint32_t i;   
	const uint8_t *datas_flag; 
	uint32_t x_end, y_start1, y_start2, y_end1, y_end2;
	uint32_t x_start = 0, y_start = 200;
	uint32_t PART_COLUMN = 200, PART_LINE = 200;  
	datas_flag = datas;
	//FULL update
    EPD_HW_Init();
    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i=0;i<ALLSCREEN_GRAGHBYTES;i++){               
        Epaper_Write_Data(*datas);
        datas++;
    } 
	EPD_Update();	 
	 
	//PART update
    EPD_Part_Init();
    datas = datas_flag;
    x_start = x_start/8;
    x_end = (x_start + PART_LINE)/8-1; 
	
	y_start1 = 0;
	y_start2 = y_start - 1;
	if(y_start >= 256){
		y_start1 = y_start2/256;
		y_start2 = y_start2%256;
	}
	y_end1 = 0;
	y_end2 = 0;
	if(y_end2 >= 256){
		y_end1 = y_end2/256;
		y_end2 = y_end2%256;		
	}		

	Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
	Epaper_Write_Data(x_start);    // RAM x address start at 00h;
	Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
	Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
	Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
	Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
	Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
	Epaper_Write_Data(y_end1);    // ????=0	

	Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
	Epaper_Write_Data(x_start); 
	Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
	Epaper_Write_Data(y_start2);
	Epaper_Write_Data(y_start1);
	
    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i=0;i<PART_COLUMN*PART_LINE/8;i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    } 
    EPD_Part_Update();
	 
	datas=datas_flag;
	Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
	Epaper_Write_Data(x_start);    // RAM x address start at 00h;
	Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
	Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
	Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
	Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
	Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
	Epaper_Write_Data(y_end1);    // ????=0	


	Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
	Epaper_Write_Data(x_start); 
	Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
	Epaper_Write_Data(y_start2);
	Epaper_Write_Data(y_start1);
	
	
    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0;i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    } 	 
}

static void EPD_Part_off(void)
{ 	
    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0x03);   
    Epaper_Write_Command(0x20); 

    Epaper_Write_Command(0x10); //enter deep sleep
    Epaper_Write_Data(0x01); 
    osDelay(500);
}

static void EPD_WhiteScreen_BW(void)
{
    uint32_t i,k;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<100;k++){
        for(i=0;i<25;i++){
            Epaper_Write_Data(0x00);
        }
    }
    for(k=0;k<100;k++){
        for(i=0;i<25;i++){
            Epaper_Write_Data(0xff);
        }
    }
    EPD_Update();	
}

static void EPD_WhiteScreen_Black(void)

{
    uint32_t i,k;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<200;k++){
        for(i=0;i<25;i++){
            Epaper_Write_Data(0x00);
        }
    }
    EPD_Update();
}

static void EPD_WhiteScreen_White(void)
{
    uint32_t i,k;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k = 0; k < 200; k++){
        for(i = 0; i < 25; i++){
            Epaper_Write_Data(0xff);
        }
    }
    EPD_Update();
}

static void EPD_Dis_Part_myself(uint32_t x_startA, uint32_t y_startA,const uint8_t * datasA,
                                uint32_t x_startB, uint32_t y_startB,const uint8_t * datasB,
                                uint32_t x_startC, uint32_t y_startC,const uint8_t * datasC,
                                uint32_t x_startD, uint32_t y_startD,const uint8_t * datasD,
                                uint32_t x_startE, uint32_t y_startE,const uint8_t * datasE,
                                uint32_t PART_COLUMN, uint32_t PART_LINE)
{
    uint32_t i;  
    uint32_t x_end,y_start1,y_start2,y_end1,y_end2;

    x_startA = x_startA/8;
    x_end = x_startA + PART_LINE/8-1; 

    y_start1 = 0;
    y_start2 = y_startA - 1;
    if(y_startA >= 256){
        y_start1 = y_start2/256;
        y_start2 = y_start2%256;
    }
    y_end1 = 0;
    y_end2 = y_startA + PART_COLUMN - 1;
    if(y_end2 >= 256){
        y_end1 = y_end2/256;
        y_end2 = y_end2%256;
    }

    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_startA);    // RAM x address start at 00h;
    Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);    // ????=0	

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(x_startA); 
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);

    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datasA);
        datasA++;
    } 

    x_startB = x_startB/8;//ת��Ϊ�ֽ�
    x_end = x_startB + PART_LINE/8-1; 

    y_start1 = 0;
    y_start2 = y_startB-1;
    if(y_startB >= 256){
        y_start1 = y_start2/256;
        y_start2 = y_start2%256;
    }
    y_end1 = 0;
    y_end2 = y_startB+PART_COLUMN-1;
    if(y_end2 >= 256){
        y_end1 = y_end2/256;
        y_end2 = y_end2%256;
    }		

    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_startB);    // RAM x address start at 00h;
    Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);    // ????=0	

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(x_startB); 
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);

    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datasB);
        datasB++;
    } 	 

    x_startC = x_startC/8;
    x_end = x_startC + PART_LINE/8-1; 

    y_start1 = 0;
    y_start2 = y_startC - 1;
    if(y_startC >= 256){
        y_start1 = y_start2/256;
        y_start2 = y_start2%256;
    }
    y_end1 = 0;
    y_end2 = y_startC + PART_COLUMN - 1;
    if(y_end2 >= 256){
        y_end1 = y_end2/256;
        y_end2 = y_end2%256;
    }		

    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_startC);    // RAM x address start at 00h;
    Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);    // ????=0	

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(x_startC); 
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);

    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
    {                         
        Epaper_Write_Data(*datasC);
        datasC++;
    } 	 	 

    x_startD = x_startD/8;//ת��Ϊ�ֽ�
    x_end = x_startD + PART_LINE/8-1; 

    y_start1 = 0;
    y_start2 = y_startD-1;
    if(y_startD >= 256){
        y_start1 = y_start2/256;
        y_start2 = y_start2%256;
    }
    y_end1 = 0;
    y_end2 = y_startD+PART_COLUMN-1;
    if(y_end2 >= 256){
        y_end1 = y_end2/256;
        y_end2 = y_end2%256;		
    }		

    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_startD);    // RAM x address start at 00h;
    Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);    // ????=0	

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(x_startD); 
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);

    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datasD);
        datasD++;
    } 

    x_startE = x_startE/8;
    x_end = x_startE+PART_LINE/8-1; 

    y_start1 = 0;
    y_start2 = y_startE-1;
    if(y_startE >= 256){
        y_start1 = y_start2/256;
        y_start2 = y_start2%256;
    }
    y_end1 = 0;
    y_end2 = y_startE+PART_COLUMN-1;
    if(y_end2 >= 256){
        y_end1 = y_end2/256;
        y_end2 = y_end2%256;
    }		

    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_startE);    // RAM x address start at 00h;
    Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);    // ????=0	

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(x_startE); 
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);

    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datasE);
        datasE++;
    } 	  
    EPD_Part_Update();
}
                                
static void EPD_Dis_Part(uint32_t x_start, uint32_t y_start,const uint8_t * datas,
                    uint32_t PART_COLUMN, uint32_t PART_LINE)
{
    const uint8_t *datas_flag; 
    uint32_t i;  
    uint32_t x_end,y_start1,y_start2,y_end1,y_end2;
    
    datas_flag=datas;//��¼�����׵�ַ
    x_start=x_start/8;//ת��Ϊ�ֽ�
    x_end=x_start+PART_LINE/8-1; 
    
    y_start1=0;
    y_start2=y_start-1;
    if(y_start>=256)
    {
        y_start1=y_start2/256;
        y_start2=y_start2%256;
    }
    y_end1=0;
    y_end2=y_start+PART_COLUMN-1;
    if(y_end2>=256)
    {
        y_end1=y_end2/256;
        y_end2=y_end2%256;      
    }       
    
    Epaper_Write_Command(0x44);     // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_start);     // RAM x address start at 00h;
    Epaper_Write_Data(x_end);       // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);     // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);      // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);      // ????=0 


    Epaper_Write_Command(0x4E);     // set RAM x address count to 0;
    Epaper_Write_Data(x_start); 
    Epaper_Write_Command(0x4F);     // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);
    
    
    Epaper_Write_Command(0x24);     //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    } 
    EPD_Part_Update();

    datas=datas_flag;
    Epaper_Write_Command(0x44);     // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_start);     // RAM x address start at 00h;
    Epaper_Write_Data(x_end);       // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);     // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);      // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);      // ????=0 

    Epaper_Write_Command(0x4E);     // set RAM x address count to 0;
    Epaper_Write_Data(x_start); 
    Epaper_Write_Command(0x4F);     // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);
    
    Epaper_Write_Command(0x24);     //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    } 
}

int EPD_Task(void)
{
    uint8_t fen_L, fen_H, miao_L, miao_H;

    EPD_HW_Init();
    EPD_WhiteScreen_ALL(gImage_1);
    osDelay(4000);

    EPD_SetRAMValue_BaseMap(gImage_logo);
    EPD_Part_Init();
    EPD_Dis_Part(0,32,gImage_num1,32,32);
    EPD_Dis_Part(0,32,gImage_num2,32,32);
    EPD_Dis_Part(0,32,gImage_num3,32,32);
    EPD_Dis_Part(0,32,gImage_num4,32,32);
    EPD_Dis_Part(0,32,gImage_num5,32,32);
    EPD_Dis_Part(0,32,gImage_num6,32,32);
    EPD_Dis_Part(0,32,gImage_num7,32,32);
    EPD_Dis_Part(0,32,gImage_num8,32,32);
    EPD_Dis_Part(0,32,gImage_num9,32,32);
    osDelay(100);  
#if 1
    EPD_SetRAMValue_BaseMap(gImage_basemap);      
#else
    EPD_HW_Init();
    EPD_WhiteScreen_Black();
    osDelay(4000);
    EPD_HW_Init();
#endif
    EPD_Part_Init();

    while(1){
        for(fen_H=0;fen_H<6;fen_H++){
            for(fen_L=0;fen_L<10;fen_L++){
#if 0
                EPD_Dis_Part(64,154,B_Num[fen_L],32,64);
                EPD_Dis_Part(64,186,B_Num[fen_H],32,64);
                for(miao_H=0;miao_H<6;miao_H++){
                    for(miao_L=0;miao_L<10;miao_L++){
                        EPD_Dis_Part(64,112,gImage_numdot,32,64);
                        EPD_Dis_Part(64,40,B_Num[miao_L],32,64);
                        EPD_Dis_Part(64,72,B_Num[miao_H],32,64);
                        EPD_Dis_Part(64,112,gImage_white,32,64);
#else
                    for(miao_H=0;miao_H<6;miao_H++){
                        for(miao_L=0;miao_L<10;miao_L++){
                        EPD_Dis_Part_myself(64,40,B_Num[miao_L],
                                            64,72,B_Num[miao_H],
                                            64,112,gImage_numdot,
                                            64,154,B_Num[fen_L],
                                            64,186,B_Num[fen_H],32,64);
                        EPD_Dis_Part_myself(64,40,B_Num[miao_L],
                                            64,72,B_Num[miao_H],
                                            64,112,gImage_white,
                                            64,154,B_Num[fen_L],
                                            64,186,B_Num[fen_H],32,64);
#endif
#if 0
                        if((fen_L==0)&&(miao_H==1)&&(miao_L==8))
                        goto Clear;
#endif
                    }
                }
            }
        }
    }  
#if 0
    osDelay(500);
Clear:
    EPD_HW_Init();
    EPD_WhiteScreen_White();
    EPD_DeepSleep();
    while(1);
#endif
}

/* End of this file. */
