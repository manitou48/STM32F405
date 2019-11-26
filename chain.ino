// chain/cascade 2 16-bit timers  TIM3 TIM4

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

void setup() {
  Serial.begin(9600);
  while (!Serial);
  /* enable the timer peripherals: */
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;

  /* Master - TIM3 counts the lower 16 bits */
  /* TIM3 should use the clock as its input by default.*/
  TIM3->PSC = 0; /* no prescaler. */
  TIM3->CR1 = 0;
  TIM3->CR2 |= 0x20; /* MMS (6:4) */

  /* slave - TIM4 counter the upper 16 bits */
  TIM4->PSC = 0;
  TIM4->CR1 = 0;
  TIM4->SMCR |= (TIM_TS_ITR2 | TIM_SLAVEMODE_EXTERNAL1);
  TIM3->CNT = 0;
  TIM4->CNT = 0;
  /* enable the two counters: */
  TIM4->CR1 |= TIM_CR1_CEN;
  TIM3->CR1 |= TIM_CR1_CEN;

  PRREG(TIM3->CR2);
  PRREG(TIM4->SMCR);
}

void loop() {
  uint16_t lsw, lsw_1, lsw_2, msw;
  uint32_t ticks;

  lsw_1 = TIM3->CNT;
  msw = TIM4->CNT;
  lsw_2 = TIM3->CNT;
  /* has TIM3 rolled over between its first and second reading? */
  if (lsw_2 < lsw_1) {
    /* rollover has happened. lsw_2 is read post-rollover.
       Not sure whether msw_1 was read pre- or post-rollover */
    lsw = lsw_2;
    msw = TIM4->CNT; /* re-read MSW to be sure we've got it post-rollover */
  } else {
    /* a rollover didn't occur between reading lsw_1 and lsw_2. We can use msw_1 safely */
    lsw = lsw_2;
  }
  Serial.print(msw); Serial.print(" "); Serial.println(lsw);
  delay(1000);

}
