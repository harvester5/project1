#include <mega8515.h>
#include <delay.h>
#include <stdlib.h>
#include <string.h>
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
//unsigned char _hsec;

char light_on, light_off, air_on, air_off, dinner, xtemp, duration;

#define MAX_DEVICES 8
unsigned char rom_codes[1][9];

char outstr[]={'-','-',':','-','-',':','-','-',' ',' '};
char* outstr2;
unsigned char soff[] ={'O','F','F',':',' ',0};
unsigned char set_hour_on[] ={'S','e','t',' ','h','o','u','r',' ','O','N',':',' ',0};
eeprom int ee_temp;      // уставка температуры
eeprom int ee_light_on;  // время включить лампу
eeprom int ee_light_off; // время выключить лампу
eeprom int ee_air_on;    // время включить компрессор
eeprom int ee_air_off;   // время выключить компрессор
eeprom int ee_dinner;    // время дать пожрать рыбкам
eeprom int ee_duration;  // продолжительность кормления

void beep()
{  PORTB.0=1;
   delay_ms(200);
   PORTB.0=0; 
}

int GetKey(char temp2, char maxx)
{
char temp;
    temp=temp2;                                                                 
    lcd_gotoxy(13,1);
    _lcd_write_data(0b00001110);          // включить мигающий курсор                 
    if (temp>199) 
       { temp -= 200;
         lcd_putchar('2'); }
    else if (temp>99)
      { temp -= 100;
        lcd_putchar('1'); }     
         
    lcd_putchar(0x30 + temp/10);
    lcd_putchar(0x30 + temp%10);
     lcd_putchar(' ');
    lcd_gotoxy(14,1);
    _lcd_write_data(0b00001110);          // включить мигающий курсор            
    
    beep();    
    while  ((PINC & (1<<PINC0))!=0){
#asm
    wdr
#endasm     
        
    if ((PINC & (1<<PINC3))==0)   {beep(); temp2++; if (temp2==maxx) temp2=0; }   
    else 
    if ((PINC & (1<<PINC2))==0)   {beep(); if (temp2==0) temp2=maxx; temp2--; }
    else
    if ((PINC & (1<<PINC1))==0)   {beep(); return 0xFF;}  // юзер передумал..      
    else goto proskok; 
    temp=temp2;        
    lcd_gotoxy(13,1);            
    if (temp>199) 
       { temp -= 200;
         lcd_putchar('2'); }
    else if (temp>99)
      { temp -= 100;
        lcd_putchar('1'); }     
         
    lcd_putchar(0x30 + temp/10);
    lcd_putchar(0x30 + temp%10);
    lcd_putchar(' ');
proskok:
    lcd_gotoxy(14,1);
    _lcd_write_data(0b00001110);          // включить мигающий курсор             
    
    delay_ms(200);
    }
    _lcd_write_data(0b00001100);  // выключить мигающий курсор 
    return temp2;

}

//процедура настроечного меню 
void Menu(){
char ex;
char tmp;
  lcd_clear();
  lcd_puts("Set hours:    ");    
  tmp=GetKey(_hour,24);
  if (tmp==0xFF) return;
  _hour=tmp;
   delay_ms(300);        
  lcd_gotoxy(0,0);
  lcd_puts("Set minutes:    ");    
  tmp=GetKey(_min,60);                
  if (tmp==0xFF) return;
  _min=tmp;
  
  rtc_set_time(_hour, _min, _sec);
   delay_ms(300); 
//процедура установки времени работы компрессора          
  lcd_gotoxy(0,0);  
  lcd_puts("Set time air");
  delay_ms(300);
  lcd_gotoxy(1,1);
  lcd_puts(set_hour_on);
  ex=air_on;    
  tmp=GetKey(ex,24);
  if (tmp==0xFF) return; 
//  ee_air_on=tmp;
  air_on=tmp;
  
  delay_ms(300);     
  lcd_gotoxy(9,1);
  lcd_puts(soff);
  ex=air_off;  
  tmp=GetKey(ex,24);
  if (tmp==0xFF) return;
  //ee_air_off=tmp;      
  air_off=tmp;
  
// процедура устанока времени работы освещения
  lcd_gotoxy(0,0);
  lcd_puts("Set light     ");
  lcd_gotoxy(0,1);
  lcd_puts(set_hour_on);
  tmp=GetKey(light_on,24);
  if (tmp==0xFF) return;
  light_on=tmp;
//  ee_light_on=tmp;
  delay_ms(300);
                 
  lcd_gotoxy(9,1);
  lcd_puts(soff);
  tmp=GetKey(light_off,24);
  if (tmp==0xFF) return;
  light_off=tmp;
//  ee_light_off=tmp;  
  delay_ms(300);
    
// процедура установки работы кормежки  
  lcd_gotoxy(0,0);
  lcd_puts("Set eat     ");
  lcd_gotoxy(0,1);
  lcd_puts(set_hour_on);

  dinner=GetKey(dinner,24);
//  ee_dinner=dinner;
  
  delay_ms(300);
      
  lcd_gotoxy(0,0);
  lcd_puts("Set duration    ");
  lcd_gotoxy(0,1);
  lcd_puts("Set seconds     ");  

  duration=GetKey(duration,24);
//  ee_duration=duration;
  
  delay_ms(300);  
    
// процедура установки работы нагревателя  
  lcd_gotoxy(0,0);
  lcd_puts("Set temp     ");
  lcd_gotoxy(0,1);          
  lcd_puts("             ");
  xtemp=ee_temp;
  ee_temp=GetKey(xtemp,360);       
}
 
