
#include "stm32f0xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "lcd.h"
#include "i2c.h"
//#include "sound.h"
#include "usart.h"
#include "util.h"
#include "oled.h"

const char font[] = {
        [' '] = 0x00,
        ['0'] = 0x3f,
        ['1'] = 0x06,
        ['2'] = 0x5b,
        ['3'] = 0x4f,
        ['4'] = 0x66,
        ['5'] = 0x6d,
        ['6'] = 0x7d,
        ['7'] = 0x07,
        ['8'] = 0x7f,
        ['9'] = 0x67,
        ['A'] = 0x77,
        ['B'] = 0x7c,
        ['C'] = 0x39,
        ['D'] = 0x5e,
        ['*'] = 0x49,
        ['#'] = 0x76,
        ['.'] = 0x80,
        ['?'] = 0x53,
        ['b'] = 0x7c,
        ['r'] = 0x50,
        ['g'] = 0x6f,
        ['i'] = 0x10,
        ['n'] = 0x54,
        ['u'] = 0x1c,
};

//
// Setup Score Display
//
void setup_portc()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER = (GPIOC->MODER & ~0x3FFFFF) | 0x155555;
}

//
// Setup Score Display Handler
//
int offset = 0;
char display[8];
void setup_tim16()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
	TIM16->PSC = 1;
	TIM16->ARR = 10000;
	TIM16->DIER |= TIM_DIER_UIE;
	TIM16->CR1 |= TIM_CR1_CEN;

	NVIC->ISER[0] = 1 << TIM16_IRQn;
}

//
// Output tick to Seven-Segment Display
//
void TIM16_IRQHandler()
{
    TIM16->SR &= ~TIM_SR_UIF;

	int off = offset & 7;
	GPIOC->ODR = (off << 8) | display[off];

    offset = (offset + 1) & 0x7;
}

// Turn integer to seven-segment byte-code
int int_to_font(int value)
{
	if(value > 9)
	{
		return font['#'];
	}
	return font['0' + value];
}

// Put an integer onto the seven-segment display
void display_int(int value)
{
	int i, digit;
	int last_digit = 0;
	for(i = 0; i < 8; i++)
	{
		digit = (value / (int)pow10(7 - i)) % 10;
//		if(digit > 0 || last_digit > 0)
//		{
			display[i] = int_to_font(digit);
//		}
		last_digit = digit;
	}
}

//
// Setup Game Ticker
//
int score = 0;
int high_score;
int tick = 0;
int last_pause_tick = -1;
bool game_over = true;
bool pause = false;
int bounces = 0;
void setup_tim17()
{
    // Set this to invoke the ISR 100 times per second.
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
	TIM17->PSC = 47999;
	TIM17->ARR = 9;
	TIM17->DIER |= TIM_DIER_UIE;
	TIM17->CR1 |= TIM_CR1_CEN;

	NVIC->ISER[0] = 1 << TIM17_IRQn;
}

void shutdown_tim17()
{
	TIM17->CR1 &= ~TIM_CR1_CEN;
}

//
// Setup Keypad
//
void setup_portb()
{
    // Enable Port B.
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // Set PB0-PB3 for output.
	GPIOB->MODER = (GPIOB->MODER & ~0xFF) | 0x55;

    // Set PB4-PB7 for input and enable pull-down resistors.
	GPIOB->MODER &= ~0xFF00;
	GPIOB->PUPDR = (GPIOB->PUPDR & ~0xFF00) | 0xAA00;
}

