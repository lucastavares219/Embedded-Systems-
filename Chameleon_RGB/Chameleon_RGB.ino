
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <LiquidCrystal_I2C.h>

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

// set to false if using a common cathode LED
#define commonAnode false

// our RGB -> eye-recognized gamma color
byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

LiquidCrystal_I2C lcd(0x27,16,2); 

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);

  if (tcs.begin()) {
    lcd.print("Sensor ativo");
  } else {
    lcd.print("Erro no sensor");
    while (1); // halt!
  }
  
  // use these three pins to drive an LED
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
    //Serial.println(gammatable[i]);
  }
}


void loop() {
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED

  delay(60);  // takes 50ms to read 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // turn off LED

  Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.println(blue);
  
  if(red>170 && green<100 && blue<150){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cor detectada:");
    lcd.setCursor(0,1);
    lcd.print("Vermelho");
  }
  else if (red>350 && red<550 && green>850 && green<1300 && blue>600){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cor detectada:");
    lcd.setCursor(0,1);
    lcd.print("Azul");
  }
   else if (red<150 && green>350 && green<600 && blue<230){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cor detectada:");
    lcd.setCursor(0,1);
    lcd.print("Verde");
  }
  else if (red>1200 && red<2900 && green<500 && blue<350){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cor detectada:");
    lcd.setCursor(0,1);
    lcd.print("Laranja");
  }


  // Figure out some basic hex code for visualization
  uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();

  analogWrite(redpin, gammatable[(int)r]);
  analogWrite(greenpin, gammatable[(int)g]);
  analogWrite(bluepin, gammatable[(int)b]);

}