#include <HotStepper.h>
#include <Wire.h>
#include <VL53L0X.h>

HotStepper stepper2(&PORTD, 0b11110000);
VL53L0X sensor;

int a;
int b;

void setup() {
  HotStepper::setup(32);
  pinMode(13, OUTPUT);
  pinMode(8, INPUT);
  Serial.begin(9600);
  Wire.begin();
  sensor.init();
  sensor.setSignalRateLimit(0.1);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  sensor.setMeasurementTimingBudget(20000);
}


void loop() {
  stepper2.turn(3000, FORWARD);
  if (digitalRead(8) == HIGH) {
    digitalWrite(13, HIGH);
    for (int i = 0; i < 50; i++) {
      int c = millis();
      Serial.print(sensor.readRangeSingleMillimeters());
      Serial.print(" , ");
      Serial.print(map(i,0,49,0,360));
      Serial.print(" . ");
      int d = millis();
      delay(26-(d-c));
    }
    digitalWrite(13, LOW);
  }
}
