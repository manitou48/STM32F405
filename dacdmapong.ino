// 405 DAC  TIM6 double buffered DMA
// A0 DAC1  TIM6  DMA1 chnl 7 stream 5
// observe DAC1 with scope, or jumper to an ADC pin and sample
// double-buffer ping pong  half full interrupts
// another options is wit DBM and two addresses  TODO
// i couldn't figure out HAL methods

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
# define MAX(x,y) ((x)>(y)?(x):(y))

static volatile uint16_t sinetable[] = {
  2047,    2147,    2248,    2348,    2447,    2545,    2642,    2737,
  2831,    2923,    3012,    3100,    3185,    3267,    3346,    3422,
  3495,    3564,    3630,    3692,    3750,    3804,    3853,    3898,
  3939,    3975,    4007,    4034,    4056,    4073,    4085,    4093,
  4095,    4093,    4085,    4073,    4056,    4034,    4007,    3975,
  3939,    3898,    3853,    3804,    3750,    3692,    3630,    3564,
  3495,    3422,    3346,    3267,    3185,    3100,    3012,    2923,
  2831,    2737,    2642,    2545,    2447,    2348,    2248,    2147,
  2047,    1948,    1847,    1747,    1648,    1550,    1453,    1358,
  1264,    1172,    1083,     995,     910,     828,     749,     673,
  600,     531,     465,     403,     345,     291,     242,     197,
  156,     120,      88,      61,      39,      22,      10,       2,
  0,       2,      10,      22,      39,      61,      88,     120,
  156,     197,     242,     291,     345,     403,     465,     531,
  600,     673,     749,     828,     910,     995,    1083,    1172,
  1264,    1358,    1453,    1550,    1648,    1747,    1847,    1948,
};

#define FREQHZ 128000

volatile uint32_t ticks;
extern "C" void DMA1_Stream5_IRQHandler()
{
  ticks++;
  if (DMA1->HISR & DMA_HISR_TCIF5) {
    // half-done interrupt, could update first half of buffer
    digitalWrite(13, HIGH);
  } else {
    // complete interrupt, could update 2nd half of buffer
    digitalWrite(13, LOW);
  }
  DMA1->HIFCR |= DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5;  // clear stream 5
}

// HAL DMA callbacks, reset done by caller
#if 0
void DMA_halfdone(DMA_HandleTypeDef *hdma) {
  digitalWrite(13, HIGH);
}
void DMA_done(DMA_HandleTypeDef *hdma) {
  digitalWrite(13, LOW);
}
#endif
#if 0
// weak in system/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
  digitalWrite(13, HIGH);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
  digitalWrite(13, LOW);
}
#endif


TIM_HandleTypeDef TIM6_Handle;
static DAC_HandleTypeDef DAC_Handle;

TIM_HandleTypeDef *timer_tim6_init(uint freq) {
  // TIM6 clock enable
  __TIM6_CLK_ENABLE();

  // Timer runs at SystemCoreClock / 2
  // Compute the prescaler value so TIM6 triggers at freq-Hz
  uint32_t period = MAX(1, HAL_RCC_GetPCLK2Freq() / freq);
  uint32_t prescaler = 1;
  while (period > 0xffff) {
    period >>= 1;
    prescaler <<= 1;
  }

  // Time base clock configuration
  TIM6_Handle.Instance = TIM6;
  TIM6_Handle.Init.Period = period - 1;
  TIM6_Handle.Init.Prescaler = prescaler - 1;
  TIM6_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // unused for TIM6
  TIM6_Handle.Init.CounterMode = TIM_COUNTERMODE_UP; // unused for TIM6
  HAL_TIM_Base_Init(&TIM6_Handle);

  return &TIM6_Handle;
}

static void TIM6_Config(uint freq) {
  // Init TIM6 at the required frequency (in Hz)
  TIM_HandleTypeDef *tim = timer_tim6_init(freq);

  // TIM6 TRGO selection
  TIM_MasterConfigTypeDef config;
  config.MasterOutputTrigger = TIM_TRGO_UPDATE;
  config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(tim, &config);

  // TIM6 start counter
  HAL_TIM_Base_Start(tim);
}


void setup() {
  Serial.begin(9600);
  while (!Serial);
  pinMode(13, OUTPUT);

  analogWrite(A0, 128); // prep DAC pin

  // DAC init
  memset(&DAC_Handle, 0, sizeof DAC_Handle);
  DAC_Handle.Instance = DAC;
  DAC_Handle.State = HAL_DAC_STATE_RESET;
  // HAL_DAC_RegisterCallback(&DAC_Handle, HAL_DAC_CH1_HALF_COMPLETE_CB_ID, DMA_halfdone);
  // HAL_DAC_RegisterCallback(&DAC_Handle, HAL_DAC_CH1_COMPLETE_CB_ID, DMA_done);

  HAL_DAC_Init(&DAC_Handle);

  TIM6_Config(FREQHZ);

  __DMA1_CLK_ENABLE();
  DMA_HandleTypeDef DMA_Handle;
  DMA_Handle.Instance = DMA1_Stream5;

  // Need to deinit DMA first
  DMA_Handle.State = HAL_DMA_STATE_READY;
  HAL_DMA_DeInit(&DMA_Handle);

  DMA_Handle.Init.Channel = DMA_CHANNEL_7;
  DMA_Handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
  DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
  DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
  DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  DMA_Handle.Init.Mode = DMA_CIRCULAR;
  DMA_Handle.Init.Priority = DMA_PRIORITY_HIGH;
  DMA_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  DMA_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  DMA_Handle.Init.MemBurst = DMA_MBURST_SINGLE;
  DMA_Handle.Init.PeriphBurst = DMA_PBURST_SINGLE;

  // HAL_DMA_RegisterCallback(&DMA_Handle, HAL_DMA_XFER_HALFCPLT_CB_ID, DMA_halfdone);
  // HAL_DMA_RegisterCallback(&DMA_Handle, HAL_DMA_XFER_CPLT_CB_ID, DMA_done);
  HAL_DMA_Init(&DMA_Handle);
  PRREG(DMA1_Stream5->CR);
  // half complete interrupts
  DMA1_Stream5->CR |= DMA_SxCR_HTIE | DMA_SxCR_TCIE;
  DMA1->HIFCR &= ~(0x3D << 6); // clear status stream 5
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  __HAL_LINKDMA(&DAC_Handle, DMA_Handle1, DMA_Handle);
  DAC_ChannelConfTypeDef config;
  config.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  config.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  HAL_DAC_ConfigChannel(&DAC_Handle, &config, DAC_CHANNEL_1);
  HAL_DAC_Start_DMA(&DAC_Handle, DAC_CHANNEL_1,
                    (uint32_t*)sinetable, sizeof(sinetable) / 2, DAC_ALIGN_12B_R);
  PRREG(DMA1_Stream5->CR);
}

void loop() {
  Serial.println(ticks);
  delay(1000);
}
