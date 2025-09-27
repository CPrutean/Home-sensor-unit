#include <LCD_I2C.h>
#include <PIR.h>

#define MOTION_SENSOR_PIN 34
#define SDA_PIN 22
#define SCL_PIN 21

LCD_I2C lcd(0x27, 16, 2);
PIR motion;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  motion.add(34);
  lcd.begin(SDA_PIN, SCL_PIN);
  lcd.backlight();
  lcd.print("Temp");
}
uint8_t reading;
void loop() {
  
}
