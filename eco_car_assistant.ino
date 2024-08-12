#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <LCD_I2C.h>
#include <IRremote.h>
#include <dht.h>

// Initialize I2C Address with LCD
LCD_I2C lcd(0x27);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Initialize Variables
const int receivePin = 11;
IRrecv irrecv(receivePin);
decode_results results;

bool power_on = false;
bool toggle_temp = true;

dht DHT;
const int DHT11_PIN = 7;

int high_accel_count = 0;
int hard_brake_count = 0;

int toggle_seconds = 0;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true); 

  // Initialize the accelerometer
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected");
    while (1);
  }
  // Set the range to +-16G
  accel.setRange(ADXL345_RANGE_16_G);
  Serial.println("ADXL345 initialized");

  // Setup Screen for LCD
  lcd.begin();
  lcd.backlight();
  lcd.print("PRESS POWER");
  lcd.setCursor(0, 1);
  lcd.print("TO START TRIP");

}

void loop() {

  // Begin Loop if Power Input from Remote Detected
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX); 
    if (power_on == false && results.value == 0xFFA25D) {
      lcd.clear();
      power_on = true;
    }
    // End Trip Screen
    else if (power_on == true && results.value == 0xFFA25D) {
      lcd.clear();
      power_on = false;
      lcd.print("TRIP ENDED");
      delay(2000);
      lcd.clear();
      lcd.print("HIGH ACCEL: ");
      lcd.print(high_accel_count);
      lcd.setCursor(0, 1);
      lcd.print("HARD BRAKE: ");
      lcd.print(hard_brake_count);
      delay(5000);
      setup();
    }
    irrecv.resume();
  }

  if (power_on) {

      // Get a new sensor event
      sensors_event_t event;
      accel.getEvent(&event);

      double x_calibrated = (event.acceleration.x - 1.80);

      // Acceleration Threshold
      double accel_threshold = 3.0;
      double brake_threshold = -3.0;
      
      // Display the acceleration values
      Serial.print("X: ");
      Serial.print(x_calibrated);
      Serial.print(" m/s^2, ");
      
      // Print on LCD display
      lcd.print("ACCL: "); 
      lcd.print(x_calibrated);
      lcd.print(" m/s2");

      readTemp();
      
      // Alert if acceleration over threshold
      if (x_calibrated > accel_threshold) {
        lcd.setCursor(0,1);
        lcd.print("ACCEL TOO FAST!");
        high_accel_count++;
      }
      else if (x_calibrated < brake_threshold) {
        lcd.setCursor(0,1);
        lcd.print("BRAKE SOFTER!!!");
        hard_brake_count++;
      }

    delay(1000);
    lcd.clear();
  }   
}

void readTemp() {
  unsigned temp_input_c = DHT.read11(DHT11_PIN);
    double temp_f = (DHT.temperature * 9/5) + 32;   

    Serial.print(temp_f); 
    Serial.print("ºF");

// Display Temp
    if (toggle_temp == true) {

      lcd.setCursor(0, 1);
      lcd.print("TEMP (F): ");
      lcd.print(temp_f);
    }
    
// Decide temperature function
    else {
     
      if (temp_f > 85) {
        lcd.setCursor(0,1);
        lcd.print("OPEN WINDOW");
      }
      else if (temp_f > 78) {
        lcd.setCursor(0,1);
        lcd.print("SET AC TEMP LOW");
      }
      else if (temp_f < 70) {
        lcd.setCursor(0,1);
        lcd.print("SET TEMP TO 78");
      }
      else {
        lcd.setCursor(0,1);
        lcd.print("AC TEMP OPTIMAL");
      }
    }
}
