#include "DEPG0154BNS75AF0.h"
#include "gpio.h"
#include "freertos.h"
#include "cmsis_os.h"
#include "main.h"
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
static void EPD_select_LUT(uint8_t * wave_data, uint32_t len);
static void EPD_SetRAMValue(unsigned char * datas,unsigned int num,unsigned char mode);
static void EPD_SetRAMValue_15(uint8_t * datas, uint32_t num, uint8_t mode);
static void EPD_All_White(void);
static void EPD_All_Black(void);
static void EPD_all_While_PART(void);
static void EPD_all_Black_PART(void);
static void EPD_SetRAM_BW(void);
static void EPD_BW_first(void);
static void EPD_Partial(void);
static void EPD_UpdateBegin_part(unsigned char x_start,unsigned char x_end,unsigned char y_start1,unsigned char y_start2,unsigned char y_end1,unsigned char y_end2);
static void EPD_SetRAMValue_part(uint8_t * datas, uint32_t num);
static void EPD_Partial_off(void);
static void EPD_Partial_Update(void);
static void EPD_Update(void);
static void EPD_Sleep(void);
static void EPD_UpdateBegin(void);
static void EPD_SetAll_white(void);
static void EPD_Black(void);
static void EPD_WhiteGrid(void);
static void EPD_WhiteGrid_reload(void);
static void EPD_SetAll_BW(void);
static void EPD_SetRAMValue_BaseMap(const uint8_t *datas);
static void EPD_Dis_Part(uint32_t x_start, uint32_t y_start,
                            const uint8_t * datas,
                            uint32_t PART_COLUMN, uint32_t PART_LINE);
static void EPD_Part_Init(void);
void EPD_SetRAMValue_part_image2(unsigned char * datas,unsigned int num);
static void EPD_SetRAMValue_part_image2(unsigned char * datas,unsigned int num);
static void EPD_UpdateAndClose_part(void);

const unsigned char LUT_DATA[]=
{						
0xA0,	0x84,	0x50,	0x0,	0x0,	0x0,	0x0,
0x50,	0x84,	0xA0,	0x0,	0x0,	0x0,	0x0,
0xA0,	0x84,	0x50,	0x0,	0x0,	0x0,	0x0,
0x50,	0x84,	0xA0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x2,	0x1,	0x0,	0x0,	0x1,		
0x5,	0x0,	0x5,	0x0,	0x1,		
0x2,	0x1,	0x0,	0x0,	0x1,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x17,	0x50,	0x0,	0x3E,			
0x11,	0x0D};					
/*const*/ unsigned char LUT_DATA_part[]=
{
0X00,0x40,0x00,0x00,0x00,0x00,0x00,	// L0 0-6 ABCD		
0X80,0x80,0x00,0x00,0x00,0x00,0x00,	// L1 0-6 ABCD		
0X40,0x40,0x00,0x00,0x00,0x00,0x00,	// L2 0-6 ABCD		
0X00,0x80,0x00,0x00,0x00,0x00,0x00,	// L3 0-6 ABCD		
0X00,0x00,0x00,0x00,0x00,0x00,0x00,	// VCOM 0-6 ABCD		
			
0x0A,0x00,0x00,0x00,0x02,//0A 0B 0C 0D R			
0x02,0x00,0x00,0x00,0x00,//1A 0B 0C 0D R			
0x00,0x00,0x00,0x00,0x00,//2A 0B 0C 0D R			
0x00,0x00,0x00,0x00,0x00,//3A 0B 0C 0D R			
0x00,0x00,0x00,0x00,0x00,//4A 0B 0C 0D R			
0x00,0x00,0x00,0x00,0x00,//5A 0B 0C 0D R			
0x00,0x00,0x00,0x00,0x00,//6A 0B 0C 0D R 
0x17,	0x41,	0x0,	0x32,			
0x11,	0x0D};			

/* const*/ unsigned char WF_FULL_DATA[76] =
 {
0xA0,	0x90,	0x50,	0x0,	0x0,	0x0,	0x0,
0x50,	0x90,	0xA0,	0x0,	0x0,	0x0,	0x0,
0xA0,	0x90,	0x50,	0x0,	0x0,	0x0,	0x0,
0x50,	0x90,	0xA0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0xF,	0xF,	0x0,	0x0,	0x0,		
0xF,	0xF,	0x0,	0x0,	0x3,		
0xF,	0xF,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x0,	0x0,	0x0,	0x0,	0x0,		
0x17,	0x41,	0x0,	0x32,			
0x11,	0x0D};
			
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
#if 0
    Epaper_Write_Command(0x2c);       // vcom write    
    Epaper_Write_Data(0x6A); 

    EPD_select_LUT(WF_FULL_DATA, sizeof(WF_FULL_DATA));

    Epaper_Write_Command(0x4E);     
    Epaper_Write_Data(0x00);

    Epaper_Write_Command(0x4F);       
    Epaper_Write_Data(0x97); //0x2b
    Epaper_Write_Data(0x00); //0x01

    Epaper_READBUSY();
