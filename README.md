##  STM32F405 sketches and such 

adafruit STM32F405@168MHz https://www.adafruit.com/product/4382 

Board also supports circuitpython and micropython.

Files | Description
---|---
adcdma.ino | free-running ADC (A2) with DMA
adcinternal.ino  |   VBAT, VREF, temperature 
capturedma.ino | TIM2 capture, sampling mode, DMA TBD
chain.ino | chain 16-bit timers for 32-bit
crc.ino | hardware CRC
dacdmahal.ino | timer controlled DAC output with circular DMA
dacdmapong.ino | timer controlled DAC output with double-buffered DMA
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

circuitpython pin toggle period (us) and floating point benchmarks
```
           sinperf   fact   llutm  toggle   raytrace(32x32)
pyboard     13679    2129    1593    13.9 us   5.384 s   micropython v1.5
STM32F405   13114    1885    1984              5.123 s   micropython v1.11
STM32F405   59997    9995    5005    38.4     24.636 s   circuitpython
M4          21973    4028    1984    21.4     10.785 s
T4          31006    3662    5371    43.6     13.178 s

		   toggle circuit playground express  100 us   SAMD21
```
Reference
  https://www.adafruit.com/product/4382 
