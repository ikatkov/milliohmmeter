#include <Arduino.h>
#include <U8g2lib.h>
#include <EasyButton.h>
#include <INA219_WE.h>

//aprox current is ~100mA
static int const I2C_ADDRESS_A = 0x40;
static int const I2C_ADDRESS_B = 0x41;
static float const RSHUNT = 0.1;
static const byte CAL_BUTTON_PIN = 6;

//aprox current is ~100mA
static const int GAIN40_STATE = 40;   //1...400 mOhm
static const int GAIN80_STATE = 80;   //0.4 ... 0.8 Ohm
static const int GAIN160_STATE = 160; //0.8 ... 1.6 Ohm
static const int GAIN320_STATE = 320; //1.6 ... 3.2 Ohm
static const int OVERFLOW_STATE = 999;

static const char PROGMEM GAIN40_RANGE[] = "1 - 400 m";
static const char PROGMEM GAIN80_RANGE[] = "400 - 800 m";
static const char PROGMEM GAIN160_RANGE[] = "0.8 - 1.6 ";
static const char PROGMEM GAIN320_RANGE[] = "1.6 - 3.2 ";
static const char PROGMEM OVERFLOW_RANGE[] = "> 3.2 ";
static const char PROGMEM OVERFLOW_MEASUREMENT[] = " ---";

//internal states
static const int CALIBRATION_ERROR_STATE = -1;
static const int CALIBRATION_STATE = 1;
static const int CALIBRATION_SHORT_LEADS_PROMPT_STATE = 2;
static const int CALIBRATION_SHORT_LEADS_STATE = 3;
static const int READY_STATE = 4;

INA219_WE ina219_A = INA219_WE(I2C_ADDRESS_A);
INA219_WE ina219_B = INA219_WE(I2C_ADDRESS_B);
U8G2_SSD1306_128X64_NONAME_1_HW_I2C display(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);
EasyButton calButton(CAL_BUTTON_PIN);

float offsetA_mV;
float offsetB_mV;
float offsetResistor;
float resistor;

int state = 0;
int gainState = GAIN40_STATE;
byte coutDownCounter = 20;

void continueCallibration();

void displayShortLeadsPrompt()
{
  display.firstPage();
  do
  {
    display.setDrawColor(1);
    display.setFont(u8g2_font_10x20_mr);
    display.drawStr(10, 32, "SHORT LEADS");
    display.setFont(u8g2_font_unifont_t_greek);
    display.drawStr(15, 50, "and press Cal.");
    char buffer[4];
    sprintf(buffer, "...%d", coutDownCounter / 2);
    display.drawStr(97, 62, buffer);
  } while (display.nextPage());
}

void directWrite(uint8_t pin, uint8_t x)
{
  if (pin / 8)
  { // pin >= 8
    PORTB ^= (-x ^ PORTB) & (1 << (pin % 8));
  }
  else
  {
    PORTD ^= (-x ^ PORTD) & (1 << (pin % 8));
  }
}

void driveOff()
{
  directWrite(7, LOW);
  directWrite(8, LOW);
  directWrite(9, LOW);

  directWrite(10, LOW);
  directWrite(11, LOW);
  directWrite(12, LOW);
}

void driveP()
{
  directWrite(10, LOW);
  directWrite(11, LOW);
  directWrite(12, LOW);

  directWrite(7, HIGH);
  directWrite(8, HIGH);
  directWrite(9, HIGH);
}

void driveN()
{
  directWrite(7, LOW);
  directWrite(8, LOW);
  directWrite(9, LOW);
  directWrite(10, HIGH);
  directWrite(11, HIGH);
  directWrite(12, HIGH);
}

//oversampling and decimation to get 3 more bits in ADC
float measure_mV(INA219_WE ina219, float offset_mV, int times)
{
  long measurement = 0;
  for (int i = 0; i < times; i++)
  {
    int16_t newMeasurement = (int16_t)ina219.readRegister(INA219_SHUNT_REG) - offset_mV;
    measurement += newMeasurement;
  }

  return ((float)measurement) / times * 0.01;
}