//
// Get Keypad Key
//
char check_key()
{
    // If the '*' key is pressed, return '*'
    // If the 'D' key is pressed, return 'D'
	int row;
	for(row = 0; row < 4; row++)
	{
		GPIOB->BSRR = 0xF0000 | (1 << row);
		int column = (GPIOB->IDR >> 4) & 0xF;
		if(row == 1)
		{
			switch(column)
			{
				case 1 << 0:
					return '4';
				case 1 << 1:
					return '5';
				case 1 << 2:
					return '6';
			}
		}
		else if(row == 0)
		{
			switch(column)
			{
				case 1 << 1:
					return '2';
			}
		}
		else if(row == 3)
		{
			switch(column)
			{
				case 1 << 0:
					return '*';
				case 1 << 1:
					return '0';
				case 1 << 2:
					return '#';
				case 1 << 3:
					return 'D';
			}
		}
	}
    // Otherwise, return 0
	return 0;
}

void setup_spi1()
{
    // Use setup_spi1() from lab 8.
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFR4 | GPIO_AFRL_AFR5 | GPIO_AFRL_AFR7);
	GPIOA->MODER = (GPIOA->MODER & ~0xCFF0) | 0x8A50;

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 = (SPI1->CR1 & ~SPI_CR1_BR) | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
	SPI1->CR2 = (SPI1->CR2 & ~SPI_CR2_DS) | SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2;
	SPI1->CR1 |= SPI_CR1_SPE;
}


// Copy a subset of a large source picture into a smaller destination.
// sx,sy are the offset into the source picture.
void pic_subset(Picture *dst, const Picture *src, int sx, int sy)
{
    int dw = dst->width;
    int dh = dst->height;
    if (dw + sx > src->width)
        for(;;)
            ;
    if (dh + sy > src->height)
        for(;;)
            ;
    for(int y=0; y<dh; y++)
        for(int x=0; x<dw; x++)
            dst->pix2[dw * y + x] = src->pix2[src->width * (y+sy) + x + sx];
}

// Overlay a picture onto a destination picture.
// xoffset,yoffset are the offset into the destination picture that the
// source picture is placed.
// Any pixel in the source that is the 'transparent' color will not be
// copied.  This defines a color in the source that can be used as a
// transparent color.
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src, int transparent)
{
    for(int y=0; y<src->height; y++) {
        int dy = y+yoffset;
        if (dy < 0 || dy >= dst->height)
            continue;
        for(int x=0; x<src->width; x++) {
            int dx = x+xoffset;
            if (dx < 0 || dx >= dst->width)
                continue;
            unsigned short int p = src->pix2[y*src->width + x];
            if (p != transparent)
                dst->pix2[dy*dst->width + dx] = p;
        }
    }
}

// Called after a bounce, update the x,y velocity components in a
// pseudo random way.  (+/- 1)
void perturb(int *vx, int *vy)
{
    if (*vx > 0) {
        *vx += (random()%3) - 1;
        if (*vx >= 3)
            *vx = 2;
        if (*vx == 0)
            *vx = 1;
    } else {
        *vx += (random()%3) - 1;
        if (*vx <= -3)
            *vx = -2;
        if (*vx == 0)
            *vx = -1;
    }
    if (*vy > 0) {
        *vy += (random()%3) - 1;
        if (*vy >= 3)
            *vy = 2;
        if (*vy == 0)
            *vy = 1;
    } else {
        *vy += (random()%3) - 1;
        if (*vy <= -3)
            *vy = -2;
        if (*vy == 0)
            *vy = -1;
    }
}

extern const Picture background; // A 240x320 background image
extern const Picture particle; // A 19x19 purple ball with white boundaries
extern const Picture ship; // A 59x5 ship
extern const Picture splash;

const int border = 10;
int xmin; // Farthest to the left the center of the ball can go
int xmax; // Farthest to the right the center of the ball can go
int ymin; // Farthest to the top the center of the ball can go
int ymax; // Farthest to the bottom the center of the ball can go
int x,y; // Center of ball
int vx,vy; // Velocity components of ball

int px; // Center of ship offset
int py;
int newpx; // New center of ship
int newpy;

