#ifndef __PONG_GAME_H__
#define __PONG_GAME_H__
#include "ssd1306.h"

// Pong size define
#define PONG_X_DEFAULT				(SSD1306_WIDTH/2)
#define PONG_Y_DEFAULT				(SSD1306_HEIGHT/2)
#define PONG_RADIUS_DEFAULT		2

// Paddle size define
#define PADDLE_WIDTH_DEFAULT	2
#define PADDLE_HEIGHT_DEFAULT	10
#define PADDLE1_X_DEFAULT			2
#define PADDLE2_X_DEFAULT			(SSD1306_WIDTH - PADDLE_WIDTH_DEFAULT - PADDLE1_X_DEFAULT)
#define PADDLE_Y_DEFAULT			((SSD1306_HEIGHT/2) - (PADDLE_HEIGHT_DEFAULT/2))

// Data structure
typedef struct Paddle_t{
	uint8_t X_pos;
	int8_t Y_pos;
	uint8_t Width;
	uint8_t Height;
}Paddle;

typedef struct Pong_t{
	uint8_t X_pos;
	uint8_t Y_pos;
	uint8_t Radius;
}Pong;

typedef struct Score_t{
	uint8_t X_pos;
	uint8_t Y_pos;
	uint8_t Point;
}Score;

// Enum
enum PONG_X_DIRECTION{
	PONG_X_LEFT 	= -1,
	PONG_X_RIGHT 	= 1,
};

enum PONG_Y_DIRECTION{
	PONG_Y_UP 	= -1,
	PONG_Y_DOWN = 1,
};

enum PADDLE_DIRECTION{
	PADDLE_UP = -1,
	PADDLE_FREEZE = 0,
	PADDLE_DOWN = 1,
};

// Common method


// Pong method
void PongMove(void);

// Paddle method
void PaddleMove(int Paddle1_direct,int Paddle2_direct);

#endif //!__PONG_GAME_H__

