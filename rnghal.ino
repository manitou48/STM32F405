// HAL RNG need to ENABLE in variant.h
#define REPS 50

RNG_HandleTypeDef hrng;

void setup() {
  uint32_t val, us;
  Serial.begin(9600);
  __RNG_CLK_ENABLE();
  hrng.Instance = RNG;
  HAL_RNG_Init(&hrng);
  HAL_RNG_GenerateRandomNumber(&hrng, &val);
  Serial.println(val, HEX);
}

void loop() {
  uint32_t t, r;
  int i;

  t = micros();
  for (i = 0; i < REPS; i++) HAL_RNG_GenerateRandomNumber(&hrng, &r);
  t = micros() - t;
  float bps = REPS * 32.e6 / t;
  Serial.println(bps, 2);
  Serial.println(r, HEX);

  delay(2000);
}
