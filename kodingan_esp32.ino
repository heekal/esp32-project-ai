#include <Fuzzy.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHT_SENSOR_PIN 33
#define DHT_SENSOR_TYPE DHT11

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define LIGHT_SENSOR_PIN 4

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Fuzzy
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput
FuzzySet *Dingin = new FuzzySet(0, 0, 15, 20);
FuzzySet *Biasa = new FuzzySet(15, 20, 25, 30);
FuzzySet *Panas = new FuzzySet(25, 30, 50, 50);

FuzzySet *Kering = new FuzzySet(0, 0, 20, 35);
FuzzySet *Lembab = new FuzzySet(20, 35, 70, 80);
FuzzySet *Basah = new FuzzySet(70, 80, 100, 100);

FuzzySet *Terang = new FuzzySet(0, 0, 400, 800);
FuzzySet *Redup = new FuzzySet(400, 800, 1400, 2000);
FuzzySet *Gelap = new FuzzySet(1400, 2000, 3200, 3200);

// FuzzyOutput
FuzzySet *TidakPerluAir = new FuzzySet(0, 0, 20, 25);
FuzzySet *PerluAirSedikit = new FuzzySet(20, 25, 45, 50);
FuzzySet *PerluAir = new FuzzySet(45, 50, 70, 75);
FuzzySet *PerluBanyakAir= new FuzzySet(70, 75, 100, 100);

