// TIM capture with DMA
// 32-bit timer with accessible input pin
// Rx PB11 RCC_AHB1ENR_GPIOBEN   TIM2/4 AF1 RCC_APB1ENR_TIM2EN
//   TIM2_CH4  DMA1 Chn 3  stream 6  or 7  RCC_AHB1ENR_DMA1EN
// test with PWM pin 5 jumpered to Rx

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

uint32_t samples[128];

void myinit() {
  // enable peripheral clocks
  RCC->AHB1ENR |=  RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_DMA1EN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  // configure capture pin B11   AF mode  AF1
  GPIOB->MODER &= ~(3 << 22); // clear mode
  GPIOB->MODER |= 2 << 22; // AF mode
  GPIOB->AFR[1] = (GPIOB->AFR[1] & ~(15 << 12)) | 1 << 12; // AF1 for PB11
  // configure timer  1mhz capture TIM2_CH4
  TIM2->CR1 = 0;   // stop timer
  TIM2->PSC = 0;
  TIM2->SR = 0;
  TIM2->CCER = 0;
  TIM2->CCMR2 = 1 << 8; // TI4 input
  TIM2->CCER = 1 << 12; // CH4 capture
  TIM2->ARR = 0xffffffff; //84 mhz
  TIM2->CR1 = TIM_CR1_CEN;   // start timer
  // configure DMA

}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  analogWrite(5, 128);  // test jumper to Rx
  Serial.println("init");
  myinit();
  PRREG(RCC->APB1ENR);
  PRREG(RCC->AHB1ENR);
  PRREG(GPIOB->MODER);
  PRREG(GPIOB->AFR[1]);
  PRREG(TIM2->CCMR2);
  PRREG(TIM2->CCER);
  PRREG(TIM2->DIER);
  PRREG(TIM2->SR);
  PRREG(TIM2->ARR);
  PRREG(TIM2->CR1);
}

void loop() {

  if (TIM2->SR & TIM_SR_CC4IF) {
    uint32_t val = TIM2->CCR4;

    TIM2->SR &= ~TIM_SR_CC4IF;  // clear
    Serial.println(val);
  }
}
