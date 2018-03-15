# STM32LowPower
Arduino library to support Low Power

## API

* **`void begin()`**: configure the Low Power

* **`void idle(uint32_t millis)`**: enter in idle mode  
**param** millis (optional): number of milliseconds before to exit the mode. At least 1000 ms. The RTC is used in alarm mode to wakeup the chip in millis milliseconds.

* **`void sleep(uint32_t millis)`**: enter in sleep mode  
**param** millis (optional): number of milliseconds before to exit the mode. At least 1000 ms. The RTC is used in alarm mode to wakeup the chip in millis milliseconds.

* **`void deepSleep(uint32_t millis)`**: enter in deepSleep mode  
**param** millis (optional): number of milliseconds before to exit the mode. At least 1000 ms. The RTC is used in alarm mode to wakeup the chip in millis milliseconds.

* **`void shutdown(uint32_t millis)`**: enter in shutdown mode
**param** millis (optional): number of milliseconds before to exit the mode. At least 1000 ms. The RTC is used in alarm mode to wakeup the board in millis milliseconds.

* **`void attachInterruptWakeup(uint32_t pin, voidFuncPtrVoid callback, uint32_t mode)`**: Enable GPIO pin in interrupt mode. If the pin is a wakeup pin, it is configured as wakeup source (see board documentation).  
**param** pin: pin number  
**param** callback: pointer to callback  
**param** mode: interrupt mode (HIGH, LOW, RISING, FALLING or CHANGE)  

* **`void enableWakeupFrom(HardwareSerial *serial, voidFuncPtrVoid callback)`**: enable a UART peripheral in low power mode. See board documentation for low power mode compatibility.  
**param** serial: pointer to a UART  
**param** callback: pointer to a callback to call when the board is waked up.  

* **`void enableWakeupFrom(TwoWire *wire, voidFuncPtrVoid callback)`**:
enable an I2C peripheral in low power mode. See board documentation for low power mode compatibility.  
**param** wire: pointer to I2C  
**param** callback: pointer to a callback to call when the board is waked up.  

* **`void enableWakeupFrom(STM32RTC *rtc, voidFuncPtr callback)`**
attach a callback to the RTC peripheral.  
**param** rtc: pointer to RTC  
**param** callback: pointer to a callback to call when the board is waked up.  

`Begin()` function must be called at least once before `idle()`, `sleep()`, `deepSleep()` or `shutdown()` functions.  

`attachInterruptWakeup()` or `enableWakeupFrom()` functions should be called before `idle()`, `sleep()`, `deepSleep()` or `shutdown()` functions.  

The board will restart when exit the deepSleep or shutdown mode.  

## Hardware state

* **Idle mode**: low wake-up latency (µs range) (e.g. ARM WFI). Memories and
voltage supplies are retained. Minimal power saving mainly on the core itself.

* **sleep mode**: low wake-up latency (µs range) (e.g. ARM WFI), Memories and
voltage supplies are retained. Minimal power saving mainly on the core itself but
higher than idle mode.

* **deep sleep mode**: medium latency (ms range), clocks are gated to reduced. Memories
and voltage supplies are retained. If supported, Peripherals wake-up is possible (UART, I2C ...).

* **shutdown mode**: high wake-up latency (posible hundereds of ms or second
timeframe), voltage supplies are cut except always-on domain, memory content
are lost and system basically reboots.

## Source

You can find the source files at  
https://github.com/stm32duino/STM32LowPower
