// access STM32F405 internal ADC channels temp sensor, VBAT, VREF
//  based on F446RE and pyboard implementation and analogin_api.c

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define CORE_TEMP_V25          (4096*.76/3.3)  // (0.76v/3.3v)*(2^ADC resoultion)
#define CORE_TEMP_AVG_SLOPE    (4096*.0025/3.3)    // (2.5mv/3.3v)*(2^ADC resoultion)

//  VBAT to ADC goes through divider  /4  ref 13.11
#define VBAT_DIV (4)


static ADC_HandleTypeDef AdcHandle;

void adc_init() {
  // Enable ADC clock
  __ADC1_CLK_ENABLE();

  // Configure ADC
  AdcHandle.Instance = (ADC_TypeDef *)ADC1;
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION12b;
  AdcHandle.Init.ScanConvMode          = DISABLE;
  AdcHandle.Init.ContinuousConvMode    = DISABLE;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfDiscConversion   = 0;
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.NbrOfConversion       = 1;
  AdcHandle.Init.DMAContinuousRequests = DISABLE;
  AdcHandle.Init.EOCSelection          = DISABLE;
  HAL_ADC_Init(&AdcHandle);
}

uint16_t adc_read_channel(uint32_t channel) {
  ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel = channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  sConfig.Offset = 0;
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);

  HAL_ADC_Start(&AdcHandle); // Start conversion

  // Wait end of conversion and get value
  if (HAL_ADC_PollForConversion(&AdcHandle, 10) == HAL_OK) {
    return (HAL_ADC_GetValue(&AdcHandle));
  } else {
    return 0;
  }
}

void setup() {
  uint16_t val;
  float tempc, Vbat, Vcc;
  char str[128];

  Serial.begin(9600);
  delay(2000);
  sprintf(str, "\nF_CPU %d  %s %s", F_CPU, __TIME__, __DATE__);
  Serial.println(str);
  adc_init();

  val = adc_read_channel(ADC_CHANNEL_VBAT);
  Vbat = 3.3 * VBAT_DIV * val / 4096;
  sprintf(str, "%d Vbat %.2f", val, Vbat);
  Serial.println(str);
  ADC->CCR = 0;   // clear CCR to avoid conflicting use
  val = adc_read_channel(ADC_CHANNEL_TEMPSENSOR);
  tempc = ((val - CORE_TEMP_V25) / CORE_TEMP_AVG_SLOPE) + 25.;
  sprintf(str, "%d %.1f C", val, tempc);
  Serial.println(str);
  val = adc_read_channel(ADC_CHANNEL_VREFINT);
  Vcc = 4095 * 1.21 / val;
  sprintf(str, "%d Vcc %.2f", val, Vcc);
  Serial.println(str);

  PRREG(ADC1->CR1);
  PRREG(ADC1->CR2);
  PRREG(ADC->CCR);

}
void loop() {
  // put your main code here, to run repeatedly:

}
