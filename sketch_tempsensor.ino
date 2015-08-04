#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//Analog pin used to read buttons
#define BUTTONS_PIN A0

#define BUTTON_1_RANGE_LOWER_BOUND 650
#define BUTTON_1_RANGE_UPPER_BOUND 750
#define BUTTON_2_RANGE_LOWER_BOUND 800
#define BUTTON_2_RANGE_UPPER_BOUND 899
#define BUTTON_3_RANGE_LOWER_BOUND 900
#define BUTTON_3_RANGE_UPPER_BOUND 950

#define UP_BUTTON_LOWER_RANGE BUTTON_3_RANGE_LOWER_BOUND
#define UP_BUTTON_UPPER_RANGE BUTTON_3_RANGE_UPPER_BOUND

#define DOWN_BUTTON_LOWER_RANGE BUTTON_1_RANGE_LOWER_BOUND
#define DOWN_BUTTON_UPPER_RANGE BUTTON_1_RANGE_UPPER_BOUND

//Don't let anyone set anything higher than 90
#define MAX_TEMPERATURE 90

#define UP_PRESS_INCREMENT 0.5
#define DOWN_PRESS_DECREMENT 0.5

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

//Pin used to control the LCD backlight
#define BACKLIGHT_ON_OFF 13

//the output connected to relay 1
#define RELAY_ONE_OUTPUT A2
#define RELAY_TWO_OUTPUT A3

//SDA (data) is A4 pin and SCL (clock) is A5
#define DS3231_I2C_ADDRESS 0x68

//set this to the number of degrees +/- the current setting
//to prevent short cycling
#define SHORT_CYCLING_FACTOR 0.5

//RS pin on arduino
#define LCD_RS_PIN 7

//LCD Enable PIN
#define LCD_ENABLE_PIN 8

//LCD Data pins
#define LCD_D4 9
#define LCD_D5 10
#define LCD_D6 11
#define LCD_D7 12

#define ONE_DAY 86400000

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

//LCD library instance
LiquidCrystal lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D4,
                  LCD_D5, LCD_D6, LCD_D7); 
 
void printNormal(void)
{
  lcd.setCursor(0,0);
  lcd.print("Current: ");
  lcd.setCursor(0, 1);
  lcd.print("Set: ");
  lcd.setCursor(12, 1);
  lcd.print("OFF");
}

// Convert normal decimal numbers to binary coded decimal
byte decimalToBinaryCodedDecimal(byte p_val)
{
  return ((p_val / 10 * 16) + (p_val % 10));
}

// Convert binary coded decimal to normal decimal numbers
byte binaryCodedDecimalToDecimal(byte p_val)
{
  return ((p_val / 16 * 10) + (p_val % 16));
}

void setDS3231time(byte p_second,
                   byte P-minute,
                   byte p_hour,
                   byte p_dayOfWeek,
                   byte p_dayOfMonth,
                   byte p_month,
                   byte p_year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  // set next input to start at the seconds register
  Wire.write(0);
  // set seconds
  Wire.write(decmalToBinaryCodedDecimal(p_second));
  // set minutes
  Wire.write(decmalToBinaryCodedDecimal(p_minute));
  // set hours
  Wire.write(decmalToBinaryCodedDecimal(p_hour));
  // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decmalToBinaryCodedDecimal(p_dayOfWeek));
  // set date (1 to 31)
  Wire.write(decmalToBinaryCodedDecimal(p_dayOfMonth));
  // set month
  Wire.write(decmalToBinaryCodedDecimal(p_month));
  // set year (0 to 99)
  Wire.write(decmalToBinaryCodedDecimal(p_year));
  Wire.endTransmission();
}
void readDS3231time(byte *p_second,
                    byte *p_minute,
                    byte *p_hour,
                    byte *p_dayOfWeek,
                    byte *p_dayOfMonth,
                    byte *p_month,
                    byte *p_year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  // set DS3231 register pointer to 00h
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *p_second = binaryCodedDecimalToDecimal(Wire.read() & 0x7f);
  *p_minute = binaryCodedDecimalToDecimal(Wire.read());
  *p_hour = binaryCodedDecimalToDecimal(Wire.read() & 0x3f);
  *p_dayOfWeek = binaryCodedDecimalToDecimal(Wire.read());
  *p_dayOfMonth = binaryCodedDecimalToDecimal(Wire.read());
  *p_month = binaryCodedDecimalToDecimal(Wire.read());
  *p_year = binaryCodedDecimalToDecimal(Wire.read());
}


