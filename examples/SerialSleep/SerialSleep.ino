/*
  SerialDeepSleep

  This sketch demonstrates the usage of Serial Interrupts to wakeup a chip in sleep mode.

  This sketch is compatible only with board supporting uart peripheral in stop mode.

  This example code is in the public domain.
*/

#include "STM32LowPower.h"

void setup() {
  Serial.begin(9600);
  // Configure low power
  LowPower.begin();
  // Enable UART in Low Power mode wakeup source
  LowPower.enableWakeupFrom(&Serial, SerialWakeup);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  // Triggers an infinite sleep (the device will be woken up only by the registered wakeup sources)
  // The power consumption of the chip will drop consistently
  LowPower.sleep();
}

void SerialWakeup() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
  delay(100);
  while(Serial.available()) {
    char c = Serial.read();
    Serial.print(c);
  }
  Serial.println();
}
