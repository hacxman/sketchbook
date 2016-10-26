#include <stdint.h>
#include <math.h>
#include <TimerOne.h>

#define BEAT_LED 4
#define CLOCK_LED 7
#define UART_LED 10

#define BPM_SYNC_PIN 2

#define MIDI_CLOCK 248

volatile long beats = 0;
void sync_pressed() {
  beats = 0;
}

void setup() {

  pinMode(BEAT_LED, OUTPUT);
  pinMode(CLOCK_LED, OUTPUT);
  pinMode(UART_LED, OUTPUT);
  digitalWrite(BEAT_LED, LOW);
  digitalWrite(CLOCK_LED, LOW);
  digitalWrite(UART_LED, HIGH);

  //Timer1.initialize(1000000/TOP);
  //Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
  Serial.begin(32500);
  Serial1.begin(32500);

  pinMode(BPM_SYNC_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BPM_SYNC_PIN), sync_pressed, RISING); 
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

unsigned long old_time=0;
int bpm = 0;
void loop() {
//  unsigned char v = analogRead(A0);
  Serial1.write('a');
  cli();
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    digitalWrite(UART_LED, HIGH);
    Serial1.write(c);
    if ((c == MIDI_CLOCK) || (c == 'k')) {
      // MIDI clock signal, 24 ppqn
      unsigned long t = millis();
      bpm = (t - old_time) / 12;
      old_time = t;
      beats ++;
      digitalWrite(CLOCK_LED, beats % 2 == 0 ? HIGH : LOW);
    }
    digitalWrite(UART_LED, LOW);
  }

  digitalWrite(BEAT_LED, beats % 24 == 0 ? HIGH : LOW);

  sei();

}
