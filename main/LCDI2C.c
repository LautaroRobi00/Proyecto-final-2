//inclusion de archivos de cabecera

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "LCDI2C.h"



//Librerias standar de ANSI C
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


//Funcion para enviar los datos al LCD por 4bits nh y nl

 void LCD_PICO_CMDi2c(uint8_t a,uint8_t rs){
	uint8_t dato = (a & 0xF0);                                  // envio el nible alto primero
	dato |= rs<<RS_BIT;                                         // bit 0
	dato |= 1<<E_BIT;                                           // bit 1
	dato |= 1<<3;                                               // backlight always on
	i2c_write_blocking(i2c_default,ADDR_LCD,&dato,1,false);
	sleep_ms(2);
	dato &= ~(1<<E_BIT);                                        // E = 0
	i2c_write_blocking(i2c_default,ADDR_LCD,&dato,1,false);

  sleep_us(100);

  dato = (a << 4);                                              // envio el nible bajo de segundo
  dato |= rs<<RS_BIT;                                           // bit 0
	dato |= 1<<E_BIT;                                           // bit 1
	dato |= 1<<3;                                               // backlight always on
	i2c_write_blocking(i2c_default,ADDR_LCD,&dato,1,false);
	sleep_ms(2);
	dato &= ~(1<<E_BIT);                                        // E = 0
	i2c_write_blocking(i2c_default,ADDR_LCD,&dato,1,false); 

  
}

//Funcion para inicializar el LCD 4bits

void LCD_PICO_INIT_I2C(void){

	LCD_PICO_CMDi2c(0x03,0);      // comando 0x03 , rs = 0
	sleep_ms(3);
	LCD_PICO_CMDi2c(0x03,0);
	LCD_PICO_CMDi2c(0x03,0);
	LCD_PICO_CMDi2c(0x02,0);
	LCD_PICO_CMDi2c(0x02,0);
	LCD_PICO_CMDi2c(0x08,0);
	LCD_PICO_CMDi2c(0x00,0);
	LCD_PICO_CMDi2c(0x08,0);
	LCD_PICO_CMDi2c(0x00,0);
	LCD_PICO_CMDi2c(0x01,0);
	LCD_PICO_CMDi2c(0x00,0);
	//Con cursor activado es 0XD,  sin cursor : 0xC
	LCD_PICO_CMDi2c(0x0C,0);

    LCD_PICO_New_Chari2c(1,0x00,0x0A,0x0A,0x00,0x11,0x0A,0x04,0x00);     // creamos 1 caracter personalizado
    LCD_PICO_New_Chari2c(2,0x0E,0x1B,0x11,0x11,0x11,0x11,0x11,0x1F);
	LCD_PICO_New_Chari2c(3,0x0E,0x1B,0x11,0x11,0x11,0x11,0x1F,0x1F);
	LCD_PICO_New_Chari2c(4,0x0E,0x1B,0x11,0x11,0x11,0x1F,0x1F,0x1F);
	LCD_PICO_New_Chari2c(5,0x0E,0x1B,0x11,0x11,0x1F,0x1F,0x1F,0x1F);
	LCD_PICO_New_Chari2c(6,0x0E,0x1B,0x11,0x1F,0x1F,0x1F,0x1F,0x1F);
	LCD_PICO_New_Chari2c(7,0x0E,0x1B,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F);
	LCD_PICO_New_Chari2c(8,0x0E,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F);

	sleep_ms(20);
}

//Funcion para ubicar el cursor en las coordenadas deseadas

void LCD_PICO_SET_CURSOR(uint8_t x,Ubicacion y){

	uint8_t m;

	switch (y) {
		case fila1:
			m=(0x80+(x-1));
			LCD_PICO_CMDi2c(m,0);
			break;

		case fila2:
			m=(0xC0+(x-1));
			LCD_PICO_CMDi2c(m,0);
      break;

		case fila3:
      m=(0x94+(x-1));
			LCD_PICO_CMDi2c(m,0);
			break;

		case fila4:
      m=(0xd4+(x-1));
			LCD_PICO_CMDi2c(m,0);
			break;
	}

}

//Funcion para enviar una cadena de caracteres al LCD por i2c

void LCD_PICO_PRINT_STRINGi2c(const char *str){

	while(*str){
		 LCD_PICO_CMDi2c(*str++,1);           // envio hasta terminar la cadena , RS = 1 escribo datos
	   }
    
}

//Funcion para crear los caracteres personalizados en el LCD memoria CGRAM 

void LCD_PICO_New_Chari2c(uint8_t a,uint8_t b,uint8_t c,uint8_t d ,uint8_t e ,uint8_t f,uint8_t g,uint8_t h,uint8_t i){

	  uint8_t cgram;
/*

address de la CGRam para almacenar los caracteres	0x40 ----> 7F (cada 8Bytes)			
									     = 56(Dec) valor inicial se va desplazando cada 56+(8*1)=64(Dec) = 0x40(primer banco de memoria CGRAM)										   	
0x40 ---> 0x47  0100-0000 ---> 0100-0111 = 64(Dec)= 0x40 primer banco de memoria de la CGRAM
0x48 ---> 0x4F	0100-1000 ---> 0100-1111 = 72(Dec)= 0x48 Segundo banco de memoria de la CGRAM
0x50 ---> 0x57  0101-0000 ---> 0101-0111 = 80(Dec)= 0x50 Tercer banco de memoria de la CGRAM
0x58 ---> 0x5F
0x60 ---> 0x67
0x68 ---> 0x6F
0x70 ---> 0x77
0x78 ---> 0x7F  0111-1000 ---> 0111-1111 = 120(Dec)= 0x78 Octavo banco de memoria de la CGRAM
			*/

	cgram = 56 + (8*a);
	LCD_PICO_CMDi2c(cgram,0);       // enviamos el comando con la direccion para almacenar el caracter, RS = 0 (comando)
			   
	LCD_PICO_CMDi2c(b,1);
    LCD_PICO_CMDi2c(c,1);
    LCD_PICO_CMDi2c(d,1);
    LCD_PICO_CMDi2c(e,1);
    LCD_PICO_CMDi2c(f,1);
    LCD_PICO_CMDi2c(g,1);
    LCD_PICO_CMDi2c(h,1);
    LCD_PICO_CMDi2c(i,1);

	LCD_PICO_CMDi2c(0x00,0);
    LCD_PICO_CMDi2c(0x01,0);
	sleep_ms(100);			   

 }

void LCD_PICO_Print_New_Char(uint8_t a){
	   LCD_PICO_CMDi2c(a-1,1);	
}

void LCD_PICO_Clear(void){
	   LCD_PICO_CMDi2c(0x01,0);
}

void LCD_PICO_Home(void){
	   LCD_PICO_CMDi2c(0x02,0);
}
 