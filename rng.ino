// random RNG
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define REPS 50

uint32_t trng() {
  while ((RNG->SR & RNG_SR_DRDY) == 0); // wait
  return RNG->DR;
}

void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.println("hello");
  delay(2000);
  //RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;   // enable RNG
  __RNG_CLK_ENABLE();

  RNG->CR = RNG_CR_RNGEN;
  PRREG(RNG->CR);
  PRREG(RNG->SR);

}

void loop() {
  uint32_t t, r;
  int i;

  t = micros();
  for (i = 0; i < REPS; i++) r = trng();
  t = micros() - t;
  float bps = REPS * 32.e6 / t;
  Serial.println(bps, 2);
  Serial.println(r, HEX);

  delay(2000);

}
