#include <WProgram.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <SoftwareSerial.h>


SoftwareSerial sLCD =  SoftwareSerial(3, 6); /* Serial LCD is connected on pin 14 (Analog input 0) */

#define COMMAND 0xFE
#define CLEAR   0x01
#define LINE0   0x80
#define LINE1   0xC0
#define COMMAND2 0x7C


int a,b,c,d;


static FILE lcdout = {0};

static int lcd_putchar(char ch, FILE* stream)
{
  sLCD.write(ch);
  return 0;
}

void setup() { 
  Serial.begin(9600);
  Serial.println("ECU Reader");  /* For debug use */
  sLCD.begin(9600);              /* Setup serial LCD and clear the screen */
  clear_lcd();
  delay(10); 

  /* setup the stream for the lcd to be able to use printf formating */ 
  fdev_setup_stream( &lcdout, lcd_putchar, NULL, _FDEV_SETUP_WRITE); 
  sLCD.write(COMMAND2);  /* set the display backlight to max */
  sLCD.write(157);
  Serial.println("initializing can");
  while (!Canbus.init(CANSPEED_500)) 
  {
    sLCD.print("Can init fucked");
    delay(1000);
  }
  Serial.println("Can init ok");
  sLCD.print("Can Init ok");
  delay(1000); 
  clear_lcd();
}
 

void loop() {
  Canbus.read_msg(a,b,c,d);    
  sLCD.write(COMMAND);
  sLCD.write(LINE0);
  fprintf(&lcdout, "OiT %3d OiP %3d",a-50,b);
  sLCD.write(COMMAND);
  sLCD.write(LINE1);
  fprintf(&lcdout, "AFR %3d Fup %3d",c/10,d);
  print_to_serial(a,b,c,d);
  delay(100); 
}

void print_to_serial( int a, int b, int c, int d)
{
  Serial.print(a);
  Serial.print(",");
  Serial.print(b);
  Serial.print(",");
  Serial.print(c);
  Serial.print(",");
  Serial.println(d);
}

void clear_lcd(void)
{
  sLCD.write(COMMAND);
  sLCD.write(CLEAR);
}  