// This C macro will create an array of Picture elements.
// Really, you'll just use it as a pointer to a single Picture
// element with an internal pix2[] array large enough to hold
// an image of the specified size.
// BE CAREFUL HOW LARGE OF A PICTURE YOU TRY TO CREATE:
// A 100x100 picture uses 20000 bytes.  You have 32768 bytes of SRAM.
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

// Create a 29x29 object to hold the ball plus padding
TempPicturePtr(object,29,29);

//
// Initialize the Game
//
void start_game()
{
	score = 0;
	bounces = 0;
	game_over = false;
	LCD_DrawPicture(0,0,&background);

	// Set all pixels in the object to white.
	for(int i=0; i<29*29; i++)
		object->pix2[i] = 0xffff;

	// Center the 19x19 ball into center of the 29x29 object.
	// Now, the 19x19 ball has 5-pixel white borders in all directions.
	pic_overlay(object,5,5,&particle,0xffff);

	// Initialize the game state.
	xmin = border + particle.width/2;
	xmax = background.width - border - particle.width/2;
	ymin = border + particle.width/2;
	ymax = background.height - border - particle.height/2;
	x = (xmin+xmax)/2; // Center of ball
	y = ymin;
	vx = 0; // Velocity components of ball
	vy = 1;

	px = -1; // Center of ship offset (invalid initial value to force update)
	py = -1;
	newpx = (xmax + xmin) / 2;
	newpy = (ymax + ymin) / 2;

}

int get_high_score()
{
	int var;
    i2c_read_flash(0x0, (char *) &var, sizeof(var));
    return var;
}

void save_high_score()
{
    i2c_write_flash(0x0, (char *) &high_score, sizeof(high_score));
	while(i2c_write_flash_complete() == 0);
}

void reset_high_score()
{
	high_score = 0;
	i2c_write_flash(0x0, (char *) &high_score, sizeof(high_score));
	while(i2c_write_flash_complete() == 0);
}

void display_high_score()
{
    high_score = get_high_score();
    char high_score_msg[num_digits(high_score)];
    sprintf(high_score_msg, "%d", high_score);
    oled_cdisplay1("Highscore:");
	oled_cdisplay2(high_score_msg);
}

void end_game()
{
	game_over = true;
	LCD_DrawPicture(0,0,&background);
	LCD_DrawPicture(
			(background.width - splash.width) / 2,
			(background.height - splash.height) / 2,
			&splash);

	// New high score
	if(score > high_score)
	{
		high_score = score;
		save_high_score();
		display_high_score();
	    oled_cdisplay1("NEW HIGHSCORE!");

		// Flash LED's
		while(check_key() != '*')
		{
			for(int n=1; n <= 8; n <<= 1)
			{
				i2c_set_gpio(n);
				int value = i2c_get_gpio();
				if ((value & 0xf) != n)
					break;
				nano_wait(50000000); // 0.1 s
			}
		}
	    oled_cdisplay1("Highscore:");
	}
}

