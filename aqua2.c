#include <mega8515.h>                                       //        программа для акваконтроллера
#include <delay.h>                                          //        переписана для 1х16 дисплея
#include <stdlib.h>                                         //
#include <string.h>                                         //
#asm   
   .equ __w1_port=0x15 ;PORTC
   .equ __w1_bit=7    
   
   .equ __ds1302_port=0x07  ;PORTE
   .equ __ds1302_io=1
   .equ __ds1302_sclk=2
   .equ __ds1302_rst=0     
#endasm
#include <ds1820.h>
 
#include <ds1302.h>
#include <alcd.h>

int Xp;
unsigned char chip;
unsigned char _hour;
unsigned char _min;
unsigned char _sec;

#define MAX_DEVICES 8
unsigned char rom_codes[1][9];

char* outstr2;                       //  универсальная строковая переменная
char outstr[9];
void convert() {
     // преобразование данных      
      outstr[0]=_hour/10 +0x30;
      outstr[1]=_hour%10 +0x30;   
      outstr[3]=_min/10  +0x30;
      outstr[4]=_min%10  +0x30;     
      outstr[6]=_sec/10  +0x30;
      outstr[7]=_sec%10  +0x30;            
}

char GetKey(char temp2, char maxx)
{
char temp;
    temp=temp2;                                                                 
// первоначальная отрисовка числа    
    lcd_gotoxy(13,1);
    _lcd_write_data(0b00001110);          // включить мигающий курсор                 
    if (temp>199)          { temp -= 200;       lcd_putchar('2'); }
    else if (temp>99)     { temp -= 100;       lcd_putchar('1'); }     
         
    lcd_putchar(0x30 + temp/10);
    lcd_putchar(0x30 + temp%10);
     lcd_putchar(' ');
    lcd_gotoxy(14,1);
    _lcd_write_data(0b00001110);          // включить мигающий курсор                    
    beep(0);    
// циклическое изменение числа в ответ на кнопки    
    while  ((PINC & (1<<PINC0))!=0){
    if ((PINC & (1<<PINC3))==0)   {beep(1); temp2++; if (temp2==maxx) temp2=0; }   
      else 
      if ((PINC & (1<<PINC2))==0)   {beep(1); if (temp2==0) temp2=maxx; temp2--; }
        else
        if ((PINC & (1<<PINC4))==0)   {beep(0); return 0xFF;}  // юзер передумал..      
          else goto proskok; 
    temp=temp2;        
    lcd_gotoxy(13,1);            
    if (temp>199)           { temp -= 200;         lcd_putchar('2'); }
    else if (temp>99)      { temp -= 100;        lcd_putchar('1'); }     
             
    lcd_putchar(0x30 + temp/10);
    lcd_putchar(0x30 + temp%10);
    lcd_putchar(' ');
proskok:
   
    delay_ms(200);
    }
    _lcd_write_data(0b00001100);  // выключить мигающий курсор 
    return temp2;

}


void menu() {
char tempstr[];
   GICR &= 0b10111111;                     // откл.прерывание INT0
   itoa(_hour, tempstr)
  _lcd_write_data(0b00001110);          // включить мигающий курсор
   lcd_gotoxy(0,1);
   lcd_puts(tempstr)
   
   itoa(_min, tempstr)
  _lcd_write_data(0b00001110);          // включить мигающий курсор
   lcd_gotoxy(0,1);
   lcd_puts(tempstr)   
   
   GICR |= 0b01000000;                    // вкл.прерывание INT0
}


interrupt [EXT_INT0] void ext_int0_isr(void)
{
   menu();
}


interrupt [EXT_INT1] void ext_int1_isr(void)
{


}

// External Interrupt 2 service routine
interrupt [EXT_INT2] void ext_int2_isr(void)
{
// Place your code here

}


void main(void)
{
    PORTA=0x00;DDRA=0x00; PORTB=0x01;DDRB=0x01; PORTC=0xFF;DDRC=0x80; PORTD=0x00;DDRD=0b11111000; PORTE=0x00;DDRE=0x00;
    TCCR0=0; 
    
    GICR|=0xE0;
    MCUCR=0x0A;

    delay_ms(100);
    chip = w1_search(0xf0, rom_codes);
        
    rtc_init(1,1,1); 
    lcd_init(16);
    rtc_get_time(&_hour, &_min, &_sec);
    rtc_set_time(_hour, _min, 0);
    while(1) {
    
     rtc_get_time(&_hour, &_min, &_sec);
     
     convert();
     
      itoa(Xp, outstr2);
      lcd_ckear();
      lcd_puts(outstr);
    }
}