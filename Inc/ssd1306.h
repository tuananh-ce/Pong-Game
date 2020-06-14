/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-ssd1306
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include "string.h"
#include "ssd1306_fonts.h"

/* vvv I2C config vvv */

#ifndef SSD1306_I2C_PORT
#define SSD1306_I2C_PORT		hi2c1
#endif

#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR        (0x3C << 1)
#endif

/* ^^^ I2C config ^^^ */

extern I2C_HandleTypeDef SSD1306_I2C_PORT;

// SSD1306 OLED height in pixels
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT          32
#endif

// SSD1306 width in pixels
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH           128
#endif

#define swap(a, b) { int16_t t = a; a = b; b = t; }
// Enumeration for screen colors
typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} SSD1306_COLOR;

// Struct to store transformations
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} SSD1306_t;



// Procedure definitions
void LCD_Init(void);
void LCD_Fill(SSD1306_COLOR color);
void LCD_UpdateScreen(void);
void LCD_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
char LCD_WriteChar(char ch, FontDef Font, SSD1306_COLOR color);
char LCD_WriteString(char* str, FontDef Font, SSD1306_COLOR color);
void LCD_SetCursor(uint8_t x, uint8_t y);

// Shape drawing
void LCD_DrawLine(uint8_t pos_X0, uint8_t pos_Y0, uint8_t pos_X1, uint8_t pos_Y1, SSD1306_COLOR Color);
void LCD_DrawVerticalLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Height,SSD1306_COLOR Color);
void LCD_DrawHorizontalLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Width,SSD1306_COLOR Color);
void LCD_DrawRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, SSD1306_COLOR Color);
void LCD_FillRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, SSD1306_COLOR Color);
void LCD_DrawCircle(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, SSD1306_COLOR Color);
void LCD_DrawCircleHelper(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, uint8_t Cornername, SSD1306_COLOR Color);
void LCD_FillCircle(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, SSD1306_COLOR Color);
void LCD_FillCircleHelper(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, uint8_t Cornername, int16_t Delta, SSD1306_COLOR Color);
void LCD_DrawTriangle(uint8_t pos_X0, uint8_t pos_Y0, uint8_t pos_X1, uint8_t pos_Y1, uint8_t pos_X2, uint8_t pos_Y2, SSD1306_COLOR Color);
void LCD_FillTriangle(uint8_t pos_X0, uint8_t pos_Y0, uint8_t pos_X1, uint8_t pos_Y1, uint8_t pos_X2, uint8_t pos_Y2, SSD1306_COLOR Color);
void LCD_DrawRoundRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, uint8_t Radius, SSD1306_COLOR Color);
void LCD_FillRoundRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, uint8_t Radius, SSD1306_COLOR Color);

void LCD_DrawVerticalDotLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Height);
void LCD_DrawHorizontalDotLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Width);
void LCD_DrawDotRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height);
void LCD_FillDotRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height);

// Low-level procedures
void LCD_Reset(void);
void LCD_WriteCommand(uint8_t byte);
void LCD_WriteData(uint8_t* buffer, size_t buff_size);

#endif //!__SSD1306_H__
