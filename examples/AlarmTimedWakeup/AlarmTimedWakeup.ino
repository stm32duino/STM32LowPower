/*
  AlarmTimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip in deep sleep mode,
  when the RTC is configured in BCD or MIX mode (BCD and BINARY)

  In this sketch:
  - RTC is configured in BCD (default) or MIX mode (BCD and BINARY)
  - RTC date and time are configured.
  - Alarm is set to wake up the processor each 'atime' and called a custom alarm callback
  which increment a value and reload alarm with 'atime' offset.

  This example code is in the public domain.
*/

#include "STM32LowPower.h"
#include <STM32RTC.h>

/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

/* Change this value to set alarm match offset in millisecond */
/* Note that STM32F1xx does not manage subsecond only second */
static uint32_t atime = 600;

// Declare it volatile since it's incremented inside an interrupt
volatile int alarmMatch_counter = 0;

// Variables for RTC configurations
static byte seconds = 0;
static byte minutes = 0;
static byte hours = 0;

static byte weekDay = 1;
static byte day = 1;
static byte month = 1;
static byte year = 18;

void setup() {
#if defined(RTC_BINARY_NONE)
  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  // By default the LSI is selected as source.
  // rtc.setClockSource(STM32RTC::LSE_CLOCK);
  // Select the STM32RTC::MODE_BCD or STM32RTC::MODE_MIX
  // By default the STM32RTC::MODE_BCD is selected.
  // rtc.setBinaryMode(STM32RTC::MODE_BCD);
  rtc.begin(true); /* reset the RTC else the binary mode is not changed */
#else
  rtc.begin();
#endif /* RTC_BINARY_NONE */
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(weekDay, day, month, year);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(" Start !");

  // Configure low power
  LowPower.begin();
  LowPower.enableWakeupFrom(&rtc, alarmMatch, &atime);

  // Configure first alarm in 2 second then it will be done in the rtc callback
  rtc.setAlarmEpoch(rtc.getEpoch() + 2);
}

void loop() {
  Serial.print("Alarm Match: ");
  Serial.print(alarmMatch_counter);
  Serial.println(" times.");
  Serial.end();
  digitalWrite(LED_BUILTIN, HIGH);
  LowPower.deepSleep();
  digitalWrite(LED_BUILTIN, LOW);
  LowPower.deepSleep();
  Serial.begin(115200);
  while (!Serial) {}
}

void alarmMatch(void* data) {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
  uint32_t epoc;
  uint32_t epoc_ms;
  uint32_t sec = 0;
  uint32_t _millis = 1000;

  if (data != NULL) {
    _millis = *(uint32_t*)data;
  }

  sec = _millis / 1000;
#ifdef STM32F1xx
  // Minimum is 1 second
  if (sec == 0) {
    sec = 1;
  }
  epoc = rtc.getEpoch(&epoc_ms);
#else
  _millis = _millis % 1000;
  epoc = rtc.getEpoch(&epoc_ms);

  //Update epoch_ms - might need to add a second to epoch
  epoc_ms += _millis;
  if (epoc_ms >= 1000) {
    sec++;
    epoc_ms -= 1000;
  }
#endif
  alarmMatch_counter++;
  rtc.setAlarmEpoch(epoc + sec, STM32RTC::MATCH_SS, epoc_ms);
}
