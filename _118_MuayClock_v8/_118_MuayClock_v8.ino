// Muay Clock v7
// v8 Campanadas multiples
// v7 Anadimos nuevos programas
// v4 incorpora cambio de Hora
// ESTUDIO ROBLE
// www.roble.uno
//based on:
//
//Ninja Timer
//Course timer for Ultimate Ninja Athlete Association course
//by John Park
//for Adafruit Industries
//Timer logic by Tod Kurt
//
////MIT License
////////////////////////////////////////////////////////////////////////////////
#include "DHT.h"
#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;
#include <Adafruit_NeoPixel.h>
#include <TaskScheduler.h>
#include <MemoryFree.h>
#include <pgmStrToRAM.h>
#define NUMPIXELS 63 // # of LEDs in a strip
////////////////////// 88:88
#define DATAPIN4 13 // 8      dm
#define DATAPIN3 12 //  8     m
#define pinDHT 11
#define pinLed 10
#define pinMotor 9
#define DATAPIN2 8  //    8   ds
#define pinEncB 7
#define DATAPIN1 6  //     8  s
#define pinBoton 5
#define DATAPIN0 4  //   :    :
#define pinHall 3
#define pinEncA 2
#define pinTavoz A3
// RTC SDA A4
// RTC 11 SDC A5
DHT dht(pinDHT, DHT11);
Adafruit_NeoPixel strip[] = { //here is the variable for the multiple strips
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN0, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN1, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN3, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN4, NEO_GRB + NEO_KHZ800)
};
const int bright = 255; //brightness for all pixels 0-255 range, 32 being dim
#include <Encoder.h>
Encoder myEnc(pinEncA, pinEncB); // Best Performance: Both signals connect to interrupt pins.
float Position;
float lastPosition = 100;
int EncStep; // Posición del encoder /4
int lastEncStep;
int prog = 1;
#define progCount 10 // Nº de programas
// Programa 1: Hora
// Programas 2-5
unsigned long defProg[progCount][8] = { // progCount - 2
  //{300, 2, 2}  {nAsaltos, tAsalto, tDescanso}
  {20, 5, 5},
  //{10, 45, 15}, // 1
  {15, 40, 20}, // 2
  {20, 30, 10}, // 3
  {3, 180, 60}, // 4
  {5, 30, 30},  // 5
  {5, 180, 60}, // 6
  {5, 240, 60}, // 7
  {5, 300, 120},// 8
  {10, 120, 30} // 9
};
int nAsaltos;
unsigned long tAsalto;
unsigned long tDescanso;
int showProg[progCount][9] = { // progCount - 1
  {4, 5, 1, 5}, // 1
  {4, 0, 2, 0}, // 2
  {3, 0, 1, 0}, // 3
  {0, 3, 0, 1}, // 88:88
  {3, 0, 3, 0}, // 5
  {0, 3, 0, 1}, // 6
  {0, 4, 0, 1}, // 7
  {0, 5, 0, 2}, // 8
  {0, 2, 3, 0}  // 9
  // 4 3 : 2 1
};
int asalto; // Nº de asalto en que nos encontramos. Empieza en 1
int luchando; // 0/1
unsigned long comienzo; // Cogerá los millis() del comienzo
int botonState = 1;
int botonLastState = 1;
volatile int tRunning = 0;
volatile int nbells; // Numero de Campanadas
int tempCada = 4; // Muestra la temp cada Xs
int colorHora = 1;
int colorTemp = 2;
int colorHR = 3;
int setHour = 0;
int setDigit = 0;
// how often the inputs are readInputs
const int inputsMillis = 10;
const int displayMillis = 100;
// task prototypes
Scheduler runner;
// change these repeat times to appropriate ones for real application
Task t1(inputsMillis, TASK_FOREVER, &readBoton);
Task t2(displayMillis, TASK_FOREVER, &updateDisplay);
void setup() { // SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
  Serial.begin(9600);
  dht.begin();
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
  pinMode(pinMotor, OUTPUT);
  pinMode(pinHall, INPUT_PULLUP);
  // DIGITAL PINS USABLE FOR INTERRUPTS
  // Mega, Mega2560, MegaADK  2, 3, 18, 19, 20, 21
  // Micro, Leonardo, other 32u4-based  0, 1, 2, 3, 7
  attachInterrupt(digitalPinToInterrupt(pinHall), hall, FALLING);
  //NeoPixel array setup
  for (int s = 0; s < 5; s++) {
    strip[s].begin(); // Initialize pins for output
    strip[s].setBrightness(bright); //full brightness 255
    strip[s].show();  // Turn all LEDs off
    delay(200);
  }
  runner.init();
  runner.addTask(t1);
  t1.enable();
  runner.addTask(t2);
  t2.enable();
  myEnc.write(0);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  /* if (rtc.lostPower()) {
     Serial.println("RTC lost power, lets set the time!");
     // following line sets the RTC to the date & time this sketch was compiled
     //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
     // This line sets the RTC with an explicit date & time, for example to set
     // January 21, 2014 at 3am you would call:
     rtc.adjust(DateTime(2018, 12, 2, 12, 13, 0));
    }*/
  // Prueba colores
  /* for (int j = 0; j <= 10; j++) {
     for (int i = 0; i <= 10; i++) {
       digitWrite(4, i, i);
       strip[4].show();
       delay(2000);
     }
    }*/
}
void bell() {
  analogWrite(pinMotor, 80);
  //tone(pinTavoz, 1000, 800);
  // Serial.println("---->");
}
void hall() {
  Serial.println(nbells);
  if (nbells == 1) {
    analogWrite(pinMotor, 80); // Solo queda una campanada. Frenamos motor.
  }
  if (nbells > 0) {
    nbells --;
  } else {
    digitalWrite(pinMotor, LOW);
    // Serial.println("XXXXXX");
  }
}
void loop() { // LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
  runner.execute();
  Position = myEnc.read();
  if (Position != lastPosition) {
    if (tRunning) { // Suena mal si movemos el encoder en tRunning
      tone(pinTavoz, 215, 200);
      lastPosition = Position;
    } else {
      if (Position >= 0) { // Para que no quede al borde
        EncStep = (Position + 1) / 4;
      } else {
        EncStep = (Position - 1) / 4;
      }
      if (EncStep != lastEncStep) {
        if (!setHour) {
          if (EncStep > lastEncStep) { // +PROG
            tone(pinTavoz, 443, 100);
            prog++;
            if (prog > progCount) {
              prog = 1;
            }
          }
          if (EncStep < lastEncStep) { // -PROG
            tone(pinTavoz, 415, 100);
            prog--;
            if (prog == 0) {
              prog = progCount;
            }
          }
        } else { // estamos cambiando la hora
          DateTime now = rtc.now();
          if (EncStep > lastEncStep) { // sube
            tone(pinTavoz, 443, 100);
            switch (setDigit) {
              case 1:
                if (now.minute() % 10 != 9) {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 1, now.second()));
                } else {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() - 9, now.second()));
                }
                break;
              case 2:
                if (now.minute() / 10  < 5) {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 10, now.second()));
                } else {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() - 50, now.second()));
                }
                break;
              case 3:
                if (now.hour() % 10  != 9) {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 1, now.minute(), now.second()));
                } else {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 9, now.minute(), now.second()));
                }
                break;
              case 4:
                if (now.hour() % 10 > 3) { // Las decenas de hora solo pueden ser 0 o 1
                  if (now.hour() / 10  == 0) {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 10, now.minute(), now.second()));
                  } else {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 10, now.minute(), now.second()));
                  }
                } else { // Las decenas de hora solo pueden ser 0, 1 o 2
                  if (now.hour() / 10  == 0 || now.hour() / 10  == 1) {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 10, now.minute(), now.second()));
                  } else {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 20, now.minute(), now.second()));
                  }
                }
                break;
            }
          }
          if (EncStep < lastEncStep) { // baja
            tone(pinTavoz, 415, 100);
            switch (setDigit) {
              case 1:
                if (now.minute() % 10 > 0) {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() - 1, now.second()));
                } else {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 9, now.second()));
                }
                break;
              case 2:
                if (now.minute() / 10 > 0) {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() - 10, now.second()));
                } else {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 50, now.second()));
                }
                break;
              case 3:
                if (now.hour() % 10 > 0) {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 1, now.minute() , now.second()));
                } else {
                  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 9, now.minute() , now.second()));
                }
                break;
              case 4:
                if (now.hour() % 10 > 3) { // Las decenas de hora solo pueden ser 0 o 1
                  if (now.hour() / 10  == 0) {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 10, now.minute(), now.second()));
                  } else {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 10, now.minute(), now.second()));
                  }
                } else { // Las decenas de hora solo pueden ser 0, 1 o 2
                  if (now.hour() / 10  == 2 || now.hour() / 10  == 1) {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 10, now.minute(), now.second()));
                  } else {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 20, now.minute(), now.second()));
                  }
                }
                break;
            }
          }
        }
      }
    }
    lastEncStep = EncStep;
    lastPosition = Position;
  }
}
void updateDisplay() { //  *************************************************************************************************
  DateTime now = rtc.now();
  switch (prog) {
    case 1: // Reloj
      if (!tRunning && !setHour) { // Sin esto, aparece la hora cuando llegas a prog 1 con el encoder
        if ((now.second() + 1) % tempCada == 0) { // temp cada 3s
          // Read temperature as Celsius (the default)
          int t = dht.readTemperature();
          digitWrite(4, t / 10, colorTemp);
          digitWrite(3, t % 10, colorTemp);
          digitWrite(0, 1, 1);
          // El º  de ºC
          segLight(2, 1, colorTemp);
          segLight(2, 2, colorTemp);
          segLight(2, 3, colorTemp);
          segLight(2, 4, 0);
          segLight(2, 5, 0);
          segLight(2, 6, 0);
          segLight(2, 7, colorTemp);
          // El C  de ºC
          segLight(1, 1, colorTemp);
          segLight(1, 2, colorTemp);
          segLight(1, 3, 0);
          segLight(1, 4, 0);
          segLight(1, 5, colorTemp);
          segLight(1, 6, colorTemp);
          segLight(1, 7, 0);
          strip[4].show();
          strip[3].show();
          strip[2].show();
          strip[1].show();
        }
        if (now.second() % tempCada  == 0) { // HR 1s después de tª
          int h = dht.readHumidity();
          digitWrite(4, h / 10, colorHR);
          digitWrite(3, h % 10, colorHR);
          digitWrite(0, 1, 1);
          // H de HR
          segLight(2, 1, colorHR);
          segLight(2, 2, 0);
          segLight(2, 3, colorHR);
          segLight(2, 4, colorHR);
          segLight(2, 5, 0);
          segLight(2, 6, colorHR);
          segLight(2, 7, colorHR);
          // R de HR
          segLight(1, 1, 0);
          segLight(1, 2, 0);
          segLight(1, 3, 0);
          segLight(1, 4, 0);
          segLight(1, 5, 0);
          segLight(1, 6, colorHR);
          segLight(1, 7, colorHR);
          strip[4].show();
          strip[3].show();
          strip[2].show();
          strip[1].show();
        }
        if (now.second() % tempCada  != 0 && (now.second() + 1) % tempCada  != 0) { // ni temp ni hr, hora
          if (now.hour() / 10 != 0) {
            digitWrite(4, now.hour() / 10, colorHora);
          } else {
            blank(4);
          }
          digitWrite(3, now.hour() % 10, colorHora);
          digitWrite(2, now.minute() / 10, colorHora);
          digitWrite(1, now.minute() % 10, colorHora);
          if (now.second() % 2) { // parpadeo segundos
            digitWrite(0, 0, colorHora);
          } else {
            digitWrite(0, 1, colorHora);
          }
          // MUESTRA
          strip[4].show();
          strip[3].show();
          strip[2].show();
          strip[1].show();
          strip[0].show();
        }
      }
      if (setHour) {
        digitWrite(4, now.hour() / 10, 1);
        digitWrite(3, now.hour() % 10, 1);
        digitWrite(2, now.minute() / 10, 1);
        digitWrite(1, now.minute() % 10, 1);
        digitWrite(0, 0, 1);
        switch (setDigit) { // El que estamos cambiando ahora... amarillo
          case 1:
            digitWrite(1, now.minute() % 10, 5);
            break;
          case 2:
            digitWrite(2, now.minute() / 10, 5);
            break;
          case 3:
            digitWrite(3, now.hour() % 10, 5);
            break;
          case 4:
            digitWrite(4, now.hour() / 10, 5);
            break;
        }
        strip[4].show();
        strip[3].show();
        strip[2].show();
        strip[1].show();
        strip[0].show();
      }
      break;
    default:
      if (tRunning) {
        int secs = ((millis() - comienzo) / 1000) % 60;
        int mins = ((millis() - comienzo) / 1000 / 60) % 60 ;
        int secsTens = secs / 10; //get the tens place of seconds
        int secsOnes = secs % 10; //get the ones place of seconds
        int minsTens = mins / 10; //get the tens place of minutes
        int minsOnes = mins % 10; //get the ones place of minutes
        if (luchando) {
          digitWrite(0, 0, 1); // Pone :
          strip[0].show();
          digitWrite(1, secsOnes, 1);
          strip[1].show();
          digitWrite(2, secsTens, 1);
          strip[2].show();
          digitWrite(3, minsOnes, 1);
          strip[3].show();
          if (asalto < 10) {
            digitWrite(4, asalto % 10, 5);
          } else {
            digitWrite(4, asalto % 10, 7);
          }
          strip[4].show();
        } else {
          digitWrite(0, 0, 2); // Pone :
          strip[0].show();
          digitWrite(1, secsOnes, 2);
          strip[1].show();
          digitWrite(2, secsTens, 2);
          strip[2].show();
          digitWrite(3, minsOnes, 2);
          strip[3].show();
          digitWrite(4, asalto % 10, 5);
          if (asalto < 10) {
            digitWrite(4, asalto % 10, 5);
          } else {
            digitWrite(4, asalto % 10, 7);
          }
          strip[4].show();
        }
      } else { // PARADO
        digitWrite(0, 1, 1); // Quita :
        strip[0].show();
        // Mostramos Programa
        if (showProg[prog - 2][0] != 0) {
          digitWrite(4, showProg[prog - 2][0], 1);
          strip[4].show();
        } else {
          blank(4);
        }
        if (showProg[prog - 2][2] != 0) {
          digitWrite(2, showProg[prog - 2][2], 2);
          strip[2].show();
        } else {
          blank(2);
        }
        digitWrite(3, showProg[prog - 2][1], 1);
        digitWrite(1, showProg[prog - 2][3], 2);
        strip[1].show();
        strip[3].show();
      }
      break;
  }
}
void readBoton() {
  botonState = digitalRead(pinBoton);
  if (botonState != botonLastState) {
    if (botonState == LOW) { //Botón pulsado
      if (prog == 1) { // en hora... Entramos/Estamos en cambio de hora
        if (setDigit == 0) { // Entramos ahora
          setHour = 1;
          setDigit = 1;
          digitalWrite(pinLed, HIGH);
        } else {
          if (setDigit == 4) {
            setHour = 0; // salimos del cambio de hora
            setDigit = 0;
            DateTime now = rtc.now();
            rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), 0));
            digitalWrite(pinLed, LOW);
            blank(0);
          } else {
            setDigit ++;
          }
        }
      } else { // estamos en programas
        if (tRunning) { // Paramos
          tRunning = 0;
          digitalWrite(pinLed, LOW);
        } else { // Nos ponemos en marcha
          tRunning = 1;
          nbells = 9;
          bell();
          digitalWrite(pinLed, HIGH);
          nAsaltos = defProg[prog - 2][0];
          Serial.println(nAsaltos);
          tAsalto = defProg[prog - 2][1] * 1000;
          tDescanso = defProg[prog - 2][2] * 1000;
          asalto = 1;
          luchando = 1;
          comienzo = millis();
        }
      }
    } else {
      // Serial.println("Soltado");
    }
    botonLastState = botonState;
  } else { // Si no pulsamos el boton
    if (tRunning) { // Actualizamos Cronómetro
      if (luchando) {
        if (millis() - comienzo > tAsalto) { // Se acabó el asalto
          if (asalto == nAsaltos) { // Era el último asalto. Se para. No hay descanso en el último asalto.
            tRunning = 0;
            digitalWrite(pinLed, LOW);
            nbells = 9;
            bell();
          } else {
            comienzo = millis(); // Comienza el descanso
            luchando = 0;
            nbells = 4;
            bell();
          }
        }
      } else { // estamos descansando
        if (millis() - comienzo > tDescanso) { // Se acabó el descanso
          nbells = 4;
          bell();
          asalto ++;
          luchando = 1;
          comienzo = millis();
        }
      }
    }
  }
}
void digitWrite(int digit, int val, int col) { // digit> 43:21
  //use this to light up a digit
  //digit is which digit panel one (right to left, 0 indexed)
  //val is the character value to set on the digit
  //col is the predefined color to use, R,G,B or W
  //example:
  //        digitWrite(0, 4, 2);
  //would set the first digit
  //on the right to a "4" in green.
  /*
    // Letters are the standard segment naming, as seen from the front,
    // numbers are based upon the wiring sequence
            A 2
       ----------
      |          |
      |          |
    F 1          | B 3
      |          |
      |     G 7  |
       ----------
      |          |
      |          |
    E 6          | C 4
      |          |
      |     D 5  |
       ----------
  */
  //these are the digit panel character value definitions,
  //if color argument is a 0, the segment is off
  if (val == 0) {
    //segments F,A,B,C,D,E,G, dp
    segLight(digit, 1, col);
    segLight(digit, 2, col);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, col);
    segLight(digit, 6, col);
    segLight(digit, 7, 0);
    segLight(digit, 8, col);
  }
  if (val == 1) {
    segLight(digit, 1, 0);
    segLight(digit, 2, 0);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, 0);
    segLight(digit, 6, 0);
    segLight(digit, 7, 0);
    segLight(digit, 8, col);
  }
  if (val == 2) {
    segLight(digit, 1, 0);
    segLight(digit, 2, col);
    segLight(digit, 3, col);
    segLight(digit, 4, 0);
    segLight(digit, 5, col);
    segLight(digit, 6, col);
    segLight(digit, 7, col);
    segLight(digit, 8, 0);
  }
  if (val == 3) {
    segLight(digit, 1, 0);
    segLight(digit, 2, col);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, col);
    segLight(digit, 6, 0);
    segLight(digit, 7, col);
    segLight(digit, 8, col);
  }
  if (val == 4) {
    segLight(digit, 1, col);
    segLight(digit, 2, 0);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, 0);
    segLight(digit, 6, 0);
    segLight(digit, 7, col);
    segLight(digit, 8, col);
  }
  if (val == 5) {
    segLight(digit, 1, col);
    segLight(digit, 2, col);
    segLight(digit, 3, 0);
    segLight(digit, 4, col);
    segLight(digit, 5, col);
    segLight(digit, 6, 0);
    segLight(digit, 7, col);
    segLight(digit, 8, col);
  }
  if (val == 6) {
    segLight(digit, 1, col);
    segLight(digit, 2, col);
    segLight(digit, 3, 0);
    segLight(digit, 4, col);
    segLight(digit, 5, col);
    segLight(digit, 6, col);
    segLight(digit, 7, col);
    segLight(digit, 8, col);
  }
  if (val == 7) {
    segLight(digit, 1, 0);
    segLight(digit, 2, col);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, 0);
    segLight(digit, 6, 0);
    segLight(digit, 7, 0);
    segLight(digit, 8, col);
  }
  if (val == 8) {
    segLight(digit, 1, col);
    segLight(digit, 2, col);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, col);
    segLight(digit, 6, col);
    segLight(digit, 7, col);
    segLight(digit, 8, col);
  }
  if (val == 9) {
    segLight(digit, 1, col);
    segLight(digit, 2, col);
    segLight(digit, 3, col);
    segLight(digit, 4, col);
    segLight(digit, 5, col);
    segLight(digit, 6, 0);
    segLight(digit, 7, col);
    segLight(digit, 8, col);
  }
}
void blank(int digit) {
  for (int i = 0; i < 71; i++) {
    strip[digit].setPixelColor(i, 0, 0, 0);
  }
  strip[digit].show();
}
void segLight(char digit, int seg, int col) {
  //use this to light up a segment
  //digit picks which neopixel strip
  //seg calls a segment
  //col is color
  int color[3];
  //color sets
  if (col == 0) { //off
    color[0] = {0};
    color[1] = {0};
    color[2] = {0};
  }
  if (col == 1) { //red
    color[0] = {255};
    color[1] = {0};
    color[2] = {0};
  }
  if (col == 2) { //green
    color[0] = {0};
    color[1] = {255};
    color[2] = {0};
  }
  if (col == 3) { //blue
    color[0] = {0};
    color[1] = {0};
    color[2] = {255};
  }
  if (col == 4) { //white -- careful with this one, 3x power consumption
    //if 255 is used
    color[0] = {100};
    color[1] = {100};
    color[2] = {100};
  }
  if (col == 5) { //yellow
    color[0] = {220};
    color[1] = {150};
    color[2] = {0};
  }
  if (col == 6) { //
    color[0] = {0};
    color[1] = {150};
    color[2] = {150};
  }
  if (col == 7) { //
    color[0] = {150};
    color[1] = {0};
    color[2] = {150};
  }
  if (col == 8) { //
    color[0] = {220};
    color[1] = {50};
    color[2] = {50};
  }
  if (col == 9) { //
    color[0] = {200};
    color[1] = {100};
    color[2] = {0};
  }
  if (col == 10) { //
    color[0] = {0};
    color[1] = {50};
    color[2] = {200};
  }
  //seg F
  if (seg == 1) {
    //light first 8
    for (int i = 0; i < 9; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg A
  if (seg == 2) {
    //light second 8
    for (int i = 9; i < 18; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg B
  if (seg == 3) {
    for (int i = 18; i < 26; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg C
  if (seg == 4) {
    for (int i = 26; i < 36; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg D
  if (seg == 5) {
    for (int i = 36; i < 45; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg E
  if (seg == 6) {
    for (int i = 45; i < 54; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg G
  if (seg == 7) {
    for (int i = 54; i < 63; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
  //seg dp
  if (seg == 8) {
    for (int i = 70; i < 71; i++) {
      strip[digit].setPixelColor(i, color[0], color[1], color[2]);
    }
  }
}