void setup()
{
  Serial.begin(9600);
  dht_sensor.begin();
  analogSetAttenuation(ADC_11db);

  FuzzyInput *temperature = new FuzzyInput(1);

  temperature->addFuzzySet(Dingin);
  temperature->addFuzzySet(Biasa);
  temperature->addFuzzySet(Panas);
  fuzzy->addFuzzyInput(temperature);

  FuzzyInput *humidity = new FuzzyInput(2);

  humidity->addFuzzySet(Kering);
  humidity->addFuzzySet(Lembab);
  humidity->addFuzzySet(Basah);
  fuzzy->addFuzzyInput(humidity);

  FuzzyInput *light = new FuzzyInput(3);

  light->addFuzzySet(Gelap);
  light->addFuzzySet(Redup);
  light->addFuzzySet(Terang);
  fuzzy->addFuzzyInput(light);

  FuzzyOutput *water = new FuzzyOutput(1);

  water->addFuzzySet(TidakPerluAir);
  water->addFuzzySet(PerluAirSedikit);
  water->addFuzzySet(PerluAir);
  water->addFuzzySet(PerluBanyakAir);
  fuzzy->addFuzzyOutput(water);

  // Output
  FuzzyRuleConsequent *NoNeedWater = new FuzzyRuleConsequent();
  NoNeedWater->addOutput(TidakPerluAir);
  FuzzyRuleConsequent *LittleWater = new FuzzyRuleConsequent();
  LittleWater->addOutput(PerluAirSedikit);
  FuzzyRuleConsequent *NeedWater = new FuzzyRuleConsequent();
  NeedWater->addOutput(PerluAir);
  FuzzyRuleConsequent * MandatoryWater = new FuzzyRuleConsequent();
  MandatoryWater->addOutput(PerluBanyakAir);
  
  // Kering - Lembab - Basah
  FuzzyRuleAntecedent *HumidityDry = new FuzzyRuleAntecedent();
  HumidityDry->joinSingle(Kering);
  FuzzyRuleAntecedent *HumidityDamp = new FuzzyRuleAntecedent();
  HumidityDamp->joinSingle(Lembab);
  FuzzyRuleAntecedent *HumidityWet = new FuzzyRuleAntecedent();
  HumidityWet->joinSingle(Basah);

  // Dingin Gelap
  FuzzyRuleAntecedent *TemperatureColdAndLightDark = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDark->joinWithAND(Dingin, Gelap);

  // Rule 1-3, Dingin Gelap: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureColdAndLightDarkAndHumidityDry = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDarkAndHumidityDry->joinWithAND(TemperatureColdAndLightDark, HumidityDry);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, TemperatureColdAndLightDarkAndHumidityDry, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule1);

  FuzzyRuleAntecedent *TemperatureColdAndLightDarkAndHumidityDamp = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDarkAndHumidityDamp->joinWithAND(TemperatureColdAndLightDark, HumidityDamp);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, TemperatureColdAndLightDarkAndHumidityDamp, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule2);

  FuzzyRuleAntecedent *TemperatureColdAndLightDarkAndHumidityWet = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDarkAndHumidityWet->joinWithAND(TemperatureColdAndLightDark, HumidityWet);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, TemperatureColdAndLightDarkAndHumidityWet, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule3);
  
  // Biasa Gelap
  FuzzyRuleAntecedent *TemperatureNormalAndLightDark = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDark->joinWithAND(Biasa, Gelap);

  // Rule 4-6, Biasa Gelap: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureNormalAndLightDarkAndHumidityDry = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDarkAndHumidityDry->joinWithAND(TemperatureNormalAndLightDark, HumidityDry);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, TemperatureNormalAndLightDarkAndHumidityDry, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule4);

  FuzzyRuleAntecedent *TemperatureNormalAndLightDarkAndHumidityDamp = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDarkAndHumidityDamp->joinWithAND(TemperatureNormalAndLightDark, HumidityDamp);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, TemperatureNormalAndLightDarkAndHumidityDamp, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule5);

  FuzzyRuleAntecedent *TemperatureNormalAndLightDarkAndHumidityWet = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDarkAndHumidityWet->joinWithAND(TemperatureNormalAndLightDark, HumidityWet);
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, TemperatureNormalAndLightDarkAndHumidityWet, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule6);

  // Panas Gelap
  FuzzyRuleAntecedent *TemperatureHotAndLightDark = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDark->joinWithAND(Panas, Gelap);

  // Rule 7-9, Panas Gelap: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureHotAndLightDarkAndHumidityDry = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDarkAndHumidityDry->joinWithAND(TemperatureHotAndLightDark, HumidityDry);
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, TemperatureHotAndLightDarkAndHumidityDry, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule7);

  FuzzyRuleAntecedent *TemperatureHotAndLightDarkAndHumidityDamp = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDarkAndHumidityDamp->joinWithAND(TemperatureHotAndLightDark, HumidityDamp);
  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, TemperatureHotAndLightDarkAndHumidityDamp, LittleWater);
  fuzzy->addFuzzyRule(fuzzyRule8);

  FuzzyRuleAntecedent *TemperatureHotAndLightDarkAndHumidityWet = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDarkAndHumidityWet->joinWithAND(TemperatureHotAndLightDark, HumidityWet);
  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, TemperatureHotAndLightDarkAndHumidityWet, LittleWater);
  fuzzy->addFuzzyRule(fuzzyRule9);

  // Dingin Redup
  FuzzyRuleAntecedent *TemperatureColdAndLightDim = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDim->joinWithAND(Dingin, Redup);

  // Rule 10-12, Dingin Redup: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureColdAndLightDimAndHumidityDry = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDimAndHumidityDry->joinWithAND(TemperatureColdAndLightDim, HumidityDry);
  FuzzyRule *fuzzyRule10 = new FuzzyRule(10, TemperatureColdAndLightDimAndHumidityDry, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule10);

  FuzzyRuleAntecedent *TemperatureColdAndLightDimAndHumidityDamp = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDimAndHumidityDamp->joinWithAND(TemperatureColdAndLightDim, HumidityDamp);
  FuzzyRule *fuzzyRule11 = new FuzzyRule(11, TemperatureColdAndLightDimAndHumidityDamp, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule11);

  FuzzyRuleAntecedent *TemperatureColdAndLightDimAndHumidityWet = new FuzzyRuleAntecedent();
  TemperatureColdAndLightDimAndHumidityWet->joinWithAND(TemperatureColdAndLightDim, HumidityWet);
  FuzzyRule *fuzzyRule12 = new FuzzyRule(12, TemperatureColdAndLightDimAndHumidityWet, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule12);

  // Biasa Redup
  FuzzyRuleAntecedent *TemperatureNormalAndLightDim = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDim->joinWithAND(Biasa, Redup);
  
  // Rule 13-15, Biasa Redup: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureNormalAndLightDimAndHumidityDry = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDimAndHumidityDry->joinWithAND(TemperatureNormalAndLightDim, HumidityDry);
  FuzzyRule *fuzzyRule13 = new FuzzyRule(13, TemperatureNormalAndLightDimAndHumidityDry, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule13);

  FuzzyRuleAntecedent *TemperatureNormalAndLightDimAndHumidityDamp = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDimAndHumidityDamp->joinWithAND(TemperatureNormalAndLightDim, HumidityDamp);
  FuzzyRule *fuzzyRule14 = new FuzzyRule(14, TemperatureNormalAndLightDimAndHumidityDamp, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule14);

  FuzzyRuleAntecedent *TemperatureNormalAndLightDimAndHumidityWet = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightDimAndHumidityWet->joinWithAND(TemperatureNormalAndLightDim, HumidityWet);
  FuzzyRule *fuzzyRule15 = new FuzzyRule(15, TemperatureNormalAndLightDimAndHumidityWet, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule15);

  // Panas Redup
  FuzzyRuleAntecedent *TemperatureHotAndLightDim = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDim->joinWithAND(Panas, Redup);

  // Rule 16-18, Panas Redup: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureHotAndLightDimAndHumidityDry = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDimAndHumidityDry->joinWithAND(TemperatureHotAndLightDim, HumidityDry);
  FuzzyRule *fuzzyRule16 = new FuzzyRule(16, TemperatureHotAndLightDimAndHumidityDry, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule16);

  FuzzyRuleAntecedent *TemperatureHotAndLightDimAndHumidityDamp = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDimAndHumidityDamp->joinWithAND(TemperatureHotAndLightDim, HumidityDamp);
  FuzzyRule *fuzzyRule17 = new FuzzyRule(17, TemperatureHotAndLightDimAndHumidityDamp, LittleWater);
  fuzzy->addFuzzyRule(fuzzyRule17);

  FuzzyRuleAntecedent *TemperatureHotAndLightDimAndHumidityWet = new FuzzyRuleAntecedent();
  TemperatureHotAndLightDimAndHumidityWet->joinWithAND(TemperatureHotAndLightDim, HumidityWet);
  FuzzyRule *fuzzyRule18 = new FuzzyRule(18, TemperatureHotAndLightDimAndHumidityWet, LittleWater);
  fuzzy->addFuzzyRule(fuzzyRule18);

  // Dingin Terang
  FuzzyRuleAntecedent *TemperatureColdAndLightBright = new FuzzyRuleAntecedent();
  TemperatureColdAndLightBright->joinWithAND(Dingin, Terang);

  // Rule 19-21, Dingin Terang: Kering Lembab Basah
  FuzzyRuleAntecedent *TemperatureColdAndLightBrightAndHummidityDry = new FuzzyRuleAntecedent();
  TemperatureColdAndLightBrightAndHummidityDry->joinWithAND(TemperatureColdAndLightBright, HumidityDry);
  FuzzyRule *fuzzyRule19 = new FuzzyRule(19, TemperatureColdAndLightBrightAndHummidityDry, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule19);

  FuzzyRuleAntecedent *TemperatureColdAndLightBrightAndHummidityDamp = new FuzzyRuleAntecedent();
  TemperatureColdAndLightBrightAndHummidityDamp->joinWithAND(TemperatureColdAndLightBright, HumidityDamp);
  FuzzyRule *fuzzyRule20 = new FuzzyRule(20, TemperatureColdAndLightBrightAndHummidityDamp, LittleWater);
  fuzzy->addFuzzyRule(fuzzyRule20);

  FuzzyRuleAntecedent *TemperatureColdAndLightBrightAndHummidityWet = new FuzzyRuleAntecedent();
  TemperatureColdAndLightBrightAndHummidityWet->joinWithAND(TemperatureColdAndLightBright, HumidityWet);
  FuzzyRule *fuzzyRule21 = new FuzzyRule(21, TemperatureColdAndLightBrightAndHummidityWet, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule21);

  // Biasa Terang
  FuzzyRuleAntecedent *TemperatureNormalAndLightBright = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightBright->joinWithAND(Biasa, Terang);

  // Rule 22-24
  FuzzyRuleAntecedent *TemperatureNormalAndLightBrightAndHummidityDry = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightBrightAndHummidityDry->joinWithAND(TemperatureNormalAndLightBright, HumidityDry);
  FuzzyRule *fuzzyRule22 = new FuzzyRule(22, TemperatureNormalAndLightBrightAndHummidityDry, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule22);

  FuzzyRuleAntecedent *TemperatureNormalAndLightBrightAndHummidityDamp = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightBrightAndHummidityDamp->joinWithAND(TemperatureNormalAndLightBright, HumidityDamp);
  FuzzyRule *fuzzyRule23 = new FuzzyRule(23, TemperatureNormalAndLightBrightAndHummidityDamp, LittleWater);
  fuzzy->addFuzzyRule(fuzzyRule23);

  FuzzyRuleAntecedent *TemperatureNormalAndLightBrightAndHummidityWet = new FuzzyRuleAntecedent();
  TemperatureNormalAndLightBrightAndHummidityWet->joinWithAND(TemperatureNormalAndLightBright, HumidityWet);
  FuzzyRule *fuzzyRule24 = new FuzzyRule(24, TemperatureNormalAndLightBrightAndHummidityWet, NoNeedWater);
  fuzzy->addFuzzyRule(fuzzyRule24);

  // Panas Terang
  FuzzyRuleAntecedent *TemperatureHotAndLightBright = new FuzzyRuleAntecedent();
  TemperatureHotAndLightBright->joinWithAND(Panas, Terang);

  // Rule 25-27
  FuzzyRuleAntecedent *TemperatureHotAndLightBrightAndHummidityDry = new FuzzyRuleAntecedent();
  TemperatureHotAndLightBrightAndHummidityDry->joinWithAND(TemperatureHotAndLightBright, HumidityDry);
  FuzzyRule *fuzzyRule25 = new FuzzyRule(25, TemperatureHotAndLightBrightAndHummidityDry, MandatoryWater);
  fuzzy->addFuzzyRule(fuzzyRule25);

  FuzzyRuleAntecedent *TemperatureHotAndLightBrightAndHummidityDamp = new FuzzyRuleAntecedent();
  TemperatureHotAndLightBrightAndHummidityDamp->joinWithAND(TemperatureHotAndLightBright, HumidityDamp);
  FuzzyRule *fuzzyRule26 = new FuzzyRule(26, TemperatureHotAndLightBrightAndHummidityDamp, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule26);

  FuzzyRuleAntecedent *TemperatureHotAndLightBrightAndHummidityWet = new FuzzyRuleAntecedent();
  TemperatureHotAndLightBrightAndHummidityWet->joinWithAND(TemperatureHotAndLightBright, HumidityWet);
  FuzzyRule *fuzzyRule27 = new FuzzyRule(27, TemperatureHotAndLightBrightAndHummidityWet, NeedWater);
  fuzzy->addFuzzyRule(fuzzyRule27);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.display();
}

