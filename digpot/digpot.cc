#include <stdint.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include <Encoder.h>
#include <TimerOne.h>

#include <avr/pgmspace.h>

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


int selected_wavetable = 0;

int t=0;
unsigned int tt=0;
int mode = 0;
char volume = 0;
float f = 440;
#define TOP 4000
char playing_voices = 0;
#include "wavetable.h"

void digitalPotWrite(int address, int value, int chip);

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

//Encoder rotEnc(0, 1);

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

const int spinor_count = 8;
struct Spinor {
  int t; //phase of rotating spinor
  int f; //it's frequency
  int a; //it's length
  int decay;
  int m; //it's resulting amplitude after ADSH
} spinors[spinor_count];
int get_spinor_by_f(int f) {
  for (int i=0; i<spinor_count; i++) {
    if (spinors[i].f == f)
      return i;
  }
  return -1;
}
void stop_spinor(int f) {
  int i = get_spinor_by_f(f);
  if (i >= 0) {
    spinors[i].f = 0;
    spinors[i].a = 0;
    spinors[i].m = 0;
    spinors[i].decay = 0;
  }
}
int find_stopped_spinor() {
  return get_spinor_by_f(0);
}
void advance_spinors() {
  for (int i=0; i<spinor_count; i++) {
    spinors[i].t += spinors[i].f;
    if (spinors[i].f > 0)
      spinors[i].decay +=2;

    if ((spinors[i].f >0) && (spinors[i].decay > spinors[i].a + spinors[i].a/4) && (spinors[i].a > 0))
      spinors[i].decay = spinors[i].a+spinors[i].a/4;

    if (spinors[i].f > 0)
      spinors[i].m = (spinors[i].a + (spinors[i].decay < spinors[i].a 
                                        ? -(spinors[i].a-spinors[i].decay)
                                        : (spinors[i].a - spinors[i].decay)));
    if (spinors[i].t >= TOP) spinors[i].t %= TOP;
    //if ((spinors[i].f >0) && (spinors[i].a <= 90) && (spinors[i].a > 0)) spinors[i].a = 90;
  }
}
void callback() {
//  unsigned char _d = char(127+volume*sin(float(tt)*6.28/TOP)); //float(tt*f*2*3.14)/23000));
  int32_t d = 0;
  for (int i=0; i<spinor_count; i++) {
    //d += (spinors[i].a*sin(float(spinors[i].t)*6.28/TOP)); //float(tt*f*2*3.14)/23000));
    d += (spinors[i].m*((signed char)pgm_read_byte_near(
            wavetables + selected_wavetable*samplelenght
            + int((int32_t(spinors[i].t)*samplelenght)/TOP)))); //float(tt*f*2*3.14)/23000));
  }
  int divisor = (playing_voices <= 0? 254: 254*playing_voices);
  d = d/divisor;
//  d /= 128;
  int dd = d > 127 ? 127 : (d < -127 ? -127: d); 
  digitalPotWrite(wiper0writeAddr, (unsigned char)(127+dd) & 0xff, 0);
  advance_spinors();
  t++;
  tt+=f;
  if (tt >= TOP) tt = tt%TOP;
//  if (t && 0b11111 == 0) selected_wavetable++;
//  if (selected_wavetable >= samplecount) selected_wavetable=0;
}

void setup() {


    fdev_setup_stream(&oled_stdout, oled_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &oled_stdout;

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  SPI.begin(); 
  SPI.setClockDivider(SPI_CLOCK_DIV2);


  Timer1.initialize(1000000/TOP);
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
  for (int i= 0; i < spinor_count; i++) {
    spinors[i].f = 0;
    spinors[i].t = 0;
    spinors[i].a = 0;
  }
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

//3928

void loop() {
//  unsigned char v = analogRead(A2);
  if (t >= 10) {
    t = 0;
    cli();
    while (Serial.available() > 0) {
      char s = Serial.read();
      if ((s&0xf0) == 0x90) {//note ON
        char k = Serial.read();
        volume = Serial.read();
        f = 440.0*powf(2, (k-0x39)/12.0);
        int idx = find_stopped_spinor();
        if (idx >= 0) {
          spinors[idx].f = f;
          spinors[idx].a = volume;
          spinors[idx].decay = 0;
//          if (volume -20 <= 0) spinors[idx].a = 1;
        }
        playing_voices++;

        digitalWrite(9, HIGH);
//        f = 440;
      }
      if ((s&0xf0) == 0x80) {//note Off
        playing_voices--;
        char k = Serial.read();
        volume = Serial.read();
        f = 440.0*powf(2, (k-0x39)/12.0);
        stop_spinor(f);
        f = 0;
        volume = 0;
        digitalWrite(9, LOW);
      }
      if ((s&0xf0) == 0xb0) {//controller change
        // so we try to change wavetable number
        char cnt = Serial.read(); // controller id
        char val = Serial.read(); // value
        selected_wavetable = val / 4;
      }

//      f = s * 10;
    }
    sei();
  }

//  f = 440;
//  volume = 100;
//  unsigned char d = char(127+volume*sin(float(tt)*6.28/TOP)); //float(tt*f*2*3.14)/23000));
//  digitalPotWrite(wiper0writeAddr, d & 0xff, 0); // Set wiper 0 to 200
  //digitalPotWrite(wiper0writeAddr, tt&0xff, 0); // Set wiper 1 to 200

//  if (tt == 0) tt = 0xff;
//  else if (tt == 0xff) tt = 0;
}
