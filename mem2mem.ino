// mem2mem from pyboard, DMA2, 32-bits


#define BYTES 2048
#define WORDS BYTES/sizeof(int)
char src[BYTES] __attribute__ ((aligned (8)));
char dst[BYTES] __attribute__ ((aligned (8)));
int *srcw = (int *) src, *dstw = (int *)dst;

unsigned int t1;

void prmbs(char *lbl, int us, int bits) {
  char str[64];
  float mbs = (float)bits / us;
  sprintf(str, "%s %.2f mbs %d us\r\n", lbl, (double)mbs, us);
  Serial.print(str);
}

void setup() {
  Serial.begin(9600);
  delay(2000);
  int i;
  char str[64];
  DMA_HandleTypeDef DMA_Handle;

  __DMA2_CLK_ENABLE();
  DMA_Handle.Instance = DMA2_Stream0;

  // Need to deinit DMA first
  DMA_Handle.State = HAL_DMA_STATE_READY;
  HAL_DMA_DeInit(&DMA_Handle);

  DMA_Handle.Init.Channel = DMA_CHANNEL_0;
  DMA_Handle.Init.Direction = DMA_MEMORY_TO_MEMORY;
  DMA_Handle.Init.PeriphInc = DMA_PINC_ENABLE;
  DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
  DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  DMA_Handle.Init.Mode = DMA_NORMAL;
  DMA_Handle.Init.Priority = DMA_PRIORITY_HIGH;
  DMA_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  DMA_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  DMA_Handle.Init.MemBurst = DMA_MBURST_SINGLE;
  DMA_Handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
  //  DMA_Handle.Init.MemBurst = DMA_MBURST_INC4;
  //  DMA_Handle.Init.PeriphBurst = DMA_PBURST_INC4;
  HAL_DMA_Init(&DMA_Handle);

  t1 = micros();
  memcpy(dst, src, BYTES);
  t1 = micros() - t1;
  prmbs("memcpy", t1, BYTES * 8);

  int errs = 0;
  memset(dst, 0, BYTES);
  for (i = 0; i < BYTES; i++) src[i] =  i;
  t1 = micros();
  HAL_DMA_Start(&DMA_Handle, (uint32_t)srcw, (uint32_t)dstw, WORDS);
  HAL_DMA_PollForTransfer(&DMA_Handle, HAL_DMA_FULL_TRANSFER , 2000);
  t1 = micros() - t1;
  prmbs("dma", t1, BYTES * 8);
  for (i = 0; i < BYTES; i++) if (dst[i] !=  i % 256)errs++;
  sprintf(str, "errs %d\r\n", errs);
  Serial.print(str);

}

void loop() {
  // put your main code here, to run repeatedly:

}
