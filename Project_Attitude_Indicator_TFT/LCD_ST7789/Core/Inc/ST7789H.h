#ifndef __ST7789H2_H
#define __ST7789H2_H

#include <stdio.h>
#include <stdint.h>


#define  ST7789H2_ID    0x85
   

#define  ST7789H2_LCD_PIXEL_WIDTH    ((uint16_t)240)
#define  ST7789H2_LCD_PIXEL_HEIGHT   ((uint16_t)240)


#define ST7789H2_ORIENTATION_PORTRAIT         ((uint32_t)0x00) /* Portrait orientation choice of LCD screen  */
#define ST7789H2_ORIENTATION_LANDSCAPE        ((uint32_t)0x01) /* Landscape orientation choice of LCD screen */
#define ST7789H2_ORIENTATION_LANDSCAPE_ROT180 ((uint32_t)0x02) /* Landscape rotated 180? orientation choice of LCD screen */


#define ST7789H2_LCD_ID             0x04   
#define ST7789H2_SLEEP_IN           0x10
#define ST7789H2_SLEEP_OUT          0x11
#define ST7789H2_PARTIAL_DISPLAY    0x12    
#define ST7789H2_DISPLAY_INVERSION  0x21
#define ST7789H2_DISPLAY_ON         0x29
#define ST7789H2_WRITE_RAM          0x2C
#define ST7789H2_READ_RAM           0x2E
#define ST7789H2_CASET              0x2A
#define ST7789H2_RASET              0x2B
#define ST7789H2_VSCRDEF            0x33 /* Vertical Scroll Definition */
#define ST7789H2_VSCSAD             0x37 /* Vertical Scroll Start Address of RAM */   
#define ST7789H2_TEARING_EFFECT     0x35
#define ST7789H2_NORMAL_DISPLAY     0x36
#define ST7789H2_IDLE_MODE_OFF      0x38
#define ST7789H2_IDLE_MODE_ON       0x39
#define ST7789H2_COLOR_MODE         0x3A
#define ST7789H2_PORCH_CTRL         0xB2
#define ST7789H2_GATE_CTRL          0xB7
#define ST7789H2_VCOM_SET           0xBB
#define ST7789H2_DISPLAY_OFF        0xBD    
#define ST7789H2_LCM_CTRL           0xC0
#define ST7789H2_VDV_VRH_EN         0xC2
#define ST7789H2_VDV_SET            0xC4    
#define ST7789H2_VCOMH_OFFSET_SET   0xC5
#define ST7789H2_FR_CTRL            0xC6
#define ST7789H2_POWER_CTRL         0xD0
#define ST7789H2_PV_GAMMA_CTRL      0xE0
#define ST7789H2_NV_GAMMA_CTRL      0xE1 
  

void     ST7789H2_Init(void);
void     ST7789H2_SetOrientation(uint32_t orientation);    
uint16_t ST7789H2_ReadID(void);
void     ST7789H2_WriteReg(uint8_t Command, uint8_t *Parameters, uint8_t NbParameters);
uint8_t  ST7789H2_ReadReg(uint8_t Command);

void     ST7789H2_DisplayOn(void);
void     ST7789H2_DisplayOff(void);
void     ST7789H2_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     ST7789H2_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode);
uint16_t ST7789H2_ReadPixel(uint16_t Xpos, uint16_t Ypos);

void     ST7789H2_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ST7789H2_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ST7789H2_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     ST7789H2_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint8_t *pdata);

void 		 ST7789H2_DrawRGBHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint8_t *pdata);

void     ST7789H2_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);


uint16_t ST7789H2_GetLcdPixelWidth(void);
uint16_t ST7789H2_GetLcdPixelHeight(void);



/* LCD IO functions */
void     LCD_IO_WriteMultipleData(uint16_t *pData, uint32_t Size);
void     LCD_IO_WriteReg(uint8_t Reg);
void     LCD_IO_WriteData(uint16_t RegValue);
uint16_t LCD_IO_ReadData(void);
void     LCD_IO_Delay(uint32_t delay);




#endif /* __ST7789H2_H */

