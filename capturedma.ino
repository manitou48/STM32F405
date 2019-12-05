// TIM capture with DMA
// 32-bit timer with accessible input pin
// Rx PB11 RCC_AHB1ENR_GPIOBEN   TIM2/4 AF1 RCC_APB1ENR_TIM2EN
//   TIM2_CH4  DMA1 Chn 3  stream 6  or 7  RCC_AHB1ENR_DMA1EN
// test with PWM pin 5 jumpered to Rx

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define NSAMPLES 128
uint32_t samples[NSAMPLES];
static DMA_HandleTypeDef DMA_Handle;

void myinit() {
  // enable peripheral clocks
  RCC->AHB1ENR |=  RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_DMA1EN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  // configure capture pin B11   AF mode  AF1
  GPIOB->MODER &= ~(3 << 22); // clear mode
  GPIOB->MODER |= 2 << 22; // AF mode
  GPIOB->AFR[1] = (GPIOB->AFR[1] & ~(15 << 12)) | 1 << 12; // AF1 for PB11
  // configure timer  84mhz capture TIM2_CH4
  TIM2->CR1 = 0;   // stop timer
  TIM2->PSC = 0;       //84 mhz
  TIM2->SR = 0;
  TIM2->CCER = 0;
  TIM2->CCMR2 = 1 << 8; // TI4 input
  TIM2->CCER = 1 << 12; // CH4 capture
  TIM2->ARR = 0xffffffff;
  TIM2->DIER |= TIM_DIER_CC4DE;   // DMA on CH4
  TIM2->CR1 = TIM_CR1_CEN;   // start timer

  // configure DMA  TIM2_CH4  DMA1 Chn 3  stream 6  or 7
  DMA_Handle.Instance = DMA1_Stream6;
  DMA_Handle.State = HAL_DMA_STATE_READY;
  HAL_DMA_DeInit(&DMA_Handle);

  DMA_Handle.Init.Channel = DMA_CHANNEL_3;
  DMA_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
  DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
  DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
  DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  DMA_Handle.Init.Mode =  DMA_NORMAL;
  DMA_Handle.Init.Priority = DMA_PRIORITY_HIGH;
  DMA_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  DMA_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  DMA_Handle.Init.MemBurst = DMA_MBURST_SINGLE;
  DMA_Handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
  HAL_DMA_Init(&DMA_Handle);
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
  uint32_t  t1 = micros();
  HAL_DMA_Start(&DMA_Handle, (uint32_t)(&TIM2->CCR4), (uint32_t)samples, NSAMPLES);
  HAL_DMA_PollForTransfer(&DMA_Handle, HAL_DMA_FULL_TRANSFER , 2000);
  t1 = micros() - t1;
  Serial.print(t1); Serial.println(" us");
  for (int i = 0; i < 4; i++) {
    Serial.println(samples[i]);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  analogWrite(5, 128);  // test jumper to Rx
  Serial.println("init");
  myinit();

}

void loop() {
#if 0
  // poll mode
  if (TIM2->SR & TIM_SR_CC4IF) {
    uint32_t val = TIM2->CCR4;

    TIM2->SR &= ~TIM_SR_CC4IF;  // clear
    Serial.println(val);
  }
#endif
}
