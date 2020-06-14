#include "ssd1306.h"

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
// Screen object
static SSD1306_t SSD1306;

void LCD_Reset(void) {
	/* for I2C - do nothing */
}

// Send a byte to the command register
inline void LCD_WriteCommand(uint8_t byte) {
	HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
inline void LCD_WriteData(uint8_t* buffer, size_t buff_size) {
	HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

// Initialize the oled screen
void LCD_Init(void) {
	// Reset OLED
	LCD_Reset();

	// Wait for the screen to boot
	HAL_Delay(100);
    
	// Init OLED
	LCD_WriteCommand(0xAE); //display off

	LCD_WriteCommand(0x20); //Set Memory Addressing Mode   
	LCD_WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid
	LCD_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

#ifdef SSD1306_MIRROR_VERT
	LCD_WriteCommand(0xC0); // Mirror vertically
#else
	LCD_WriteCommand(0xC8); //Set COM Output Scan Direction
#endif

  LCD_WriteCommand(0x00); //---set low column address
  LCD_WriteCommand(0x10); //---set high column address

  LCD_WriteCommand(0x40); //--set start line address - CHECK

  LCD_WriteCommand(0x81); //--set contrast control register - CHECK
  LCD_WriteCommand(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
  LCD_WriteCommand(0xA0); // Mirror horizontally
#else
  LCD_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
  LCD_WriteCommand(0xA7); //--set inverse color
#else
  LCD_WriteCommand(0xA6); //--set normal color
#endif

// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
  // Found in the Luma Python lib for SH1106.
  LCD_WriteCommand(0xFF);
#else
  LCD_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#endif

#if (SSD1306_HEIGHT == 32)
  LCD_WriteCommand(0x1F); //
#elif (SSD1306_HEIGHT == 64)
  LCD_WriteCommand(0x3F); //
#elif (SSD1306_HEIGHT == 128)
  LCD_WriteCommand(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

  LCD_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

  LCD_WriteCommand(0xD3); //-set display offset - CHECK
  LCD_WriteCommand(0x00); //-not offset

  LCD_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
  LCD_WriteCommand(0xF0); //--set divide ratio

  LCD_WriteCommand(0xD9); //--set pre-charge period
  LCD_WriteCommand(0x22); //

  LCD_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
#if (SSD1306_HEIGHT == 32)
  LCD_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
  LCD_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
  LCD_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

  LCD_WriteCommand(0xDB); //--set vcomh
  LCD_WriteCommand(0x20); //0x20,0.77xVcc

  LCD_WriteCommand(0x8D); //--set DC-DC enable
  LCD_WriteCommand(0x14); //
  LCD_WriteCommand(0xAF); //--turn on SSD1306 panel

  // Clear screen
  LCD_Fill(Black);
    
  // Flush buffer to screen
  LCD_UpdateScreen();
    
  // Set default values for screen object
  SSD1306.CurrentX = 0;
  SSD1306.CurrentY = 0;
    
  SSD1306.Initialized = 1;
}

// Fill the whole screen with the given color
inline void LCD_Fill(SSD1306_COLOR color) {
	/* Set memory */
	uint8_t clr = (color == Black) ? 0x00 : 0xFF;
	memset(SSD1306_Buffer,clr,sizeof(SSD1306_Buffer));
}

// Write the screenbuffer with changed to the screen
inline void LCD_UpdateScreen(void) {
	// Write data to each page of RAM. Number of pages
	// depends on the screen height:
  //
  //  * 32px   ==  4 pages
  //  * 64px   ==  8 pages
  //  * 128px  ==  16 pages
  for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
		LCD_WriteCommand(0xB0 + i); // Set the current RAM page address.
    LCD_WriteCommand(0x00);
    LCD_WriteCommand(0x10);
    LCD_WriteData(&SSD1306_Buffer[SSD1306_WIDTH*i],SSD1306_WIDTH);
  }
}

//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
inline void LCD_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
  if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
    // Don't write outside the buffer
    return;
  }
  // Check if pixel should be inverted
  if(SSD1306.Inverted) {
    color = (SSD1306_COLOR)!color;
  }
  // Draw in the right color
  if(color == White) {
    SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y & 7);
  } else { 
    SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
  }
}

