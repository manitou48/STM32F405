// ADC DMA on A2
// A2 is PA6 ADC1_IN6   DMA2 chnl 0 stream 4 or 0

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define SAMPLES 1000

static ADC_HandleTypeDef AdcHandle;

void adc_init() {
  // we assume AnalogIn has configureed GPIO, we need ADC channel ?
  __ADC1_CLK_ENABLE();  // Enable ADC clock

  // Configure ADC
  AdcHandle.Instance = (ADC_TypeDef *)ADC1;
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION12b;
  AdcHandle.Init.ScanConvMode          = DISABLE;
  AdcHandle.Init.ContinuousConvMode    = ENABLE;    // DMA
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfDiscConversion   = 0;
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.NbrOfConversion       = 1;
  AdcHandle.Init.DMAContinuousRequests = ENABLE;    // DMA
  AdcHandle.Init.EOCSelection          = DISABLE;
  HAL_ADC_Init(&AdcHandle);
}

static DMA_HandleTypeDef DMA_Handle;

void dma_init() {
  //  DMA init  ADC1 is DMA2 channel0 stream 0 or 4 use DMA2_Stream0  thd

  __DMA2_CLK_ENABLE();
  DMA_Handle.Instance = DMA2_Stream0;
  DMA_Handle.State = HAL_DMA_STATE_READY;
  HAL_DMA_DeInit(&DMA_Handle);

  DMA_Handle.Init.Channel = DMA_CHANNEL_0;
  DMA_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
  DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
  DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
  DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  DMA_Handle.Init.Mode =  DMA_NORMAL;
  DMA_Handle.Init.Priority = DMA_PRIORITY_HIGH;
  DMA_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  DMA_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  DMA_Handle.Init.MemBurst = DMA_MBURST_SINGLE;
  DMA_Handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
  HAL_DMA_Init(&DMA_Handle);

  __HAL_LINKDMA(&AdcHandle, DMA_Handle, DMA_Handle);
}

void adc_readn( uint16_t * data, uint32_t nelems) {
  ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel = ADC_CHANNEL_6;  //  need to derive
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  sConfig.Offset = 0;
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);

  HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)data, nelems);
  while (DMA_Handle.Instance->CR & DMA_SxCR_EN);   // spin
  HAL_ADC_Stop(&AdcHandle);
}

void setup() {
  uint16_t samples[SAMPLES];
  uint32_t t1;
  int i;
  char str[64];

  Serial.begin(9600);
  sprintf(str, "\nF_CPU %d  %s %s", F_CPU, __TIME__, __DATE__);
  Serial.println(str);

  memset(samples, 0, sizeof samples);
  t1 = micros();
  for (i = 0; i < SAMPLES; i++) samples[i] = analogRead(A2);
  t1 = micros() - t1;
  sprintf(str, "loop sample time %.1f us  %d elapsed %d us", (float)t1 / SAMPLES, samples[5], t1);
  Serial.println(str);

  adc_init();
  dma_init();
  memset(samples, 77, sizeof samples);
  t1 = micros();
  adc_readn(samples, SAMPLES);
  t1 = micros() - t1;
  sprintf(str, "DMA sample time %.2f us  %d  elapsed %d us", (float)t1 / SAMPLES, samples[5], t1);
  Serial.println(str);
}

void loop() {

}
