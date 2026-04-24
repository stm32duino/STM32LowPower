# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(STM32LowPower INTERFACE)
add_library(STM32LowPower_usage INTERFACE)

target_include_directories(STM32LowPower_usage INTERFACE
  src
)


target_link_libraries(STM32LowPower_usage INTERFACE
  base_config
)

target_link_libraries(STM32LowPower INTERFACE STM32LowPower_usage)



add_library(STM32LowPower_bin OBJECT EXCLUDE_FROM_ALL
  src/low_power.c
  src/STM32LowPower.cpp
)
target_link_libraries(STM32LowPower_bin PUBLIC STM32LowPower_usage)

target_link_libraries(STM32LowPower INTERFACE
  STM32LowPower_bin
  $<TARGET_OBJECTS:STM32LowPower_bin>
)