void main(void)
{
    PORTA=0x00;DDRA=0x00; PORTB=0x01;DDRB=0x01; PORTC=0x0F;DDRC=0xF0; PORTD=0x00;DDRD=0b11111000; PORTE=0x00;DDRE=0x00;
    TCCR0=0; 
    delay_ms(100);
    chip = w1_search(0xf0, rom_codes);
        
    rtc_init(1,1,1); 
    lcd_init(16);
    rtc_get_time(&_hour, &_min, &_sec);
    rtc_set_time(_hour, _min, 0);
   
    lcd_puts("-Aquacontroller-");
    if (chip ==0)       lcd_puts("No temp sensors!"); 
                else    lcd_puts("-- by Dolotoff--");
//    *outstr2 = ' ',' ',' ',' ',' ',' ';
   delay_ms(500);
// грузим константы..
    if (ee_light_off != 255) {light_off=ee_light_off;} else { light_off=22; }           
    if (ee_air_on != 255)    {air_on=ee_air_on;}       else { air_on=07; }   
    if (ee_air_off != 255)   {air_off=ee_air_off;}     else { air_off=13; }   
    if (ee_light_on !=255)  {light_on=ee_light_on;}   else { light_on=17; }  
    if (ee_dinner != 255)    {dinner=ee_dinner;}       else  { dinner=17;  }
    if (ee_duration != 255)   {duration=ee_duration;}   else { duration=4; }
    if (ee_temp>200) ee_temp=30;
    xtemp=19; //ee_temp;
                             
while (1)
      {             
#asm
   wdr
#endasm         
// опрос клавиатуры
      if (!(PINC & (1<<PINC0)))  Menu();
      if (!(PINC & (1<<PINC1)))  { PORTD |= 0b10000000; delay_ms(300); PORTD &= 0b01111111;}
//      if (!(PINC & (1<<PINC2)))  { PORTD |= 0b00100000; }  // manual lamp 
//        else { PORTD &= 0b11011111;}
//      if (!(PINC & (1<<PINC3)))  { PORTD |= 0b00010000; }  // manual air 
//        else { PORTD &= 0b11101111; }              
// опрос датчиков      
      rtc_get_time(&_hour, &_min, &_sec);
#asm("cli");     
      Xp=ds1820_temperature_10(&rom_codes[0][0]);  
#asm("sei");
      Xp=Xp / 100;      
      delay_ms(100);
// преобразование данных      
      outstr[0]=_hour/10 +0x30;
      outstr[1]=_hour%10 +0x30;   
      outstr[3]=_min/10  +0x30;
      outstr[4]=_min%10  +0x30;     
      outstr[6]=_sec/10  +0x30;
      outstr[7]=_sec%10  +0x30;
      
      itoa(Xp, outstr2);
    //  outstr2[3]=outstr2[2];
    //  outstr2[2]=',';
    //  outstr2[4]='C';
    //  outstr2[5]=0;
// блок вывода на экран
      lcd_gotoxy(0,1);
      lcd_puts("                ");   // clear the line
      lcd_gotoxy(0,1);
      lcd_puts(outstr);
      lcd_putchar(0xfa);                      
      lcd_puts(outstr2);
      lcd_putchar('C');       
// блок проверки условий работы периферии c учетом того что MOC61 инверсная      
      if (((_hour>=light_on) & (_hour < light_off )) | (PINC.2==0))   
              { PORTD |= 0b00100000; }  
              else 
              { PORTD &= 0b11011111; }
      if (((_hour>=air_on) & (_hour<air_off)) | (PINC.3==0))          
              {PORTD |= 0b00010000; }  
              else 
              {PORTD &= 0b11101111; }
      if ( Xp < xtemp )     PORTD &= 0b11110111;    else {PORTD |= 0b00001000;};                   
      if ((_hour==dinner) & !(_min) & (_sec<duration))   {PORTD |= 0b10000000; beep();}  
                                                     else PORTD &= 0b01111111;
 
      
      lcd_gotoxy(0,0);
      if ((PORTD&0b00100000)!=0) lcd_puts ("lamp "); else lcd_puts("     ");
      lcd_gotoxy(5,0);
      if ((PORTD&0b00010000)!=0) lcd_puts ("air  "); else lcd_puts("     ");
      lcd_gotoxy(10,0);
      if ((PORTD&0b00001000)==0) lcd_puts ("heat "); else lcd_puts("     ");                          
      }
}
