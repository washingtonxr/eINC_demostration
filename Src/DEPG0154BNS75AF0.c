#include "DEPG0154BNS75AF0.h"
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
static void EPD_HW_Init(void);
static void EPD_UpdateAndClose(void);
static void EPD_UpdateAndClose(void);
static void EPD_SetRAMValue_15(uint8_t * datas, uint32_t num, uint8_t mode);

static void Epaper_READBUSY(void)
{ 
    while(1){	 //=1 BUSY
        if(HAL_GPIO_ReadPin(GPIOE, eINC_BUSY_Pin) == GPIO_PIN_RESET) 
            break;
        osDelay(10);
    }  
}

static void Epaper_Spi_WriteByte(uint8_t TxData)
{				   			 
    uint8_t ret;    
    ret = HAL_SPI_Transmit_IT(&hspi1, &TxData, 1);
    if(HAL_OK != ret){
        Debug_printf("Error: HAL_SPI_Transmit_IT write failed.\n");
    }
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

static void EPD_HW_Init(void)
{
    /* Hardware reset. */
    HAL_GPIO_WritePin(GPIOE, eINC_RES_Pin, GPIO_PIN_SET);
    /* Expect 200us delay, use 1ms temporary. */
    osDelay(1);
    HAL_GPIO_WritePin(GPIOE, eINC_RES_Pin, GPIO_PIN_RESET);
    /* Expect 200us delay, use 1ms temporary. */
    osDelay(1);
    HAL_GPIO_WritePin(GPIOE, eINC_RES_Pin, GPIO_PIN_SET);
    /* Expect 200us delay, use 1ms temporary. */
    osDelay(1);    

    /* Wait for busy low. */
    Epaper_READBUSY();
    
    /* Soft reset. */
    Epaper_Write_Command(0x12);
    
    /* Wait for busy low. */
    Epaper_READBUSY();
    
    /* Set analog block control. */
    Epaper_Write_Command(0x74);
    Epaper_Write_Data(0x54);
    
    /* Set digital block control. */
    Epaper_Write_Command(0x74);       
    Epaper_Write_Data(0x54);
    
    /* Set analog block control. */
    Epaper_Write_Command(0x7E);
    Epaper_Write_Data(0x3B);

    /* Driver output control. */
    Epaper_Write_Command(0x01);
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    
    /* Data entry mode setting. */
    Epaper_Write_Command(0x11);
    Epaper_Write_Data(0x01);  

    /* Set RAM X-address Start/End position. */
    Epaper_Write_Command(0x44);     // set RAM x address start/end
    Epaper_Write_Data(0x00);        // RAM x address start at 00h;
    Epaper_Write_Data(0x12);        // RAM x address end at 0fh(15+1)*8->128 

    /* Set RAM Y-address Start/End position. */
    Epaper_Write_Command(0x45);     // set RAM y address start/end
    Epaper_Write_Data(0x97);        // RAM y address START at 00h;
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);

    /* Border waveform control. */
    Epaper_Write_Command(0x3C);
    Epaper_Write_Data(0x01);
    
    /* Temperature sensor control. */
    Epaper_Write_Command(0x18);     // write VCOM voltage
    //HalLcd_HW_Write(0x48);        // use the external temperature sensor
    Epaper_Write_Data(0x80);        // use the internal temperature sensor

    /* Display update control2. */
    Epaper_Write_Command(0x22);     // data enter mode
    Epaper_Write_Data(0xB1);

    /* Master activation. */
    Epaper_Write_Command(0x20);     // data enter mode

    /* Wait for busy low. */
    Epaper_READBUSY();
}

static void EPD_UpdateAndClose(void)
{   
    //HalLcd_HW_Control(0x21); 
    //HalLcd_HW_Write(0x40);   
    /* Display update control2. */
    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0xC7);

    /* Master activation. */
    Epaper_Write_Command(0x20);
    
    /* Wait for busy low. */
    Epaper_READBUSY();  
    //HalLcd_HW_Control(0x10);      //enter deep sleep
    //HalLcd_HW_Write(0x01); 
}

void EPD_select_LUT(unsigned char * wave_data)
{        
    unsigned char count;
    Epaper_Write_Command(0x32);
    for(count=0;count<70;count++)
        Epaper_Write_Data(wave_data[count]);
}

