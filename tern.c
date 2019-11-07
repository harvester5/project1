
#include <mega8515.h>

#asm
   .equ __i2c_port=0x07 ;PORTE
   .equ __sda_bit=2
   .equ __scl_bit=1
   .equ __w1_port=0x07 ;PORTE
   .equ __w1_bit=0  
#endasm
//#include <1wire.h>
#include <ds1820.h>
#include <alcd.h>

#define MAX_DEVICES 8
unsigned char rom_codes[MAX_DEVICES][9];
#define xtal 8000000L

void main(void)
{
    char chip;
    char temp;
//    w1_init();
    chip = w1_search(0xf0, rom_codes);       
    lcd_init(16);

    lcd_puts("-Aquacontroller-");
    if (chip ==0)       lcd_puts("No temp sensors!"); 
                else    lcd_puts("-- by Dolotoff--");
while (1)
      {/*
    temp=ds1820_temperature_10(&rom_codes[0][0]);
    lcd_putchar(temp/10+0x30);
    lcd_putchar(temp%10+0x30);    */
      }
}
