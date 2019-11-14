// STM32F405 SPI basic and DMA Tx  from pyboard SPI
// SPI2 (max 21mhz) TX is DMA1 channel 0 stream 4
#include <SPI.h>

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define CS 10

#define SPI_BUFF_SIZE 1024
uint8_t rx_buffer[SPI_BUFF_SIZE];
uint8_t tx_buffer[SPI_BUFF_SIZE];

static SPI_HandleTypeDef SpiHandle;

static void spiInit() {
  SpiHandle.Instance = SPI2;

  __HAL_SPI_DISABLE(&SpiHandle);

  SpiHandle.Init.Mode              = SPI_MODE_MASTER;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;  // 2 is 45 mhz    32 is 2.81mhz
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;  // mode 0
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;

  if (HAL_SPI_Init(&SpiHandle) != HAL_OK) {
    Serial.println("Cannot initialize SPI");
    while (1);
  }

  __HAL_SPI_ENABLE(&SpiHandle);
}

DMA_HandleTypeDef tx_DMA_Handle, rx_DMA_Handle;

static void dmaInit() {
  __DMA1_CLK_ENABLE();
  tx_DMA_Handle.Instance = DMA1_Stream4;

  // Need to deinit DMA first
  tx_DMA_Handle.State = HAL_DMA_STATE_READY;
  HAL_DMA_DeInit(&tx_DMA_Handle);

  tx_DMA_Handle.Init.Channel = DMA_CHANNEL_0;
  tx_DMA_Handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
  tx_DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
  tx_DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
  tx_DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  tx_DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  tx_DMA_Handle.Init.Mode = DMA_NORMAL;
  tx_DMA_Handle.Init.Priority = DMA_PRIORITY_LOW;
  tx_DMA_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  tx_DMA_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  tx_DMA_Handle.Init.MemBurst = DMA_MBURST_INC4;
  tx_DMA_Handle.Init.PeriphBurst = DMA_PBURST_INC4;
  HAL_DMA_Init(&tx_DMA_Handle);
  //__HAL_LINKDMA(spiHandle, DMA_Handle, tx_DMA_Handle);   // TODO macro
  tx_DMA_Handle.Parent = &SpiHandle;
  SpiHandle.hdmatx = &tx_DMA_Handle;
  SpiHandle.hdmarx = NULL;
}


static void  spiSend(uint8_t *data, uint16_t bytes) {
  // ? need SPI handle
  HAL_SPI_Transmit_DMA(&SpiHandle, data, bytes);  // TODO
  HAL_DMA_PollForTransfer(&tx_DMA_Handle, HAL_DMA_FULL_TRANSFER , 2000);
  // while (tx_DMA_Handle.Instance->CR & DMA_SxCR_EN);   // spin
}

void spitst(int mhz) {
  uint32_t t1;
  double mbs;
  char str[64];

  SPI.beginTransaction(SPISettings(mhz * 1000000, MSBFIRST, SPI_MODE0));
  for (int i = 0; i < SPI_BUFF_SIZE; i++) tx_buffer[i] = i;
  digitalWrite(CS, LOW);
  t1 = micros();
  SPI.transfer(tx_buffer, SPI_BUFF_SIZE);
  t1 = micros() - t1;
  digitalWrite(CS, HIGH);
  mbs = 8 * SPI_BUFF_SIZE / (float)t1;
  sprintf(str, "clock %d MHz  %d us  %.2f mbs CR1 %0X ", mhz, t1, mbs, SPI2->CR1);
  Serial.println(str);
  SPI.endTransaction();
}

void setup() {
  char str[64];

  Serial.begin(9600); while (!Serial);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  Serial.println(); Serial.print(F_CPU); Serial.print(" ");
  Serial.print(__TIME__); Serial.print(" "); Serial.println(__DATE__);
  SPI.begin();

  Serial.println("begin");
  spitst(4);
  spitst(8);
  spitst(16);
  spitst(24);
  spitst(32);

  spiInit();
  dmaInit();
  digitalWrite(CS, LOW);
  uint32_t us = micros();
  spiSend(tx_buffer, SPI_BUFF_SIZE);
  us = micros() - us;
  digitalWrite(CS, HIGH);
  sprintf(str, "DMAspi  %d us %.3f mbs  CR1 %0x", us, 8.*SPI_BUFF_SIZE / us, SPI2->CR1);
  Serial.println(str);
}

void loop() {

}