void TIM17_IRQHandler(void)
{
    TIM17->SR &= ~TIM_SR_UIF;

    // If game is over parse setting
    if(game_over)
    {
    	if(check_key() == '*')
    	{
			start_game();
    	}
    	return;
    }

    tick++;

	// Handle the Key-press //
    char key = check_key();

    // Pause - wait for un-pause (with bootleg-debounce)
    if(key == '#' && (tick - last_pause_tick > 25))
    {
		last_pause_tick = tick;
		pause = !pause;
    }
    if(pause)
    {
    	return;
    }

    key = check_key();
    switch(key)
    {
    	case '2':
			newpy--;
			break;
    	case '5':
    		newpy++;
    		break;
    	case '4':
            newpx--;
            break;
    	case '6':
            newpx++;
            break;
    }

    // Update score
    if((tick % 10) == 0)
    	display_int(++score);

    if (newpx - ship.width/2 <= border || newpx + ship.width / 2 >= background.width - border)
        newpx = px;

    if (newpy - ship.height/2 <= border || newpy + ship.height / 2 >= background.height - border)
		newpy = py;

    // Update Ship Location
    if (newpx != px || newpy != py)
    {
        px = newpx;
        py = newpy;
        TempPicturePtr(tmp, 32, 34);

        pic_subset(tmp, &background, px - tmp->width/2, py); // Copy the background
        pic_overlay(tmp, 1, 1, &ship, -1);
        LCD_DrawPicture(px - tmp->width/2, py, tmp);
    }

    x += vx;
    y += vy;

    // Ball hit left
    if (x <= xmin)
    {
        bounces++;
        vx = - vx;
        if (x < xmin)
            x = xmin + 1;
        perturb(&vx,&vy);
    }
    // Ball hit right
    else if (x >= xmax)
    {
        bounces++;
        vx = -vx;
        if (x > xmax)
            x = xmax - 1;
        perturb(&vx,&vy);
    }

    // Ball hit top
    if (y <= ymin)
    {
        bounces++;
        vy = -vy;
        if (y < ymin)
            y = ymin + 1;
        perturb(&vx,&vy);
    }
	// Ball hit bottom
    else if (y >= ymax)
    {
    	bounces++;
		vy = -vy;
		if (y > ymax)
			y = ymax - 1;
		perturb(&vx,&vy);
	}

    // Ball hit ship
    if (y <= (py + (ship.height/2 + 30)) &&
		y >= (py - ship.height/2) &&
    	x >= (px - (ship.width/2 + 15)) &&
        x <= (px + (ship.width/2 + 15))) {

    	// Game over
    	end_game();
    }


    TempPicturePtr(tmp,29,29); // Create a temporary 29x29 image.
    pic_subset(tmp, &background, x - tmp->width/2, y - tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, object, 0xffff); // Overlay the object

    LCD_DrawPicture(x - tmp->width/2, y - tmp->width/2, tmp); // Re-draw it to the screen
    // The object has a 5-pixel border around it that holds the background
    // image.  As long as the object does not move more than 5 pixels (in any
    // direction) from it's previous location, it will clear the old object.
}

int main(void)
{
    setup_portb(); // Push buttons
    setup_portc(); // Seven-segment output

    setup_i2c();   // Storage
    i2c_set_iodir(0xf0);

    setup_spi1();  // LCD display
    LCD_Init();

    setup_tim16(); // Score display
	setup_tim17(); // Game ticker

	setup_spi2_oled(); // OLED display
	init_oled();
    display_high_score();

	setup_usart5(); // Admin command handler
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    enable_tty_interrupt();


	LCD_DrawPicture(0,0,&background);
	LCD_DrawPicture(
			(background.width - splash.width) / 2,
			(background.height - splash.height) / 2,
			&splash);

	// Handle Admin Commands //
	printf("\nEnter commands here. Type \"help\" for a list of commands.\n");
	for(;;)
	{
		printf("> ");
		char line[100];
		fgets(line, 99, stdin);
		line[99] = '\0'; // just in case

		if(strcmp(line, "reset\n") == 0)
		{
			reset_high_score();
			display_high_score();
			printf("High score reset\n");
		}
		else if(strcmp(line, "hs\n") == 0)
		{
			int high_score = get_high_score();
			printf("Highscore: %d\n", high_score);
		}
		else if(strcmp(line, "pause\n") == 0)
		{
			pause = !pause;
//			GPIOC->ODR = 0;
			if(pause)
				printf("Game paused\n");
			else
				printf("Game unpaused\n");
		}
		else if(strcmp(line, "stop\n") == 0)
		{
			if(!game_over)
				end_game();
			printf("Game stopped\n");
		}
		else
		{
			printf("Commands:\n"
					"\thelp:  show all commands\n"
					"\ths:    get highscore\n"
					"\treset: reset highest score\n"
					"\tpause: pause game\n"
					"\tstop:  stop the game\n");
		}
	}


}