void purgeADC()
{
  // Throw away 1st readings to allow ADC to settle
  for (int lp = 0; lp < 20; lp++)
  {
    ina219_A.getShuntVoltage_mV();
    ina219_B.getShuntVoltage_mV();
    // Throw away 1st readings to allow ADC to settle
  }
}

bool doAutoRanging()
{
  Serial.println("doAutoRanging");
  driveP();
  ina219_B.setPGain(PG_40);
  delay(2);
  purgeADC();
  float voltageB = ina219_B.getShuntVoltage_mV() - offsetB_mV;
  Serial.println(voltageB);
  if (!ina219_B.getOverflow())
  {
    Serial.println("set gain to PG_40");
    gainState = GAIN40_STATE;
    driveOff();
    delay(2);
    return true;
  }

  ina219_B.setPGain(PG_80);
  delay(2);
  purgeADC();
  voltageB = ina219_B.getShuntVoltage_mV() - offsetB_mV;
  Serial.println(voltageB);
  if (!ina219_B.getOverflow())
  {
    Serial.println("set gain to PG_80");
    gainState = GAIN80_STATE;
    driveOff();
    delay(2);
    return true;
  }

  ina219_B.setPGain(PG_160);
  delay(2);
  purgeADC();
  voltageB = ina219_B.getShuntVoltage_mV() - offsetB_mV;
  Serial.println(voltageB);
  if (!ina219_B.getOverflow())
  {
    Serial.println("set gain to PG_160");
    gainState = GAIN160_STATE;
    driveOff();
    delay(2);
    return true;
  }

  ina219_B.setPGain(PG_320);
  delay(2);
  purgeADC();
  voltageB = ina219_B.getShuntVoltage_mV() - offsetB_mV;
  Serial.println(voltageB);
  if (!ina219_B.getOverflow())
  {
    Serial.println("set gain to PG_320");
    gainState = GAIN320_STATE;
    driveOff();
    delay(2);
    return true;
  }
  Serial.println("Overflow or open circuit");
  gainState = OVERFLOW_STATE;
  driveOff();
  delay(2);
  return false;
}

void displayAll()
{
  char bufferRange[12];
  char bufferMeasurement[15];
  char bufferMeasurement2[4] = "";

  if (resistor < 0.4)
  {
    strncpy_P(bufferRange, GAIN40_RANGE, 12);

    int mOhmResistance = (int)(resistor * 1000);
    int fractionPart = round((resistor * 1000 - mOhmResistance) * 100);
    fractionPart = fractionPart > 0 ? fractionPart : 0;
    snprintf(bufferMeasurement, 15, "%d", mOhmResistance);
    snprintf(bufferMeasurement2, 4, ".%02d", fractionPart);
  }
  else if (resistor < 0.8)
  {
    strncpy_P(bufferRange, GAIN80_RANGE, 12);
    int mOhmResistance = (int)(resistor * 1000);
    int fractionPart = round((resistor * 1000 - mOhmResistance) * 100);
    fractionPart = fractionPart > 0 ? fractionPart : 0;
    snprintf(bufferMeasurement, 15, "%d", mOhmResistance);
    snprintf(bufferMeasurement2, 4, ".%02d", fractionPart);
  }
  else if (resistor < 1.6)
  {
    strncpy_P(bufferRange, GAIN160_RANGE, 12);
    snprintf(bufferMeasurement, 15, "%.3f", (double)resistor);
  }
  else if (resistor < 3.2)
  {
    strncpy_P(bufferRange, GAIN320_RANGE, 12);
    snprintf(bufferMeasurement, 15, "%.3f", (double)resistor);
  }
  else
  {
    strncpy_P(bufferRange, OVERFLOW_RANGE, 12);
    strncpy_P(bufferMeasurement, OVERFLOW_MEASUREMENT, 15);
  }

  display.firstPage();
  do
  {
    display.setDrawColor(1);
    display.setFont(u8g2_font_logisoso34_tn); //u8g2_font_10x20_mr
    int width = display.drawUTF8(2, 52, bufferMeasurement);
    display.setFont(u8g2_font_unifont_t_greek);
    display.drawStr(width + 2, 52, bufferMeasurement2);
    width = display.drawStr(0, 10, bufferRange);
    display.drawGlyph(width + 2, 10, 0x03a9);

  } while (display.nextPage());

  Serial.print("Resistance: ");
  Serial.print(resistor, 5);
  Serial.print(" Ohm\tOffset Resistance: ");
  Serial.print(offsetResistor, 5);
  Serial.println(" Ohm");
}