void setup(void)
{
  //set to off initially
  pinMode(RELAY_ONE_OUTPUT, OUTPUT);
  digitalWrite(RELAY_ONE_OUTPUT, HIGH);
  //turn backlight on initially
  pinMode(BACKLIGHT_ON_OFF, OUTPUT);
  //High turns backlight on, low turns it off
  digitalWrite(BACKLIGHT_ON_OFF, HIGH);

  //start wire lib for I2C bus
  Wire.begin();

  //set the initial time here:
  //DS3231 seconds, minutes, hours, day, date, month, year
  // 24 hour clock, days are 1-7 (Sunday - Saturday)
  //setDS3231time(30, 42, 21, 4, 26, 11, 14);

  // start serial port
  //Serial.begin(9600);

  // Start up the library
  sensors.begin();
  
  //Start LCD library
  lcd.begin(16, 2);
  printNormal();
  //lcd.print("Current: ");
  //lcd.setCursor(0, 1);
  //lcd.print("Set: ");
  //lcd.setCursor(12, 1);
  //lcd.print("OFF");
}

//should be local to this loop
static unsigned long times[] = {ONE_DAY * 6, ONE_DAY / 2, ONE_DAY / 2, ONE_DAY * 10, ONE_DAY / 2, ONE_DAY / 2, ONE_DAY / 2, ONE_DAY / 2, ONE_DAY / 2, ONE_DAY * 3};
static float temperatures[] = {52.0, 57.0, 62.0, 67.0, 62.0, 57.0, 52.0, 47.0, 40.0, 32.0};
static unsigned long initialStartTime(millis());
static int currentIndex(0);
static int maxIndex(4);
static unsigned long changeTime(initialStartTime + times[currentIndex]);
static bool isOn(false);
static bool isIdle(false);
static unsigned long idleTime(initialStartTime + 20000);
static bool isChangedByUser = false;


void printMenu(void)
{
  unsigned long currentTime = millis();
  
  lcd.setCursor(0, 1);
  lcd.print("Cng: ");

  while(currentTime < idleTime)
  {
    int buttonValue = analogRead(BUTTONS_PIN);
    if(buttonValue > UP_BUTTON_LOWER_RANGE &&
       buttonValue <= UP_BUTTON_UPPER_RANGE)
    {
      //Do UP press action
      if(temperatures[currentIndex] < MAX_TEMPERATURE)
      {
        isChangedByUser = true;
        temperatures[currentIndex] += UP_PRESS_INCREMENT;
        delay(500);
      }
    }
    else if(buttonValue > DOWN_BUTTON_LOWER_RANGE &&
       buttonValue <= DOWN_BUTTON_UPPER_RANGE)
    {
      //Do DOWN press action
      //if current setting is higher than the amount being subtracted
      if(temperatures[currentIndex] > DOWN_PRESS_DECREMENT)
      {
        isChangedByUser = true;
        temperatures[currentIndex] -= DOWN_PRESS_DECREMENT;
        delay(500);
      }
    }
    //update idle time and screen
    if(buttonValue > BUTTON_1_RANGE_LOWER_BOUND)
    {
      idleTime = currentTime + 20000;
      lcd.setCursor(5, 1);
      lcd.print(temperatures[currentIndex]);
    }
    
    currentTime = millis();
  }
  printNormal();
}

void loop(void)
{
  unsigned long currentTime = millis();
  if(!isChangedByUser &&
     currentTime > changeTime &&
     currentIndex < maxIndex)
  {
    //Serial.print(currentTime);
    //Serial.println("milliseconds, changing index");
    ++currentIndex;
    changeTime = millis() + times[currentIndex];
  }

  int buttonValue = analogRead(BUTTONS_PIN);
  if(isIdle && buttonValue > BUTTON_1_RANGE_LOWER_BOUND)
  {
    digitalWrite(BACKLIGHT_ON_OFF, HIGH);
    isIdle = false;
    idleTime = currentTime + 20000;
    printMenu();
  }
  if(currentTime > idleTime)
  {
    digitalWrite(BACKLIGHT_ON_OFF, LOW);
    isIdle = true;
  }


  //Serial.println(buttonValue);

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures

  //Serial.print("Current Setting: ");
  //Serial.print(temperatures[currentIndex]);
  //Serial.print(" Actual Temp for Device 1 is: ");
  float tempInFahrenheit = sensors.getTempCByIndex(0) * 9. / 5. + 32;
  
  lcd.setCursor(9, 0);
  lcd.print(tempInFahrenheit);
  lcd.setCursor(5, 1);
  lcd.print(temperatures[currentIndex]);
  //Serial.println(tempInFahrenheit); // Why "byIndex"? 
    // You can have more than one IC on the same bus. 
    // 0 refers to the first IC on the wire
  if(tempInFahrenheit > (temperatures[currentIndex] + SHORT_CYCLING_FACTOR)
     && !isOn)
  {
    isOn = true;
    lcd.setCursor(12, 1);
    lcd.print("ON ");
    //Serial.println(": TURN ON Refrigerator");
    digitalWrite(RELAY_ONE_OUTPUT, LOW);
  }
  else if(tempInFahrenheit <= (temperatures[currentIndex] - SHORT_CYCLING_FACTOR)
          && isOn)
  {
    isOn = false;
    lcd.setCursor(12, 1);
    lcd.print("OFF");
    //Serial.println(": TURN OFF Refrigerator");
    digitalWrite(RELAY_ONE_OUTPUT, HIGH);
  }
}