void loop()
{
  int lembabread = dht_sensor.readHumidity();
  int suhuread = dht_sensor.readTemperature(); 
  int cahayaread = analogRead(LIGHT_SENSOR_PIN);
 
  if (isnan(lembabread) || isnan(suhuread)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: "); Serial.println(suhuread);
  Serial.print("Humidity: "); Serial.println(lembabread);
  Serial.print("Light: "); Serial.println(cahayaread);

  fuzzy->setInput(1, suhuread);
  fuzzy->setInput(2, lembabread);
  fuzzy->setInput(3, cahayaread);

  fuzzy->fuzzify();

  float output = fuzzy->defuzzify(1);
  
  
  float maxTerang = Gelap->getPertinence();
  String maxKeterangan = "Gelap";
  float valueTerang = Redup->getPertinence();
  if (valueTerang > maxTerang) {
      maxTerang = valueTerang;
      maxKeterangan = "Redup";
  }

  float valueTerang2 = Terang->getPertinence();
  if (valueTerang2 > maxTerang) {
      maxTerang = valueTerang2;
      maxKeterangan = "Terang"; 
  }

  float maxValue = TidakPerluAir->getPertinence();
  String maxLabel = "Tidak Perlu";

  float valueAir = PerluAirSedikit->getPertinence();
  if (valueAir > maxValue) {
      maxValue = valueAir;
      maxLabel = "Sedikit Air";
  }

  float valueAir2 = PerluAir->getPertinence();
  if (valueAir2 > maxValue) {
      maxValue = valueAir2;
      maxLabel = "Perlu Air";
  }

  float valueAir3 = PerluBanyakAir->getPertinence();
  if (valueAir3 > maxValue) {
      maxValue = valueAir3;
      maxLabel = "Banyak Air";
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.print("Suhu: ");
  display.print(suhuread);
  display.print(" ");
  display.cp437(true);
  display.write(167);
  display.println("C");
  display.print("Kelembaban: ");
  display.print(lembabread);
  display.println(" %");
  display.print("Cahaya: ");
  display.println(maxKeterangan);
  display.println(" ");
  display.println("Perlu Air? ");
  display.setTextSize(1.5);
  display.println(maxLabel);

  display.display();
  delay(20000);
}