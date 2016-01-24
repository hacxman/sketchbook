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

void setup() {

  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
//  pinMode(ROT_RIGHT, INPUT);
//  digitalWrite(ROT_RIGHT, HIGH);
//  pinMode(ROT_LEFT, INPUT);
//  digitalWrite(ROT_LEFT, HIGH);
//  pinMode(ROT_PUSH, INPUT);
//  digitalWrite(ROT_PUSH, HIGH);
//  attachInterrupt(digitalPinToInterrupt(7), doRot, CHANGE);


  Serial.begin(9600);
 // oled.begin(SSD1306_SWITCHCAPVCC, 0x78);  // initialize with the I2C addr 0x3D (for the 128x64)
  oled.begin();
  oled.clearDisplay();
  oled.setCursor(1,1);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.println("oi!FET meter");
  oled.display();
oled.dim(true);
}

int t=0;
void loop() {
  oled.drawPixel(1,1, WHITE);
  oled.display();
  delay(125);

  oled.drawPixel(1,1, BLACK);
  oled.display();
  delay(125);

  digitalWrite(9, LOW);
  digitalWrite(8, HIGH);
  uint16_t v1=analogRead(A0)*5;
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);
  uint16_t v2=analogRead(A0)*5;
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);

  oled.fillRect(0,0,96,16, BLACK);
  oled.setCursor(2,1);
  oled.print(rotEnc.read()/4, DEC);
  oled.print(" ");
  oled.print(v1, DEC);
  oled.print(" ");
  oled.print(v2, DEC);
  oled.display();

  uint16_t v3 = 0;
  while (Serial.available() > 0) {
    char s = Serial.read();
    oled.println(s);
oled.display();
    if (s == 'A') {
    if (!(v1 == 0 && v2 == 0)) {
      t++;
      if (t>95) t=0;
      oled.drawPixel(t, 32-v2/100, WHITE); 
   }
  

  Serial.write(0x02);
  Serial.write((uint8_t)1|(1<<7));
  Serial.write((uint8_t)0);

  Serial.write("    ");

  uint16_t v=rotEnc.read()/4;
  Serial.write(v>>8);
  Serial.write(v&0xff);
  
  Serial.write(v1>>8);
  Serial.write(v1&0xff);
  Serial.write(v2>>8);
  Serial.write(v2&0xff);
  Serial.write(v3>>8);
  Serial.write(v3&0xff);

  Serial.write("                        ");
  Serial.write((uint8_t)0);
  Serial.write((uint8_t)0);
  Serial.write((uint8_t)0);
  Serial.write((uint8_t)0);
  Serial.write((uint8_t)0);


  Serial.write((uint8_t)3);
}
}
}