#else
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
#endif
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

static void EPD_select_LUT(uint8_t * wave_data, uint32_t len)
{        
    unsigned char count;
    Epaper_Write_Command(0x32);
    for(count=0; count < len; count++)
        Epaper_Write_Data(wave_data[count]);
}

static void EPD_SetRAMValue(unsigned char * datas,unsigned int num,unsigned char mode)
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

static void EPD_All_White(void)
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

static void EPD_All_Black(void)
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

static void EPD_all_While_PART(void)
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

static void EPD_all_Black_PART(void)
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

static void EPD_SetRAM_BW(void)
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

static void EPD_BW_first(void)
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

static void EPD_Partial(void)
{
    Epaper_READBUSY();
    osDelay(20);
    EPD_select_LUT((unsigned char *)LUT_DATA_part, sizeof(LUT_DATA_part));
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

static void EPD_UpdateBegin_part(unsigned char x_start,unsigned char x_end,unsigned char y_start1,unsigned char y_start2,unsigned char y_end1,unsigned char y_end2)
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

static void EPD_SetRAMValue_part(uint8_t * datas, uint32_t num)
{
    uint32_t i;   
    uint8_t tempOriginal;      
    uint32_t tempcol=0;
    uint32_t templine=0;   

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

static void EPD_Partial_off(void)
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

static void EPD_Partial_Update(void)
{
    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0x0c);   
    Epaper_Write_Command(0x20); 
    Epaper_READBUSY(); 	
    //osDelay(20);
}

static void EPD_Update(void)
{
    Epaper_Write_Command(0x22);
    Epaper_Write_Data(0xC7);    // (Enable Clock Signal, Enable CP) (Display update,Disable CP,Disable Clock Signal) ��������˳�� 
    Epaper_Write_Command(0x20);
    Epaper_READBUSY();
    //osDelay(100);
}

static void EPD_Sleep(void)
{
    Epaper_Write_Command(0x10);
    Epaper_Write_Data(0x01);
    osDelay(100);
    osDelay(100);
}

static void EPD_UpdateBegin(void)
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

static void EPD_SetAll_white(void)
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

static void EPD_Black(void)
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

static void EPD_WhiteGrid(void)
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

static void EPD_WhiteGrid_reload(void)
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

static void EPD_SetAll_BW(void)
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
/* Test begin.. */
static void EPD_SetRAMValue_BaseMap(const uint8_t *datas)
{
	uint32_t i;   
	const uint8_t *datas_flag; 
	uint32_t x_end, y_start1, y_start2, y_end1, y_end2;
	uint32_t x_start = 0, y_start = 152;
	uint32_t PART_COLUMN = 152, PART_LINE = 152;  
	datas_flag = datas;
	//FULL update
    EPD_HW_Init();
    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0; i < ALLSCREEN_GRAGHBYTES; i++){               
        Epaper_Write_Data(*datas);
        datas++;
    }

    EPD_Update();	 
	//EPD_Partial();
	//PART update
    //EPD_Part_Init();
    EPD_Partial();
    
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
    EPD_UpdateBegin_part((uint8_t)x_start, x_end, y_start1, y_start1, y_end1, y_end2);
#if 0
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
#endif
    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i=0;i<PART_COLUMN*PART_LINE/8;i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    }
    //EPD_Part_Update();
	EPD_Partial_Update();

	datas=datas_flag;
    EPD_UpdateBegin_part((uint8_t)x_start, x_end, y_start1, y_start1, y_end1, y_end2);
#if 0
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
#endif
    Epaper_Write_Command(0x24);   //Write Black and White image to RAM
    for(i = 0;i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    }
}

