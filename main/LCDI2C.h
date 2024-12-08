#ifndef INC_LCDI2C_H_
#define INC_LCDI2C_H_


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

//definicion de constantes y de los puertos de entrada

#define ADDR_LCD 0x27           // direccion del dispositivo bus i2c
#define RS_BIT	0
#define RW_BIT	1
#define E_BIT 	2

//creamos str
  
typedef enum{
	fila1=0,
	fila2,
	fila3,
	fila4
}Ubicacion;

void LCD_PICO_CMDi2c(uint8_t a,uint8_t rs);
void LCD_PICO_INIT_I2C(void);
void LCD_PICO_SET_CURSOR(uint8_t x,Ubicacion y);
void LCD_PICO_PRINT_STRINGi2c(const char* str);
void LCD_PICO_New_Chari2c(uint8_t a,uint8_t b,uint8_t c,uint8_t d ,uint8_t e ,uint8_t f,uint8_t g,uint8_t h,uint8_t i);
void LCD_PICO_Print_New_Char(uint8_t a);
void LCD_PICO_Clear(void);
void LCD_PICO_Home(void);


#endif /* INC_LCDI2C_H_ */