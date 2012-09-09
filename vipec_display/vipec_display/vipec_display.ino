#include <WProgram.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <SoftwareSerial.h>
//#include <PinChangeInt.h>

SoftwareSerial sLCD =  SoftwareSerial(3, 6);

#define COMMAND 0xFE
#define CLEAR   0x01
#define LINE0   0x80
#define LINE1   0xC0
#define COMMAND2 0x7C

#define DOUBLE_CLICK_DELAY 100

#define NR_OF_CHANNELS (sizeof(channel)/sizeof(data_channel))

enum {
  MIN,
  MAX,
  RUNTIME
};

struct data_channel {
  char *name;
  int multiplier;
  int divisor;
  int offset;
  int min_value;
  int max_value;
  int value;
};

int a,b,c,d,current;

int page_type = RUNTIME;


data_channel channel[8] = {
    { "OiT", 1, 1, -50, 9999 },
    { "OiP", 1, 1, 0  , 9999 },
    { "AFR", 1,10, 0  , 9999 },
    { "FuP", 1, 1, 0  , 9999 },
    { "Map", 1, 1, 0, 9999 },
    { "IaT", 1, 1, -50, 9999 },
    { "Ign", 1, 1, -1000  , 9999 },
    { "Vbt", 1, 10, 0  , 9999 }
};


static FILE lcdout = {0};

static int lcd_putchar(char ch, FILE* stream)
{
  sLCD.write(ch);
  return 0;
}


void flip_page () {
  static unsigned int last_click;
  if (millis()-200 > last_click ) {
    current+=4;
    if (current > NR_OF_CHANNELS - 4) current=0;
    last_click = millis();
 }
}

void setup() {
  Serial.begin(9600);
  Serial.println("ECU Reader");  /* For debug use */
  sLCD.begin(9600);              /* Setup serial LCD and clear the screen */
  clear_lcd();
  delay(10);

  pinMode(3, INPUT);
  digitalWrite(3,HIGH);
  attachInterrupt(1,flip_page,RISING);
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
  Serial.println(sizeof(channel));
  Serial.println("is the channel size");
  delay(1000);
  clear_lcd();
  current = 0;
}
void read_channel()
{
  Canbus.read_msg(a,b,c,d);
  if(a*3 < sizeof(channel) ) process_value(a*3,b);
  if(a*3+1 < sizeof(channel) ) process_value(a*3+1,c);
  if(a*3+2 < sizeof(channel) ) process_value(a*3+2,d);
}
void process_value( int ch, int val )
{
  int value = val * channel[ch].multiplier / channel[ch].divisor + channel[ch].offset;
  channel[ch].value = value;
  if (value > channel[ch].max_value ) channel[ch].max_value = value;
  if (value < channel[ch].min_value ) channel[ch].min_value = value;
}
void display_page() {
  static int old_page;
  if(old_page!=current) clear_lcd();
  old_page=current;
  int disp_values[4];
  for ( int i = 0; i < 4; i++ )
  {
    switch (page_type) {
      case MIN:
        disp_values[i] = channel[current+i].min_value;
        break;
      case MAX:
        disp_values[i] = channel[current+i].max_value;
        break;
      default:
        disp_values[i] = channel[current+i].value;
     }
  }
  sLCD.write(COMMAND);
  sLCD.write(LINE0);
  fprintf(&lcdout, "%s %3d %s %3d",channel[current].name,   disp_values[0],
                                   channel[current+1].name, disp_values[1]);
  sLCD.write(COMMAND);
  sLCD.write(LINE1);
  fprintf(&lcdout, "%s %3d %s %3d",channel[current+2].name, disp_values[2],
                                   channel[current+3].name, disp_values[3]);
}

void loop() {
  for(int i=0;i<4;i++ )  read_channel();
  display_page();
  print_to_serial();
  delay(100);
}

void print_to_serial()
{
  Serial.print(millis());
  for( int i = 0; i < sizeof(channel)/sizeof(data_channel) ; i++ ) {
    if(i) Serial.print(",");
    else Serial.print(":");
    Serial.print(channel[i].name);
    Serial.print("=");
    Serial.print(channel[i].value);
  }
  Serial.println();
}

void clear_lcd(void)
{
  sLCD.write(COMMAND);
  sLCD.write(CLEAR);
}

