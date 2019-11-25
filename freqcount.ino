// frequency count from external pin
// TIM2 and TIM5 32-bit timers
// test with PWM pin 10 (TIM4)  jumper to 6
// TIM3_CH1 AF2 PC6 pin 6   16-bit
// TODO 32-bit counter via oflow interrupt or chain timers ?


#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define TIMx TIM3


// clock timer's external pin

void timx_init() {

  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   // power up TIM3
  RCC->AHB1ENR |=  RCC_AHB1ENR_GPIOCEN;
  TIMx->CR1 = 0x80;  // disable
  GPIOC->MODER &= ~(3 << 12); // clear PC6 mode
  GPIOC->MODER |= 2 << 12; // AF mode for PC6
  GPIOC->AFR[0] |= 2 << 24; // AF2 for PC6

  TIMx->CCER = 0;        //rising
  TIMx->CCMR1 = 0x01;   // CC1S(1) ch1 TI1
  TIMx->SMCR = 0x57;   //  TS(5) TI1 edge detection SMS(7) ext
  TIMx->SR = 0;
  TIMx->ARR = 0xffffffff;  // ? need
  TIMx->CNT = 0;
  TIMx->CR1 = 0x81;  // enable
  PRREG(RCC->APB1ENR);
  PRREG(RCC->AHB1ENR);
  PRREG(GPIOC->MODER);
  PRREG(GPIOC->AFR[0]);
  PRREG(TIMx->SMCR);
  PRREG(TIMx->CCMR1);
  PRREG(TIMx->CCER);
  PRREG(TIMx->DIER);
  PRREG(TIMx->SR);
  PRREG(TIMx->CR1);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  Serial.println("starting");
  delay(2000);

  analogWriteFrequency( 50 * 1000); 
  analogWrite(10, 128);  // duty

  timx_init();
}

void loop() {
  TIMx->CNT = 0;
  delay(1000);
  uint32_t pulses = TIMx->CNT;
  Serial.println(pulses);
}