void HalLcd_SetRAMValue(unsigned char * datas,unsigned int num,unsigned char mode)
{
    unsigned int i;   
    unsigned char tempOriginal;      
    unsigned int tempcol=0;
    unsigned int templine=0;   
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    if(mode == MONO_MODE)
        Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    if(mode == RED_MODE_21)
        Epaper_Write_Command(0x26);   //write RAM for RED (1) / black white(0)

    for(i=0;i<num;i++){         
        tempOriginal=*(datas+templine*MAX_COLUMN_BYTES+tempcol);
        templine++;
        if(templine>=MAX_LINE_BYTES){
            tempcol++;
            templine=0;
        }     
        Epaper_Write_Data(tempOriginal); 
    }
}

static void EPD_SetRAMValue_15(uint8_t * datas, uint32_t num, uint8_t mode)
{
    uint32_t i;
    uint8_t tempOriginal;
    uint32_t tempcol=0;
    uint32_t templine=0;

    Epaper_Write_Command(0x24);     // write RAM for black(0)/white (1)

    for(i = 0; i < num; i++){         
        tempOriginal = *(datas + templine*MAX_COLUMN_BYTES + tempcol);
        templine++;
        if(templine >= MAX_LINE_BYTES){
            tempcol++;
            templine = 0;
        }     
        Epaper_Write_Data(~tempOriginal); 
    }
}

void EPD_All_White()
{
    unsigned int i,k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0xff);
        }
    }
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();

    Epaper_Write_Command(0x26);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0x00);
        }
    }
}

void EPD_All_Black()
{
    unsigned int i,k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0x00);
        }
    }
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();

    Epaper_Write_Command(0x26);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0x00);
        }
    }
}

void EPD_all_While_PART()
{
    unsigned int i,k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0xff);
        }
    }
}

void EPD_all_Black_PART()
{
    unsigned int i,k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0x00);
        }
    }
}

void EPD_SetRAM_BW(void)
{
    Epaper_Write_Command(0x11);      //    
    Epaper_Write_Data(0x01);//

    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(0x00);    // RAM x address start at 00h;
    Epaper_Write_Data(0x31);    // RAM x address end at 0fh(15+1)*8->128 

    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(0x2B);    // RAM y address start at 0127h;
    Epaper_Write_Data(0x01);    // RAM y address start at 0127h;
    Epaper_Write_Data(0x00);    // RAM y address end at 00h;
    Epaper_Write_Data(0x00);    // ��λ��ַ=0	   

    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x2B);
    Epaper_Write_Data(0x01);
}

void EPD_BW_first()
{
    unsigned int i,k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x97);
    Epaper_Write_Data(0x00);
    Epaper_READBUSY();
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<152;k++){
        for(i=0;i<19;i++){
            Epaper_Write_Data(0xff);
        }
    }
}

void EPD_Partial(void)
{
    Epaper_READBUSY();
    osDelay(20);
    EPD_select_LUT((unsigned char *)LUT_DATA_part);
    Epaper_Write_Command(0x37);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x40);
    Epaper_Write_Data(0x00);
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x22);
    Epaper_Write_Data(0xC0);
    Epaper_Write_Command(0x20);
    Epaper_READBUSY();
    osDelay(20);
}

void EPD_UpdateBegin_part(unsigned char x_start,unsigned char x_end,unsigned char y_start1,unsigned char y_start2,unsigned char y_end1,unsigned char y_end2)
{
    Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
    Epaper_Write_Data(x_start);    // RAM x address start at 00h;
    Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
    Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
    Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
    Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
    Epaper_Write_Data(y_end1);    // ��λ��ַ=0	


    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(x_start); 
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(y_start2);
    Epaper_Write_Data(y_start1);
}

void EPD_SetRAMValue_part(unsigned char * datas,unsigned int num)
{
    unsigned int i;   
    unsigned char tempOriginal;      
    unsigned int tempcol=0;
    unsigned int templine=0;   

    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i=0;i<num;i++){           
        tempOriginal=*(datas+templine*PART_COLUMN_BYTES+tempcol);
        templine++;
        if(templine>=PART_LINE_BYTES){
            tempcol++;
            templine=0;
        }     
        Epaper_Write_Data(tempOriginal);
    } 
}

void EPD_Partial_off(void)
{
    Epaper_READBUSY();
    Epaper_Write_Command(0x22);
    Epaper_Write_Data(0x03);
    Epaper_Write_Command(0x20);
    Epaper_READBUSY();
    // HalLcd_HW_Control(0x10); //enter deep sleep
    // HalLcd_HW_Write(0x01);
    osDelay(100);
}

