#include <LiquidCrystal_I2C.h>

#include <OneWire.h>
#include <DallasTemperature.h>

int sensorInput; // The variable we will use to store the sensor input
double temp; // The variable we will use to store temperature in degrees.

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t bit0 = 2;
const uint8_t bit1 = 4;
const uint8_t bit2 = 6;
const uint8_t bit3 = 8;

#define ONE_WIRE_BUS  = A1;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int ADC_Temp_Pin = A2;

int ADC_Co2_Reading;
int ADC_Temp_Reading;

int Co2_PPM;
double Temp;

void setup()
{
  pinMode(bit0, OUTPUT);
  pinMode(bit1, OUTPUT);
  pinMode(bit2, OUTPUT);
  pinMode(bit3, OUTPUT);

  digitalWrite(bit0, 1);
  digitalWrite(bit1, 1);
  digitalWrite(bit2, 1);
  digitalWrite(bit3, 1);

  Serial.begin(115200);
  lcd.begin();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Co2 test");

  delay(1000);

  digitalWrite(bit0, 0);
  digitalWrite(bit1, 0);
  digitalWrite(bit2, 0);
  digitalWrite(bit3, 0);
  lcd.clear();
}

void loop()
{
  sensors.requestTemperatures(); 
  ADC_Temp_Reading = analogRead(ADC_Temp_Pin);

  Co2_PPM = ADC_Co2_Reading * (2000 / 1023.0);
  temp = sensors.getTempCByIndex(0);

  int Temp_Int = temp;
  
  Serial.println(Co2_PPM);
  Serial.println(temp);

  lcd.clear();
  lcd.print("Co2: ");
  lcd.print(Co2_PPM);
  lcd.print(" PPM");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(Temp_Int);
  lcd.print(" C");

  delay(1000);
  lcd.setCursor(0, 0);
}