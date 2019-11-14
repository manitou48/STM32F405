// CRC

#define BUFSIZE 8*1024
uint8_t buf[BUFSIZE];

#define REPS 1000


__attribute__((always_inline))  uint32_t rbit(uint32_t x) {
  uint32_t y;
  asm("rbit %0,%1" : "=r" (y) : "r" (x));
  return y;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  uint32_t x;
  x = 0x41414141;
  asm("rbit %1,%0" : "=r" (x) : "r" (x));
  Serial.println(x, HEX);
  asm("rbit %1,%0" : "=r" (x) : "r" (x));
  Serial.println(x, HEX);
  x = rbit(x);
  Serial.println(x, HEX);
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN ;  // enable CRC
  CRC->CR = 1;
  //while (CRC->DR != ~0); // wait for reset
  while (CRC->CR & 1);
  CRC->DR = rbit(0x41414141);   // AAAA   crc should be 9b0d08f1
  Serial.println(~rbit(CRC->DR), HEX);
  CRC->CR |= 1;
  while (CRC->DR != ~0); // wait for reset
  CRC->DR = 0;    // should be 2144DF1C
  Serial.println(~0 ^ CRC->DR, HEX);

  //Fill array with testdata
  for (uint16_t i = 0; i < BUFSIZE; i++) {
    buf[i] = (i + 1) & 0xff;
  }
}

void loop() {
  uint32_t t, r;
  int i;

  CRC->CR = 1;
  while (CRC->DR != ~0); // wait for reset
  t = micros();
  for (i = 0; i < REPS; i++) CRC->DR = rbit(i);
  r = ~rbit(CRC->DR);
  t = micros() - t;

  printf("val %x %d us  %d reps\n", r, t, REPS);

  CRC->CR = 1;
  while (CRC->DR != ~0); // wait for reset
  uint32_t *p = (uint32_t *) buf;
  t = micros();
  for (i = 0; i < BUFSIZE / 4; i++) CRC->DR = rbit(p[i]);
  for (i = 0; i < BUFSIZE / 4; i++) CRC->DR = rbit(p[i]); // 16k
  r = ~rbit(CRC->DR);   // should be 0x1271457f
  t = micros() - t;
  printf("%d bytes %x  %d us\n", 2 * BUFSIZE, r, t);
  delay(2000);

}
