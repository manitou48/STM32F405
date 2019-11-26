##  STM32F405 sketches and such 

adafruit STM32F405@168MHz https://www.adafruit.com/product/4382 

Board also supports circuitpython and micropython.

Files | Description
---|---
adcdma.ino | free-running ADC (A2) with DMA
adcinternal.ino  |   VBAT, VREF, temperature 
chain.ino | chain 16-bit timers for 32-bit
crc.ino | hardware CRC
dacdmahal.ino | timer controlled DAC output with circular DMA
mem2mem.ino | memory-to-memory DMA
freqcount.ino | count external pulses
rng.ino | hardware random number generator
rnghal.ino | HAL version hardware random number generator
rtc.ino |  RTC tests
spiperf.ino | SPI2 DMA

--------
Some CoreMark comparisons at various clock speeds. To change
frequency on STM32F405, modified
PLLN and PLLP in variants/FEATHER_F405/variant.cpp.

![coremark](405t3.png)
![coremark power](405t3a.png)

Some performance comparisons at

   https://github.com/manitou48/DUEZoo

Reference
  https://www.adafruit.com/product/4382 