// Draw 1 char to the screen buffer
// ch       => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color    => Black or White
inline char LCD_WriteChar(char ch, FontDef Font, SSD1306_COLOR color) {
	uint32_t i, b, j;
  // Check if character is valid
  if (ch < 32 || ch > 126) return 0;
  // Check remaining space on current line
  if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.FontWidth) ||
    SSD1306_HEIGHT < (SSD1306.CurrentY + Font.FontHeight))
  {
    // Not enough space on current line
    return 0;
  }
  // Use the font to write
  for(i = 0; i < Font.FontHeight; i++) {
    b = Font.data[(ch - 32) * Font.FontHeight + i];
    for(j = 0; j < Font.FontWidth; j++) {
			if((b << j) & 0x8000)  {
				LCD_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
			} else {
				LCD_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
			}
    }
  }
  // The current space is now taken
  SSD1306.CurrentX += Font.FontWidth; 
  // Return written char for validation
  return ch;
}

// Write full string to screenbuffer
inline char LCD_WriteString(char* str, FontDef Font, SSD1306_COLOR color) {
	// Write until null-byte
	while (*str) {
		if (LCD_WriteChar(*str, Font, color) != *str) {
			// Char could not be written
			return *str;
		}
		// Next char
		str++;
	}
	// Everything ok
	return *str;
}

// Position the cursor
inline void LCD_SetCursor(uint8_t x, uint8_t y) {
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

//////Shape drawing////////
inline void LCD_DrawLine(uint8_t pos_X0, uint8_t pos_Y0, uint8_t pos_X1, uint8_t pos_Y1, SSD1306_COLOR Color)
{
  uint8_t steep = abs(pos_Y1 - pos_Y0) > abs(pos_X1 - pos_X0);

  if (steep) {
    swap(pos_X0, pos_Y0);
    swap(pos_X1, pos_Y1);
  }

  if (pos_X0 > pos_X1) {
    swap(pos_X0, pos_X1);
    swap(pos_Y0, pos_Y1);
  }

  int16_t dx, dy;
  dx = pos_X1 - pos_X0;
  dy = abs(pos_Y1 - pos_Y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (pos_Y0 < pos_Y1){
		ystep = 1;
  }else{
		ystep = -1;
  }

  for (; pos_X0<=pos_X1; pos_X0++){
    if (steep){
			LCD_DrawPixel(pos_Y0, pos_X0, Color);
    }else{
			LCD_DrawPixel(pos_X0, pos_Y0, Color);
    }
    err -= dy;
    if (err < 0){
			pos_Y0 += ystep;
			err += dx;
    }
  }
}

inline void LCD_DrawVerticalLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Height,SSD1306_COLOR Color)
{
	LCD_DrawLine(pos_X, pos_Y, pos_X, pos_Y + Height - 1, Color);
}

inline void LCD_DrawHorizontalLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Width,SSD1306_COLOR Color)
{
	LCD_DrawLine(pos_X, pos_Y, pos_X + Width - 1, pos_Y, Color);
}

inline void LCD_DrawRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, SSD1306_COLOR Color)
{
	LCD_DrawVerticalLine(pos_X0, pos_Y0, Height, Color);
	LCD_DrawVerticalLine(pos_X0 + Width -1, pos_Y0, Height, Color);
	LCD_DrawHorizontalLine(pos_X0, pos_Y0, Width, Color);
	LCD_DrawHorizontalLine(pos_X0, pos_Y0 + Height - 1, Width, Color);
}

inline void LCD_FillRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, SSD1306_COLOR Color)
{
	for (uint8_t i=pos_X0; i<pos_X0 + Width; i++){
    LCD_DrawVerticalLine(i, pos_Y0, Height, Color);
  }
}

