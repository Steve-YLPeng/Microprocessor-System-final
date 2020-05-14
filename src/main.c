
#define SET_REG(REG,SELECT,VAL){((REG)=((REG)&(~(SELECT)))|(VAL));};
#define TIME_SEC 5.78
#define TIM_ARR_VAL 3999
#define MSI_DEFAULT_FREQ 4000000
#include "stm32l476xx.h"
//TODO: define your gpio pin
/*#define X0 //PA8
#define X1 //PA9
#define X2 //PA10
#define X3 //PA12
#define Y0 //PB5
#define Y1 //PB6
#define Y2 //PB7
#define Y3 //PB9*/
//unsigned int x_pin[4] = {X0, X1, X2, X3};
//unsigned int y_pin[4] = {Y0, Y1, Y2, Y3};

int Table[4][4]={
		{1,2,3,-1},
		{4,5,6,-1},
		{7,8,9,-1},
		{-1,0,-1,-1}
};

int menu[10]={
	//bit map
	//espresso, tea, chocolate, coffee, milk
	0b00000,
	0b10000,
	0b01000,
	0b00100,
	0b00010,
	0b10001,
	0b10010,
	0b01010,
	0b01001,
	0b10101
};

//These functions inside the asm file
extern void GPIO_init();
extern void max7219_init();
extern void MAX7219Send(unsigned char address, unsigned char data);

/* TODO: initial keypad gpio pin, X as output and Y as input
*/
void keypad_init()
{
	// SET keypad gpio OUTPUT //
	RCC->AHB2ENR = RCC->AHB2ENR|0b111;
	//Set PA8,9,10,12 as output mode
	GPIOA->MODER= GPIOA->MODER&0xFDD5FFFF;
	//set PA8,9,10,12 is Pull-up output
	GPIOA->PUPDR=GPIOA->PUPDR|0x1150000;
	//Set PA8,9,10,12 as medium speed mode
	GPIOA->OSPEEDR=GPIOA->OSPEEDR|0x1150000;
	//Set PA8,9,10,12 as high
	GPIOA->ODR=GPIOA->ODR|0b10111<<8;
	// SET keypad gpio INPUT //
	//Set PB5,6,7,9 as INPUT mode
	GPIOB->MODER=GPIOB->MODER&0xFFF303FF;
	//set PB5,6,7,9 is Pull-down input
	GPIOB->PUPDR=GPIOB->PUPDR|0x8A800;
	//Set PB5,6,7,9 as medium speed mode
	GPIOB->OSPEEDR=GPIOB->OSPEEDR|0x45400;

	GPIOC->MODER &= 0xFFFF0000;
	GPIOC->MODER |= 0b0101010101010101;
	//GPIOC->PUPDR &= 0xFFFFFF00;
	//GPIOC->PUPDR |= 0b01010101;
	GPIOC->OSPEEDR &= 0xFFFF0000;
	GPIOC->OSPEEDR |= 0b1010101010101010;
	//GPIOC->OTYPER &= 0b0000;
	GPIOC->ODR=GPIOC->ODR|0b00000<<3 ;
}
void Timer_init( /*TIM_TypeDef *timer*/)
{
 //TODO: Initialize timer
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	SET_REG(TIM2->CR1, TIM_CR1_DIR , 1<<4);//down counter
	TIM2->ARR = (uint32_t)TIME_SEC*100;//Reload value
	TIM2->PSC = (uint32_t)39999;//Prescalser
	TIM2->EGR = TIM_EGR_UG;//Reinitialize the counte
}
void Timer_start(/*TIM_TypeDef *timer*/){
 //TODO: start timer and show the time on the 7-SEG LED.
	TIM2->CR1 |= TIM_CR1_CEN;//start timer
	int pre_val = TIME_SEC*100;
	while(1){
		int timerValue = TIM2->CNT;//polling the counter value
		if(pre_val < timerValue){//check if times up
			TIM2->CR1 &= ~TIM_CR1_CEN;
			return;
		}
		pre_val = timerValue;
		//int dis_val = TIME_SEC*100*timerValue/TIM_ARR_VAL;//convert counter value to time(seconds)
		int dis_val = TIME_SEC*100-timerValue;
		display_sec(dis_val, 8);//display the time on the 7-SEG LED
	}
}
void display_sec(int data, int num_digs)
{
	for(int i=1; i<=num_digs; i++){
		int num = data%10;
		data/=10;
		MAX7219Send(i, (i==3 ? (num|1<<7) : num));
		if(data==0){
			for(int j=i+1 ; j<=num_digs; j++)
				if(j<=3)MAX7219Send(j, (j==3 ? 1<<7 : 0));
				else MAX7219Send(j, 15);
			break;
		}
	}
	for(int i=num_digs+1; i<=8; i++)
		MAX7219Send(i, 15);
}
/**
* TODO: Show data on 7-seg via max7219_send
* Input:
* data: decimal value
* num_digs: number of digits will show on 7-seg
* Return:
* 0: success
* -1: illegal data range(out of 8 digits range)
*/
void display(int data)
{
	int num = data/10;
	if(num)MAX7219Send(2,num);
	else MAX7219Send(2,15);
	data %= 10;
	MAX7219Send(1,data);
}
void blank()
{
	for(int i=1; i<=8; i++)
		MAX7219Send(i,15);
}
void main()
{
	keypad_init();
	max7219_init();
	blank();

	Timer_init();

	/*Timer_start();
	blank();
	Timer_start();
	blank();*/

	int flag_keypad, k, flag_debounce, flag_keypad_r, position_r, position_c, select=-1;
	while(1){
		flag_keypad=GPIOB->IDR&0b10111<<5;
		if(flag_keypad!=0){
			k=4500;
			while(k!=0){
				flag_debounce=GPIOB->IDR&0b10111<<5;
				k--;
			}
			if(flag_debounce!=0){
				for(int i=0;i<4;i++){ //scan keypad from first column
					position_c=i+8;
					if(i==3)position_c++;
					//set PA8,9,10,12(column) low and set pin high from PA8
					GPIOA->ODR=(GPIOA->ODR&0xFFFFE8FF)|1<<position_c;
					for(int j=0;j<4;j++){ //read input from first row
						position_r=j+5;
						if(j==3)
							position_r++;
						flag_keypad_r=GPIOB->IDR&1<<position_r;
						if(flag_keypad_r!=0){

							display(Table[j][i]);

							if(Table[j][i]==0 && select>0 && select<=9){
								GPIOC->ODR=GPIOC->ODR|menu[select]<<3;
								Timer_start();
								blank();
								GPIOC->ODR=GPIOC->ODR&0b00000111;
							}


							select=Table[j][i];
							GPIOA->ODR=GPIOA->ODR|0b10111<<8;
							i=j=4;
						}
					}
				}
			}
		}
	}
}