void EPD_Partial_Update(void)
{
    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0x0c);   
    Epaper_Write_Command(0x20); 
    Epaper_READBUSY(); 	
    osDelay(20);
}

void EPD_Update()
{
    Epaper_Write_Command(0x22);
    Epaper_Write_Data(0xC7);    // (Enable Clock Signal, Enable CP) (Display update,Disable CP,Disable Clock Signal) ��������˳�� 
    Epaper_Write_Command(0x20);
    Epaper_READBUSY();
    //osDelay(100);
}

void EPD_Sleep()
{
    Epaper_Write_Command(0x10);
    Epaper_Write_Data(0x01);
    osDelay(100);
    osDelay(100);
}


void EPD_UpdateBegin(void)
{
    Epaper_READBUSY();  
    Epaper_Write_Command(0x44);       // set RAM x address start/end
    Epaper_Write_Data(0x00);    // RAM x address start at 00h;
    Epaper_Write_Data(0x0f);    // RAM x address end at 0fh(15+1)*8->128 

    Epaper_Write_Command(0x45);       // set RAM y address start/end
    Epaper_Write_Data(0x00);    // RAM y address START at 00h;
    Epaper_Write_Data(0xf9);    // Y end f9h+1=250

    Epaper_Write_Command(0x4E);    //SET X address counter
    Epaper_Write_Data(0x00); 

    Epaper_Write_Command(0x4F);    //set y  address  counter
    Epaper_Write_Data(0x00);
}

void EPD_SetAll_white(void)
{
    unsigned int i, k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x27);
    Epaper_Write_Data(0x01);
    osDelay(500);//500mS  

    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<296;k++){
        for(i=0;i<16;i++){
            Epaper_Write_Data(0xff);
        }
    }
}

void EPD_Black(void)
{
    unsigned int i,k;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<106;k++){
        for(i=0;i<13;i++){
            Epaper_Write_Data(0x00);
        }	
    }
    for(k=0;k<106;k++){
        for(i=0;i<13;i++){
            Epaper_Write_Data(0xff);
        }	
    }
}

void EPD_WhiteGrid(void)
{
    unsigned int i,k;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(k=0;k<296;k++){
        if(k%16<8){
            for(i=0;i<16;i++){
                if(i%2==0)
                    Epaper_Write_Data(0xff);
                else
                    Epaper_Write_Data(0x00);                 
            }
        }
        if(k%16>=8){
            for(i=0;i<16;i++){
                if(i%2==0)
                    Epaper_Write_Data(0x00);
                else
                    Epaper_Write_Data(0xff);                 
            }
        }
    }
}

void EPD_WhiteGrid_reload(void)
{
    unsigned int i,k;
    Epaper_Write_Command(0x26);   //write RAM for black(0)/white (1)
    for(k=0;k<296;k++){
        if(k%16<8){
            for(i=0;i<16;i++){
                if(i%2==0)
                    Epaper_Write_Data(0xff);
                else
                    Epaper_Write_Data(0x00);                 
            }
        }
        if(k%16>=8){
            for(i=0;i<16;i++){
                if(i%2==0)
                    Epaper_Write_Data(0x00);
                else
                    Epaper_Write_Data(0xff);                 
            }
        }
    }
}

void EPD_SetAll_BW(void)
{
    unsigned int i,k;
    Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
    Epaper_Write_Data(0x00);
    Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
    Epaper_Write_Data(0x27);
    Epaper_Write_Data(0x01);
    osDelay(500);//500mS  

    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
	for(k=0;k<96;k++){
    	for(i=0;i<16;i++){
    		Epaper_Write_Data(0xff);
        }
    }
    for(k=0;k<96;k++){
    	for(i=0;i<16;i++){
    		Epaper_Write_Data(0xff);
        }
    }
    for(k=0;k<104;k++){
        for(i=0;i<16;i++){
            Epaper_Write_Data(0x00);
        }
    }
}

int EPD_Task(void)
{
    uint8_t i;
    Debug_printf("Info: EPD_Task.\n");
    EPD_HW_Init();
    while(1){
        for(i = 0; i < 36; i++){
            EPD_SetRAMValue_15((uint8_t *)gImage_clock[i],ALLSCREEN_GRAGHBYTES,MONO_MODE);
            EPD_Update();
        }
    }
}


/* End of this file. */
