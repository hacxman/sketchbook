#include <stdint.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include <Encoder.h>

#define SSD1306_96_64
#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

#define ROT_LEFT 9
#define ROT_RIGHT 7
#define ROT_PUSH 10

#define NOT_AN_INTERRUPT -1
volatile int encoder0Pos = 0;
volatile int button_state = 0;
volatile int old_button_state = 0;


//digpots
const int slaveSelectPinA = 5;
const int slaveSelectPinB = 6;
const int shutdownPin = 7;
const int wiper0writeAddr = B00000000;
const int wiper1writeAddr = B00010000;

void doRot() {
//  if (digitalRead(ROT_LEFT) == digitalRead(ROT_RIGHT)) {
//    encoder0Pos++;
//  } else {
//    encoder0Pos--;
//  }


  if (digitalRead(ROT_LEFT) == HIGH) {   // found a low-to-high on channel A
    if (digitalRead(ROT_RIGHT) == LOW) {  // check channel B to see which way
                                             // encoder is turning
      encoder0Pos = encoder0Pos - 1;         // CCW
    } 
    else {
      encoder0Pos = encoder0Pos + 1;         // CW
    }
  }
  else                                        // found a high-to-low on channel A
  { 
    if (digitalRead(ROT_RIGHT) == LOW) {   // check channel B to see which way
                                              // encoder is turning  
      encoder0Pos = encoder0Pos + 1;          // CW
    } 
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }

  }
}

Encoder rotEnc(0, 1);

// Function that printf and related will use to print
int oled_putchar(char c, FILE* f) {
    if (c == '\n') oled_putchar('\r', f);
    return oled.write(c) == 1? 0 : 1;
}

FILE oled_stdout;

void doButton() {
  int v = digitalRead(7);
  old_button_state = button_state;
  button_state = !v;
}

bool button_changed() {
  bool t = old_button_state != button_state;
  old_button_state = button_state;
  return t;
}

void setup() {


    fdev_setup_stream(&oled_stdout, oled_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &oled_stdout;

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  SPI.begin(); 
  SPI.setClockDivider(SPI_CLOCK_DIV2);


  Serial.begin(9600);
}

// This function takes care of sending SPI data to the pot.
void digitalPotWrite(int address, int value, int chip) {
  // take the SS pin low to select the chip:
  digitalWrite(chip == 0 ? slaveSelectPinA : slaveSelectPinB ,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value>255?255:(value<0?0:value));
  // take the SS pin high to de-select the chip:
  digitalWrite(chip == 0 ? slaveSelectPinA : slaveSelectPinB ,HIGH);
//  digitalWrite(slaveSelectPin,HIGH); 
}

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000};
 
 if (precision < 0) precision = 0;
 if (precision >= sizeof(p)/sizeof(p[0])) precision = sizeof(p)/sizeof(p[0])-1;
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}

int t=0;
unsigned int tt=0;
int mode = 0;
char volume = 127;
float f = 1200;
void loop() {
  t++;
//  unsigned char v = analogRead(A2);
  if (t % 1000 == 0) {
    while (Serial.available() > 0) {
      char s = Serial.read();
      if (s&0xf0 == 0x90) {//note ON
        char k = Serial.read();
        volume = Serial.read();
        f = 440*powf(2, (k-0x39)/12);
        f = 440;
      }
      if (s&0xf0 == 0x80) {//note ON
        f = Serial.read();
        volume = Serial.read();
      }

      f = s * 10;
    }
  }

  unsigned char d = 127+volume*sin(float(tt*f*2*3.14)/48000);
  digitalPotWrite(wiper0writeAddr, d & 0xff, 0); // Set wiper 0 to 200
  //digitalPotWrite(wiper0writeAddr, tt&0xff, 0); // Set wiper 1 to 200

  if (tt == 0) tt = 0xff;
  else if (tt == 0xff) tt = 0;
}
