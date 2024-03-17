#include <stm8s.h>    
#include <stm8s_gpio.h>    
#include <stm8s_tim1.h>    
#include <pins.h>    
   

@inline void _nop_( void )
{
  _asm("nop\n");
}

#define uchar  unsigned char
#define uint  unsigned int

void WriteCommand(int t);

void WriteCommand0( uchar command);
void WriteCommand1( uchar dat1,uchar command);
void WriteCommand2( uchar dat1,uchar dat2,uchar command );
@inline void WriteData( uchar DataByte );
void LcmClear( void ); 
void LcmInit( void );
void DisplayBMP(uchar x,uchar y,uchar W,uchar H, @near uchar const  *puts );
void Delay(uint MS); 


void Locate0(void ) {
	//START ADDRESS 0x0800
	WriteCommand2(0,0x08,0x24);
}
/***************************/   
/*LCM READ AND WRITE BUSY CHECK                   
/***************************/ 

void CheckBusy_S0S1(void)
{
	uchar b;
	GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_IN_FL_NO_IT);
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_NO_IT); //WARN patched PB2->PD2
	do	
	{
		SetCD1();
		SetRD0();
		b=DISP_GPIO_DATA->IDR;
	  SetRD1();	
	}
	while(b&0x03!=0x03);
  GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST);  //WARN patched PB2->PD2
		
 }

/***************************/   
/*DATA AUTO READ BUSY CHECK                    
/***************************/ 

void CheckBusy_S2(void)
{
	uchar b;
	GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_IN_FL_NO_IT);
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_NO_IT); //WARN patched PB2->PD2
	do	
	{
		SetCD1();
		SetRD0();
		b=DISP_GPIO_DATA->IDR;
		SetRD1();	
	}
	while(b&0x03==0);
  GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST); //WARN patched PB2->PD2
	 	
}

/***************************/   
/*WRITE DATA                 */   
/***************************/   
@inline void WriteData( uchar DataByte )   
{    
	SetCD0();   
	DISP_GPIO_DATA->ODR = DataByte;   
 	GPIOD->ODR = DataByte;  //WARN patched PB2->PD2
	SetWR0();  
	_nop_();    
	SetWR1();   
}   
   
/***************************/   
/*DATA AUTO WRITE BUSY CHECK                    
/***************************/ 

void CheckBusy_S3(void)
{uchar b;
	 GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_IN_FL_NO_IT);
	 GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_NO_IT); //WARN patched PB2->PD2

	 do	
	{
	 SetCD1();
	 SetRD0();
	 b=DISP_GPIO_DATA->IDR;
	 SetRD1();	
	 }
	 while(b&0x08!=0x08);
   GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_OUT_PP_HIGH_FAST);
	 GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST); //WARN patched PB2->PD2
}

/***************************/   
/*WRITE COMMANDS             */   
/***************************/   
void WriteCommand2( uchar dat1,uchar dat2,uchar command)   
{   
  CheckBusy_S0S1();	
 	WriteData(dat1);
	
	CheckBusy_S0S1();	
 	WriteData(dat2);

	CheckBusy_S3();
 	SetCD1();   
 	DISP_GPIO_DATA->ODR = command; 
 	GPIOD->ODR = command; 
 	SetWR0(); 
	_nop_();     
 	SetWR1(); 
}   

void WriteCommand1( uchar dat1, uchar command )   
{   

	CheckBusy_S0S1();	
 	WriteData(dat1);

	CheckBusy_S3();
 	SetCD1();   
 	DISP_GPIO_DATA->ODR = command; 
 	GPIOD->ODR = command; 
 	SetWR0(); 
	_nop_();     
 	SetWR1(); 

}   

void WriteCommand0(uchar command)   
{   
	CheckBusy_S3();
 	SetCD1();   
 	DISP_GPIO_DATA->ODR = command; 
 	GPIOD->ODR = command; 
 	SetWR0(); 
	_nop_();     
 	SetWR1(); 
}   

   
void LcmClear( void )   
{   uint a;
	WriteCommand2(0x00,0x00,0x24);
	WriteCommand0(0xb0); 
	for(a=0;a<8192;a++)
	{
		WriteData( 0 ); 
	}   
	WriteCommand0(0xb2);
}   

void Delay(uint MS)   
{   
 unsigned char us,usn;   
 while(MS!=0)   
 {   
  usn = 2; //for 12M    
  while(usn!=0)   
  {   
   us=0xf6;   
   while (us!=0){us--;};   
   usn--;   
  }   
  MS--;   
 }  
 //while(log);
}   

void LcmInit( void )   
{  
 	SetRESET0();
 	Delay(50);
	SetRESET1();
 	Delay(50);
	WriteCommand2(0,0,0x40 );   
	WriteCommand2(30,0,0x41 );    
	WriteCommand2(0,0x08,0x42 );   
	WriteCommand2(30,0,0x43 );  
	WriteCommand0(0xa7);
	WriteCommand0(0x80);
	WriteCommand0(0x9c);   
}   

void testPins( void )
{
	while(1) {
		SetWR0();
		SetWR1();
	
		SetRD0();
		SetRD1();
	
		SetCD0();
		SetCD1();
	
		SetRESET0();
		SetRESET1();
		
		DISP_GPIO_DATA->ODR = 1;
		DISP_GPIO_DATA->ODR = 2;
		DISP_GPIO_DATA->ODR = 4;
		DISP_GPIO_DATA->ODR = 8;
		DISP_GPIO_DATA->ODR = 16;
		DISP_GPIO_DATA->ODR = 32;
		DISP_GPIO_DATA->ODR = 64;
		DISP_GPIO_DATA->ODR = 128;
	}
}

enum DISP_STATE {
	D_ESCAPE, D_BRIGHTNESS, D_WRITE, D_ERROR
};

#define ESC_BYTE (0xAA)
#define ESC_ESC_BYTE (0xA9)
#define ESC_START_BYTE (0xA0)
uint8_t readUartByte(void);
void display_run( void )   
{
	uint8_t c;
	enum DISP_STATE state = D_ERROR;
//	testPins();
	LcmInit(); 
	GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_IN_FL_NO_IT);
	LcmClear(); 

  while(1)  
	{
		c = readUartByte();
		
		if(c==ESC_BYTE) {
			state = D_ESCAPE;
		} else {
		
			switch(state) {
				case D_ESCAPE: 
					switch(c) {
						case ESC_ESC_BYTE:
							WriteData(ESC_BYTE);
							state = D_WRITE;
							break;
						case ESC_START_BYTE:
							WriteCommand0(0xb2 );
							state = D_BRIGHTNESS;
							Locate0();
							break;
						default:
							state = D_ERROR;
							break;
					}
				break;
				case D_BRIGHTNESS:
					if(c < 101) {
						TIM1_SetCompare4(c);
						state = D_WRITE;
						WriteCommand0(0xb0);
					} else {
						state = D_ERROR;
					}
				break;
				case D_WRITE:
					WriteData(c);
					break;
			}
		}
	} 
}
  