static void EPD_Dis_Part(uint32_t x_start, uint32_t y_start,
                            const uint8_t * datas,
                            uint32_t PART_COLUMN, uint32_t PART_LINE)
{
    const uint8_t *datas_flag; 
    uint32_t i;  
    uint32_t x_end,y_start1,y_start2,y_end1,y_end2;
    
    datas_flag = datas;
    x_start = x_start / 8;
    x_end = x_start + PART_LINE / 8 - 1; 
    
    y_start1 = 0;
    y_start2 = y_start - 1;
    if(y_start >= 256){
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    y_end1 = 0;
    y_end2 = y_start + PART_COLUMN-1;
    if(y_end2 >= 256){
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;      
    }
    EPD_UpdateBegin_part((uint8_t)x_start, x_end, y_start1, y_start1, y_end1, y_end2);
#if 1
    Epaper_Write_Command(0x24);     //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    }
#else
    uint8_t tempOriginal;
    uint32_t tempcol=0;
    uint32_t templine=0;

    Epaper_Write_Command(0x24);     // write RAM for black(0)/white (1)
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){         
        tempOriginal = *(datas + templine*PART_COLUMN/8 + tempcol);
        templine++;
        if(templine >= PART_LINE){
            tempcol++;
            templine = 0;
        }     
        Epaper_Write_Data(~tempOriginal); 
    }
#endif
    EPD_Partial_Update();

    datas=datas_flag;
    EPD_UpdateBegin_part((uint8_t)x_start,x_end,y_start1,y_start1,y_end1,y_end2);
#if 1
    Epaper_Write_Command(0x24);     //Write Black and White image to RAM
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){                         
        Epaper_Write_Data(*datas);
        datas++;
    }
#else
    tempcol=0;
    templine=0;

    Epaper_Write_Command(0x24);     // write RAM for black(0)/white (1)
    for(i = 0; i < PART_COLUMN*PART_LINE/8; i++){         
        tempOriginal = *(datas + templine*PART_COLUMN/8 + tempcol);
        templine++;
        if(templine >= PART_LINE){
            tempcol++;
            templine = 0;
        }     
        Epaper_Write_Data(~tempOriginal); 
    }
#endif
    EPD_Partial_Update();
}

static void EPD_Part_Init(void)
{
#if 0
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
#else
    Epaper_READBUSY();  

    EPD_select_LUT(LUT_DATA_part, sizeof(LUT_DATA_part));

    Epaper_Write_Command(0x37);      
    Epaper_Write_Data(0x00); 
    Epaper_Write_Data(0x00); 
    Epaper_Write_Data(0x00); 
    Epaper_Write_Data(0x00); 
    Epaper_Write_Data(0x40); 
    Epaper_Write_Data(0x00); 
    Epaper_Write_Data(0x00); 

    Epaper_Write_Command(0x3C);       // board 
    Epaper_Write_Data(0x80); 

    Epaper_Write_Command(0x22); 
    Epaper_Write_Data(0xC0);   
    Epaper_Write_Command(0x20); 
    Epaper_READBUSY();  
#endif
}

static void EPD_SetRAMValue_part_image2(unsigned char * datas,unsigned int num)
{
  unsigned int i;   
  unsigned char tempOriginal;      
  unsigned int tempcol=0;
  unsigned int templine=0;   

  Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<num;i++)
   {           
     tempOriginal=*(datas+templine*48+tempcol);
     templine++;
     if(templine>=9)
     {
       tempcol++;
       templine=0;
     }     
     Epaper_Write_Data(tempOriginal);
   } 

}

static void EPD_UpdateAndClose_part(void)
{   
  osDelay(500);//500mS  
  //HalLcd_HW_Control(0x21); 
  //HalLcd_HW_Write(0x40);   
 
  Epaper_Write_Command(0x22); 
  Epaper_Write_Data(0x0f);   
  Epaper_Write_Command(0x20); 
  Epaper_READBUSY();  

  //HalLcd_HW_Control(0x10); //enter deep sleep
  //HalLcd_HW_Write(0x01); 

  osDelay(1000);  
}

int EPD_Task(void)
{
    uint8_t i;
    Debug_printf("Info: EPD_Task.\n");
    while(1){
#if 0
        EPD_HW_Init();
        for(i = 0; i < 1; i++){
            EPD_SetRAMValue((uint8_t *)gImage_clock[i], ALLSCREEN_GRAGHBYTES, MONO_MODE);
            EPD_Update();
        }
         osDelay(1000);
        EPD_SetRAMValue_BaseMap((uint8_t *)gImage_152bb);
        //EPD_Partial();
        EPD_Part_Init();
#if 1
        EPD_Dis_Part(0, 32, gImage_num1, 32, 32);      
        EPD_Dis_Part(0, 32, gImage_num2, 32, 32);
#endif
#else
        EPD_HW_Init();
        EPD_SetRAMValue((unsigned char *)INCH_BW,ALLSCREEN_GRAGHBYTES,MONO_MODE);    
        EPD_UpdateAndClose(); 
        osDelay(1000);
        
        EPD_Part_Init();
        EPD_UpdateBegin_part(0x00,0x8,0x00,0x83,0x00,0x54); //132 84
        EPD_SetRAMValue_part_image2((unsigned char *)XG, 432);   
        EPD_UpdateAndClose_part();
#endif
    }
}

/* End of this file. */
