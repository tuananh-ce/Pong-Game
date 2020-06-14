#include "Pong_Game.h"
#include <stdlib.h>
// Pong direction
static int X_direction = PONG_X_LEFT;
static int Y_direction = PONG_Y_UP;

// Paddle object
Paddle Game_Paddle1 = {PADDLE1_X_DEFAULT,PADDLE_Y_DEFAULT,PADDLE_WIDTH_DEFAULT,PADDLE_HEIGHT_DEFAULT};
Paddle Game_Paddle2 = {PADDLE2_X_DEFAULT,PADDLE_Y_DEFAULT,PADDLE_WIDTH_DEFAULT,PADDLE_HEIGHT_DEFAULT};

// Pong object
Pong Game_Pong = {PONG_X_DEFAULT,PONG_Y_DEFAULT, PONG_RADIUS_DEFAULT};

// Movement of pong
void PongMove(void){
	if(((Game_Pong.X_pos-Game_Pong.Radius) == 0) ||								// Bound of pong hit the left wall
		(Game_Pong.X_pos+Game_Pong.Radius) >= (SSD1306_WIDTH-1)){		// Bound of pong hit the right wall
		// Reset Pong position
		Game_Pong.X_pos = PONG_X_DEFAULT;
		Game_Pong.Y_pos = PONG_Y_DEFAULT;
		Game_Pong.Radius = PONG_RADIUS_DEFAULT;
		// Initialize random direction
		int pong_rand_direct = rand();
		if ((pong_rand_direct&0x1) == 1){
			X_direction = PONG_X_LEFT;
		}else{
			X_direction = PONG_X_RIGHT;
		}
		pong_rand_direct = rand();
		if ((pong_rand_direct&0x1) == 1){
			Y_direction = PONG_Y_UP;
		}else{
			Y_direction = PONG_Y_DOWN;
		}
		
		//X_direction = PONG_X_LEFT;
		//Y_direction = PONG_Y_UP;
	}
		
	// Bound of pong hit the left paddle
	if(((Game_Pong.X_pos-Game_Pong.Radius) == (Game_Paddle1.X_pos+Game_Paddle1.Width))	// Check X cordinate
		&& (Game_Pong.Y_pos >= Game_Paddle1.Y_pos)																				// Check Y range cordinate
	  && (Game_Pong.Y_pos <= (Game_Paddle1.Y_pos + Game_Paddle1.Height - 1))){
		X_direction = PONG_X_RIGHT;
	}

	// Bound of pong hit the right paddle
	if(((Game_Pong.X_pos+Game_Pong.Radius) == (Game_Paddle2.X_pos))											// Check X cordinate
		&& (Game_Pong.Y_pos >= Game_Paddle2.Y_pos)																				// Check Y range cordinate
		&& (Game_Pong.Y_pos <= (Game_Paddle2.Y_pos + Game_Paddle2.Height -1))){
		X_direction = PONG_X_LEFT;
	}
	Game_Pong.X_pos += X_direction;
	
	// Bound of pong hit the top border
	if((Game_Pong.Y_pos-Game_Pong.Radius) == 0){
		Y_direction = PONG_Y_DOWN;
	}
	// Bound of pong hit the bottom border
	if((Game_Pong.Y_pos+Game_Pong.Radius) >= (SSD1306_HEIGHT-1)){
		Y_direction = PONG_Y_UP;
	}
	Game_Pong.Y_pos += Y_direction;
}

// Movement of paddle
void PaddleMove(int Paddle1_direct,int Paddle2_direct){
	Game_Paddle1.Y_pos += Paddle1_direct;
	// Check paddle1 hit top boder
	if(Game_Paddle1.Y_pos <= 0){
		Game_Paddle1.Y_pos = 0;
	}
	// Check paddle1 hit bottom boder
	if((Game_Paddle1.Y_pos + Game_Paddle1.Height-1) >= (SSD1306_HEIGHT - 1)){
		Game_Paddle1.Y_pos = SSD1306_HEIGHT - Game_Paddle1.Height;
	}
	
	Game_Paddle2.Y_pos += Paddle2_direct;
	// Check paddle2 hit top boder
	if(Game_Paddle2.Y_pos <= 0 ){
		Game_Paddle2.Y_pos = 0;
	}
	// Check paddle2 hit bottom boder
	if((Game_Paddle2.Y_pos + Game_Paddle2.Height - 1) >= (SSD1306_HEIGHT - 1)){
		Game_Paddle2.Y_pos = SSD1306_HEIGHT - Game_Paddle2.Height;
	}
}

