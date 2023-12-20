/*
  binAlarmTimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip in deep sleep mode.
  when the RTC is configured in BINary mode (BIN_ONLY)

  In this sketch:
  - RTC in BINARY only for the stm32 that supports this mode
  - Alarm is set to wake up the processor each 'atime' and called a custom alarm callback
  which increment a value and reload alarm with 'atime' offset.

  This example code is in the public domain.
*/

#include "STM32LowPower.h"
#include <STM32RTC.h>

/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

/* Change this value to set alarm match offset in millisecond */
static uint32_t atime = 600;
static uint32_t time_ts; /* value to get the binary time */

// Declare it volatile since it's incremented inside an interrupt
volatile int alarmMatch_counter = 0;

void setup() {
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  // By default the LSI is selected as source.
  // rtc.setClockSource(STM32RTC::LSE_CLOCK);
  // Select the STM32RTC::MODE_BIN
  rtc.setBinaryMode(STM32RTC::MODE_BIN);
  rtc.begin(true); /* reset the RTC else the mode is not changed */

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(" Start !");

  // Configure low power
  LowPower.begin();
  LowPower.enableWakeupFrom(&rtc, alarmMatch, &atime);

  rtc.getEpoch(&time_ts);

  // Configure first alarm in 2 seconds then it will be done in the rtc callback
  rtc.setAlarmEpoch(0, STM32RTC::MATCH_SUBSEC, (time_ts + 2000), STM32RTC::ALARM_A);
}

void loop() {
  Serial.print("Alarm Match: ");
  Serial.print(alarmMatch_counter);
  Serial.println(" times.");
  Serial.flush();
  digitalWrite(LED_BUILTIN, HIGH);
  LowPower.deepSleep();
  digitalWrite(LED_BUILTIN, LOW);
  LowPower.deepSleep();
}

void alarmMatch(void* data) {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
  uint32_t _millis = 1000;

  if (data != NULL) {
    _millis = *(uint32_t*)data;
  }

  rtc.getEpoch(&time_ts);
  alarmMatch_counter++;
  rtc.setAlarmEpoch(0, STM32RTC::MATCH_SUBSEC, (time_ts + _millis), STM32RTC::ALARM_A);
}
