
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

void printDigits(int digits);
void digitalClockDisplay();
void WriteDigits(int digits,File dataFile);
void digitalClockWrite(File dataFile);
//*************************DataLogger******************************
const int chipSelect = BUILTIN_SDCARD;

//*************************Filter**********************************
#define FILTER_SHIFT 8   //parameter K

int32_t filter_reg;             // Delay element - 32 bits
int16_t filter_output;          // Filter output - 16 bits


uint8_t ledstatus = false;


//*************************SparkFun ACS712 and ACS723 Demo***********************************
const int analogInPin = A1;

// Number of samples to average the reading over
// Change this to make the reading smoother... but beware of buffer overflows!
const int avgSamples = 10;

int sensorValue = 0;

float sensitivity = -1.42857;//100.0/70.0;//100.0 / 500.0; //100mA per 500mV = 0.2
float Vref = 470; // Output voltage with no current: ~ 2500mV or 2.5V

unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillisLED = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 50;           // interval at which to blink (milliseconds)
const long intervalLED = 500;           // interval at which to blink (milliseconds)

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);

// set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  pinMode(LED_BUILTIN,OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
      digitalWrite(LED_BUILTIN,ledstatus);
      ledstatus = !ledstatus;
      delay(250);
    }
  }
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
    // if the file is available, write header:
    if (dataFile) {
      dataFile.print("Time,");
      dataFile.println("mA");
      dataFile.close();
    }
}

void loop() {
  if (millis() - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = millis();

    filter_reg = filter_reg - (filter_reg >> FILTER_SHIFT) + analogRead(analogInPin);

    // sensorValue = sensorValue / avgSamples;
    sensorValue = filter_reg >> FILTER_SHIFT;

    // The on-board ADC is 10-bits -> 2^10 = 1024 -> 3.3V / 1024 ~= 3.22mV
    // The voltage is in millivolts
    float voltage = 3.22 * sensorValue;

    // This will calculate the actual current (in mA)
    // Using the Vref and sensitivity settings you configure
    float current = (voltage - Vref) * sensitivity;

    // make a string for assembling the data to log:
    String dataString = "";
    dataString = String(current);

    // open the file.
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      digitalClockWrite(dataFile);
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      digitalClockDisplay();
      Serial.println(dataString);
    } else {
      // if the file isn't open, pop up an error:
      Serial.println("SD card failed");
      while (1) {
        // No SD card, so don't do anything more - stay stuck here
        digitalWrite(LED_BUILTIN,ledstatus);
        ledstatus = !ledstatus;
        delay(150);
      }
    }
    // Reset the sensor value for the next reading
    sensorValue = 0;
  }

  if (millis() - previousMillisLED >= intervalLED) {
      digitalWrite(LED_BUILTIN,ledstatus);
      ledstatus = !ledstatus;
      previousMillisLED = millis();
  }
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print("/");
  Serial.print(month());
  Serial.print("/");
  Serial.print(year()); 
  Serial.print(","); 
}

void WriteDigits(int digits,File dataFile){
dataFile.print(":");
if(digits < 10)
  dataFile.print('0');
dataFile.print(digits);
}

void digitalClockWrite(File dataFile) {
dataFile.print(hour());
WriteDigits(minute(),dataFile);
WriteDigits(second(),dataFile);
dataFile.print(" ");
dataFile.print(day());
dataFile.print("/");
dataFile.print(month());
dataFile.print("/");
dataFile.print(year()); 
dataFile.print(","); 
}