inline void LCD_DrawCircle(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, SSD1306_COLOR Color)
{
  int16_t f = 1 - Radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * Radius;
  int16_t x = 0;
  int16_t y = Radius;

  LCD_DrawPixel(pos_XCenter  , pos_YCenter+Radius, Color);
  LCD_DrawPixel(pos_XCenter  , pos_YCenter-Radius, Color);
  LCD_DrawPixel(pos_XCenter+Radius, pos_YCenter  , Color);
  LCD_DrawPixel(pos_XCenter-Radius, pos_YCenter  , Color);

  while (x<y)
	{
    if (f >= 0)
		{
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    LCD_DrawPixel(pos_XCenter + x, pos_YCenter + y, Color);
    LCD_DrawPixel(pos_XCenter - x, pos_YCenter + y, Color);
    LCD_DrawPixel(pos_XCenter + x, pos_YCenter - y, Color);
    LCD_DrawPixel(pos_XCenter - x, pos_YCenter - y, Color);
    LCD_DrawPixel(pos_XCenter + y, pos_YCenter + x, Color);
    LCD_DrawPixel(pos_XCenter - y, pos_YCenter + x, Color);
    LCD_DrawPixel(pos_XCenter + y, pos_YCenter - x, Color);
    LCD_DrawPixel(pos_XCenter - y, pos_YCenter - x, Color);
  }
}

inline void LCD_DrawCircleHelper(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, uint8_t Cornername,SSD1306_COLOR Color)
{
  int16_t f     = 1 - Radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * Radius;
  int16_t x     = 0;
  int16_t y     = Radius;

  while (x<y)
	{
    if (f >= 0)
		{
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (Cornername & 0x4)
		{
      LCD_DrawPixel(pos_XCenter + x, pos_YCenter + y, Color);
      LCD_DrawPixel(pos_XCenter + y, pos_YCenter + x, Color);
    } 
    if (Cornername & 0x2)
		{
      LCD_DrawPixel(pos_XCenter + x, pos_YCenter - y, Color);
      LCD_DrawPixel(pos_XCenter + y, pos_YCenter - x, Color);
    }
    if (Cornername & 0x8)
		{
      LCD_DrawPixel(pos_XCenter - y, pos_YCenter + x, Color);
      LCD_DrawPixel(pos_XCenter - x, pos_YCenter + y, Color);
    }
    if (Cornername & 0x1)
		{
      LCD_DrawPixel(pos_XCenter - y, pos_YCenter - x, Color);
      LCD_DrawPixel(pos_XCenter - x, pos_YCenter - y, Color);
    }
  }
}

inline void LCD_FillCircle(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, SSD1306_COLOR Color)
{
	LCD_DrawVerticalLine(pos_XCenter, pos_YCenter - Radius, 2 * Radius + 1, Color);
  LCD_FillCircleHelper(pos_XCenter, pos_YCenter, Radius, 3, 0, Color);
}

inline void LCD_FillCircleHelper(uint8_t pos_XCenter, uint8_t pos_YCenter, uint8_t Radius, uint8_t Cornername, int16_t Delta, SSD1306_COLOR Color)
{
  int16_t f     = 1 - Radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * Radius;
  int16_t x     = 0;
  int16_t y     = Radius;

  while (x<y)
	{
    if (f >= 0)
		{
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (Cornername & 0x1)
		{
      LCD_DrawVerticalLine(pos_XCenter+x, pos_YCenter-y, 2*y+1+Delta, Color);
      LCD_DrawVerticalLine(pos_XCenter+y, pos_YCenter-x, 2*x+1+Delta, Color);
    }
    if (Cornername & 0x2)
		{
      LCD_DrawVerticalLine(pos_XCenter-x, pos_YCenter-y, 2*y+1+Delta, Color);
      LCD_DrawVerticalLine(pos_XCenter-y, pos_YCenter-x, 2*x+1+Delta, Color);
    }
  }
}

inline void LCD_DrawTriangle(uint8_t pos_X0, uint8_t pos_Y0, uint8_t pos_X1, uint8_t pos_Y1, uint8_t pos_X2, uint8_t pos_Y2, SSD1306_COLOR Color)
{
	LCD_DrawLine(pos_X0, pos_Y0, pos_X1, pos_Y1, Color);
  LCD_DrawLine(pos_X1, pos_Y1, pos_X2, pos_Y2, Color);
	LCD_DrawLine(pos_X2, pos_Y2, pos_X0, pos_Y0, Color);
}

inline void LCD_FillTriangle(uint8_t pos_X0, uint8_t pos_Y0, uint8_t pos_X1, uint8_t pos_Y1, uint8_t pos_X2, uint8_t pos_Y2, SSD1306_COLOR Color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (pos_Y0 > pos_Y1) {
    swap(pos_Y0, pos_Y1); swap(pos_X0, pos_X1);
  }
  if (pos_Y1 > pos_Y2) {
    swap(pos_Y2, pos_Y1); swap(pos_X2, pos_X1);
  }
  if (pos_Y0 > pos_Y1) {
    swap(pos_Y0, pos_Y1); swap(pos_X0, pos_X1);
  }

  if(pos_Y0 == pos_Y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = pos_X0;
    if(pos_X1 < a)      a = pos_X1;
    else if(pos_X1 > b) b = pos_X1;
    if(pos_X2 < a)      a = pos_X2;
    else if(pos_X2 > b) b = pos_X2;
    LCD_DrawHorizontalLine(a, pos_Y0, b-a+1, Color);
    return;
  }

  int16_t
    dx01 = pos_X1 - pos_X0,
    dy01 = pos_Y1 - pos_Y0,
    dx02 = pos_X2 - pos_X0,
    dy02 = pos_Y2 - pos_Y0,
    dx12 = pos_X2 - pos_X1,
    dy12 = pos_Y2 - pos_Y1;
  int32_t
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(pos_Y1 == pos_Y2) last = pos_Y1;   // Include y1 scanline
  else         last = pos_Y1-1; // Skip it

  for(y=pos_Y0; y<=last; y++) {
    a   = pos_X0 + sa / dy01;
    b   = pos_X0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    LCD_DrawHorizontalLine(a, y, b-a+1, Color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - pos_Y1);
  sb = dx02 * (y - pos_Y0);
  for(; y<=pos_Y2; y++) {
    a   = pos_X1 + sa / dy12;
    b   = pos_X0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    LCD_DrawHorizontalLine(a, y, b-a+1, Color);
  }
}

inline void LCD_DrawRoundRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, uint8_t Radius, SSD1306_COLOR Color) {
  // smarter version
  LCD_DrawHorizontalLine(pos_X0+Radius  , pos_Y0    			, Width-2*Radius	, Color); // Top
  LCD_DrawHorizontalLine(pos_X0+Radius  , pos_Y0+Height-1	, Width-2*Radius	, Color); // Bottom
  LCD_DrawVerticalLine(pos_X0    				, pos_Y0+Radius  	, Height-2*Radius	, Color); // Left
  LCD_DrawVerticalLine(pos_X0+Width-1		, pos_Y0+Radius  	, Height-2*Radius	, Color); // Right
  // draw four corners
  LCD_DrawCircleHelper(pos_X0+Radius    		, pos_Y0+Radius    			, Radius, 1, Color);
  LCD_DrawCircleHelper(pos_X0+Width-Radius-1, pos_Y0+Radius    			, Radius, 2, Color);
  LCD_DrawCircleHelper(pos_X0+Width-Radius-1, pos_Y0+Height-Radius-1, Radius, 4, Color);
  LCD_DrawCircleHelper(pos_X0+Radius    		, pos_Y0+Height-Radius-1, Radius, 8, Color);
}

inline void LCD_FillRoundRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height, uint8_t Radius, SSD1306_COLOR Color) {
  // smarter version
  LCD_FillRect(pos_X0+Radius, pos_Y0, Width-2*Radius, Height, Color);

  // draw four corners
  LCD_FillCircleHelper(pos_X0+Width-Radius-1, pos_Y0+Radius, Radius, 1, Height-2*Radius-1, Color);
  LCD_FillCircleHelper(pos_X0+Radius    , pos_Y0+Radius, Radius, 2, Height-2*Radius-1, Color);
}

inline void LCD_DrawVerticalDotLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Height){
	for(uint8_t i = pos_Y; i<Height ; i++){
		if((i&0x1) == 1){
			LCD_DrawPixel(pos_X,i,White);
		}else{
			LCD_DrawPixel(pos_X,i,Black);
		}
	}
}

inline void LCD_DrawHorizontalDotLine(uint8_t pos_X, uint8_t pos_Y, uint8_t Width){
	for(uint8_t i = pos_X; i<Width ; i++){
		if((i&0x1) == 1){
			LCD_DrawPixel(i,pos_Y,White);
		}else{
			LCD_DrawPixel(i,pos_Y,Black);
		}
	}
}

inline void LCD_DrawDotRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height){
	LCD_DrawVerticalDotLine(pos_X0, pos_Y0, Height);
	LCD_DrawVerticalDotLine(pos_X0 + Width -1, pos_Y0, Height);
	LCD_DrawHorizontalDotLine(pos_X0, pos_Y0, Width);
	LCD_DrawHorizontalDotLine(pos_X0, pos_Y0 + Height - 1, Width);
}

inline void LCD_FillDotRect(uint8_t pos_X0, uint8_t pos_Y0, uint8_t Width, uint8_t Height){
	for (uint8_t i = pos_X0; i<pos_X0 + Width; i+=2){
    LCD_DrawVerticalDotLine(i, pos_Y0, Height);
  }
}

