#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <FastLED.h>
 
 
#define NUM_LEDS 60
#define DATA_PIN_LEFT 3
#define CLOCK_PIN_LEFT 4
#define DATA_PIN_RIGHT 6
#define CLOCK_PIN_RIGHT 7
#define CHIPSET APA102

#define MAX_WERTE 15



CRGB leds_left[NUM_LEDS];
CRGB leds_right[NUM_LEDS];


#define DIAMETER 0.07              //Hier den durchmesser des Rades in METER angeben. 70mm entsprechen 0.07m
#define LED_DISTANCE 16666         //Abstand der LEDs zueinander in MIKROMETERN
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
volatile int rpm_count;            //Variable zum speichern der einzelnen umdrehungen, die durch den interrupt hochgezählt werden

struct sensorik {
	int raw;
	int smoothed;
	int threshold;
	unsigned long millisOld;
};


void setup() {
  Serial.begin(9600);
  alpha4.begin(0x70);                     //beginne Kommunikation mit Alphanumeric Display//Clock bei Pin A5//Data bei Pin A4
  attachInterrupt(0, rpm_ISR, FALLING);   //Irgendwas mit Interrupt Service Routine auf Digitalen Pin 2

  FastLED.addLeds<CHIPSET, DATA_PIN_LEFT, CLOCK_PIN_LEFT>(leds_left, NUM_LEDS);
  FastLED.addLeds<CHIPSET, DATA_PIN_RIGHT, CLOCK_PIN_RIGHT>(leds_right, NUM_LEDS);
  FastLED.setBrightness(128);
}


void loop() {
  static float m_per_s;              //static, da sie für die berechnung des lauflichts benötigt wird
  float kmh;                         //geschwindigkeit in kmh
  static float max_kmh = 0;          //Höchstgeschwindigkeit
  static float distance = 0;         //zurückgelegte strecke

  static int LED_position;           //Position des Lauflichtes
  unsigned int incrementTime;        //Zeit, wann die nächste Position kommt
  int fadingTime = 20;                
  static unsigned long microsOld;
  static unsigned long fadingTimeOld;

  static unsigned long measure = 200;
  static unsigned long measureOld = 0;
  static unsigned long timeold = 0;   //Zeitspeicher im main loop
  static int interval = 0;                 //aktualisierungsinterval
  static bool _blink = LOW;                //taktstatus



  //Berechnen der Geschwindigkeit alle 200ms
  if((millis() - measureOld) >= measure) 
  {             
    
    measureOld = millis();
    if(rpm_count > 0)         //wenn zählimpulse kommen, dann berechne
    {
      m_per_s = calculate_m_per_s(rpm_count, DIAMETER);
      kmh = calculate_kmh(m_per_s);
      distance = calculate_distance(distance, DIAMETER, rpm_count);
      max_kmh = calculate_max_kmh(max_kmh, kmh);
      rpm_count = 0;
    }
    else                      //sonst ist kmh = 0 und m_per_s = 0
    {
      kmh = 0;
      m_per_s = 0;
    }
  }
  
  //Anzeigen der Werte am Display  
  if((millis() - timeold) >= interval) //interval entspricht dem aktualisierungsinterval
  {             
    
    timeold = millis();
    
    if(_blink == LOW)   //Blinktaktung
    {
      _blink = HIGH;
    }
    else
    {
      _blink = LOW;
    }    


    
    if (kmh >= 1)                 //wenn schneller/gleich 1 kmh, zeige Geschwindigkeit an...
    {
    display_speed(kmh, 'V', ' '); 
    interval = 500;              //das aktualisierungsinterval bei Fahrt 0.5 sek
    }
    else                          //...sonst (also bei Stillstand) zeige zurückgelegte Strecke und Höchstgeschwindigkeit an
    {
      interval = 3000;            //Wechselintervall zwischen Strecke und Vmax ist 3 sek
      if(_blink == LOW){
        display_speed(distance, 'S', ' ');    //Anzeige von Strecke
      }
      else if(_blink == HIGH){
        display_speed(max_kmh, 'V', 'm');     //Anzeige von Vmax
      }
    }
    
  }

  //LED Steuerung
  
  //hilfe...

  if(m_per_s > 0.01)      //wenn eine Geschwindigkeit vorliegt, soll das lauflicht animiert werden. Musste 0.01 nehmen, da die Variable weiter oben nie wirklich auf 0 gesetzt wird, sondern auf 0.00003 oder ähnliches
  {
    
    incrementTime = LED_DISTANCE/m_per_s;     //Zeit, wann in die nächste LED geschaltet werden soll (in Mikrosekunden)
    //Serial.println(incrementTime);
    //Serial.println(m_per_s,6);
    
    if ((micros() - microsOld) > incrementTime)         //wenn die Zeit zur nächsten LED abgelaufen ist, schreibe die daten in die strips
    {      
      leds_right[LED_position] += CHSV( 255, 255, 255);     //linke LED
      leds_left[LED_position] += CHSV( 255, 255, 255);      //rechte LED
      LED_position++;                                       //erhöhe anschließend den positionswert zur nächsten LED
      microsOld = micros();
      FastLED.show();                                       //zeige nun die animation
      if(LED_position == NUM_LEDS)       //wenn die LED_position die höhe von NUM_LEDS hat (in dem fall 60), dann setze die Position wieder auf 0                 
      {
        LED_position = 0;
      }
    }
    
  }
  
  if((millis() - fadingTimeOld) > fadingTime)           //schweif für das lauflicht
  {                                                     //wenn die zeit zur verringerung der helligkeit ALLER LEDs abgelaufen ist
    for(int x = 0; x < NUM_LEDS; x++){                  //dann verringere die Helligkeit aller LEDs
      leds_right[x].nscale8(180);
      leds_left[x].nscale8(180);
    }
    fadingTimeOld = millis();
    FastLED.show();
  }
 
  
/*
    Serial.print(" rpm_count: ");
    Serial.print(incrementTime);
    Serial.print(" max_kmh:  ");
    Serial.print(max_kmh);
    Serial.print(" m_per_s: ");
    Serial.print(m_per_s,4);
    Serial.print(" kmh: ");
    Serial.println(kmh);
    */
}