void drawScreen()
{
  switch (state)
  {
  case CALIBRATION_SHORT_LEADS_PROMPT_STATE:
  {
    displayShortLeadsPrompt();
    break;
  }
  case READY_STATE:
  {
    displayAll();
    break;
  }
  }
}

void startCallibration()
{
  state = CALIBRATION_STATE;
  drawScreen();
  driveOff();
  delay(2);
  purgeADC();
  offsetA_mV = measure_mV(ina219_A, 0, 64);
  offsetB_mV = measure_mV(ina219_B, 0, 64);
  continueCallibration();
}

float measureResistor(int numLoops, float offset)
{
  float resistorB = 0;
  driveP();
  delay(2);
  purgeADC();
  for (int i = 0; i < numLoops; i++)
  {
    float voltageA = measure_mV(ina219_A, offsetA_mV, 1);
    float voltageB = measure_mV(ina219_B, offsetB_mV, 1);
    float newResistorB = voltageA == 0 ? 0 : (voltageB / voltageA * RSHUNT);
    resistorB += newResistorB;
  }
  driveN();
  delay(2);
  purgeADC();
  for (int i = 0; i < numLoops; i++)
  {
    float voltageA = measure_mV(ina219_A, offsetA_mV, 1);
    float voltageB = measure_mV(ina219_B, offsetB_mV, 1);
    float newResistorB = voltageA == 0 ? 0 : (voltageB / voltageA * RSHUNT);
    resistorB += newResistorB;
  }

  driveOff();
  return resistorB = (resistorB / (2 * numLoops)) - offset;
}

void continueCallibration()
{
  Serial.println("continueCallibration");
  if (!doAutoRanging())
  {
    coutDownCounter = 6;
    state = CALIBRATION_SHORT_LEADS_PROMPT_STATE;
  }
  else
  {
    offsetResistor = measureResistor(128, 0);
    Serial.print(" offsetResistor: ");
    Serial.print(offsetResistor * 1000, 10);
    Serial.println(" mOhm");
    state = READY_STATE;
  }
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  Serial.println("Setup");
  calButton.onPressed(startCallibration);

  if (!ina219_A.init())
  {
    Serial.println("INA219 A is not connected!");
  }
  if (!ina219_B.init())
  {
    Serial.println("INA219 B is not connected!");
  }

  if (!display.begin())
  {
    Serial.println("Display is not connected!");
  }

  ina219_A.setPGain(PG_40);
  ina219_A.setBusRange(BRNG_16);
  ina219_A.setShuntSizeInOhms(RSHUNT);

  ina219_B.setPGain(PG_40);
  ina219_B.setBusRange(BRNG_16);
  ina219_B.setShuntSizeInOhms(RSHUNT);

  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  startCallibration();
}

void loop()
{
  drawScreen();
  if (state == CALIBRATION_SHORT_LEADS_PROMPT_STATE)
  {
    coutDownCounter--;
    if (coutDownCounter <= 0)
    {
      state = CALIBRATION_SHORT_LEADS_STATE;
      continueCallibration();
    }
  }
  else if (state == READY_STATE)
  {
    doAutoRanging();
    resistor = measureResistor(128, offsetResistor);
    if (resistor < 0.0005)
    {
      continueCallibration();
    }
    else if (resistor > 3.6)
    {
      gainState = OVERFLOW_STATE;
    }
  }
  delay(500);
}
