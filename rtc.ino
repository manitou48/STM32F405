// stm32f405 test version pre RTC software
//  RTC alarm A B  on EXTI line 17

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)

#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)
#define BYTE2BCD(byte)      ((byte % 10) | ((byte / 10) << 4))
#define BCD2BYTE(byte)      ((byte & 0x0f) + 10*((byte >> 4) & 0x0f))
static int secsinc;  // interrupt every secsinc seconds

volatile uint32_t ticks;

extern "C" void RTC_Alarm_IRQHandler(void) {
  // both A and B
  if (RTC->ISR & RTC_ISR_ALRAF) {
    ticks++;
    RTC->ISR &= (uint32_t)~RTC_ISR_ALRAF;   // clear wakeup
    EXTI->PR |= EXTI_PR_PR17;
    // update target time
    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    int secs = BCD2BYTE(RTC->ALRMAR);
    int bcdsecs = BYTE2BCD((secs + secsinc) % 60);

    RTC->CR &= ~(RTC_CR_ALRAIE | RTC_CR_ALRAE);  // clear
    while ((RTC->ISR & RTC_ISR_ALRAWF)  == 0);  // wait
    RTC->ALRMAR = RTC_ALRMAR_MSK4  | RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK2 | bcdsecs;
    RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE ;   // enable  ALARM A
    RTC->WPR = 0xff; //  enable write protection
  }
}

void alarm_init(int secs) {
  /* Disable the write protection for RTC registers */
  secsinc = secs % 60;
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  __disable_irq();         // no interrupts
  //  enable wakeup
  EXTI->PR |= EXTI_PR_PR17;    // clear
  EXTI->IMR |= EXTI_IMR_MR17;      // line 17 RTC ALARM EXTI
  EXTI->RTSR |= EXTI_RTSR_TR17;   // rising

  RTC->CR &= ~(RTC_CR_ALRAIE | RTC_CR_ALRAE);  // clear
  while ((RTC->ISR & RTC_ISR_ALRAWF)  == 0);  // wait
  // configure A ALARM  secsinc from now
  int bcdsecs = BYTE2BCD((rtc_ms() / 1000 + secsinc) % 60);
  RTC->ALRMAR = RTC_ALRMAR_MSK4  | RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK2 | bcdsecs;
  RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE ;   // enable  ALARM A

  RTC->WPR = 0xff; //  enable write protection

  __enable_irq();         //Alarm is set, so irqs can be enabled again

  NVIC_DisableIRQ(RTC_Alarm_IRQn);
  NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);
  // NVIC_SetPriority(RTC_Alarm_IRQn,0);
  NVIC_EnableIRQ(RTC_Alarm_IRQn);
}


uint32_t rtc_ms() {
  uint32_t ms, tmpreg, SSticks = (RTC->PRER & RTC_PRER_PREDIV_S) + 1;
  uint8_t hrs, mins, secs, ss;

  do {
    ss = RTC->SSR;  // subseconds  counting down, rollover issue
    tmpreg = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);
    RTC->DR;   // must read DR after TR
  } while (ss != RTC->SSR);

  hrs = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
  mins = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >> 8);
  secs = (uint8_t)RTC_Bcd2ToByte(tmpreg & (RTC_TR_ST | RTC_TR_SU));
  tmpreg = 3600 * hrs + 60 * mins + secs;
  ms = 1000 * tmpreg + 1000 * (SSticks - 1 - ss) / SSticks;
  return ms;
}

void rtc_init() {
  // from pyboard core
  /* Disable the write protection for RTC registers */
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
  // set init mode
  RTC->ISR = (uint32_t)RTC_INIT_MASK;
  while ((RTC->ISR &  RTC_ISR_INITF) == 0); // ? timeout

  // default/reset prescale  256 ticks/sec
  RTC->PRER = 0x00ff | (0x007f) << 16;
  //   RTC->PRER = RTC_PRER_PREDIV_S;     // 32768 ticks/sec
  RTC->DR = 0x162514;     // BCD calendar
  RTC->TR = 0;
  RTC->CR |= RTC_CR_BYPSHAD;   // need do-while if set ?

  // optional calibration
#if 1
  RTC->CR |= RTC_CR_DCE;
  RTC->CALIBR = 0x80 | 31;  //  max is -62 ppm
#endif

  // exit init mode
  RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
  //  enable write protection
  RTC->WPR = 0xff;

}


#define RCC_LSI_ON 1
#define RCC_LSI_RDY 2
#define RCC_RTCCLKSOURCE_LSI ((uint32_t)0x00000200)
#define RCC_RTCCLKSOURCE_LSE ((uint32_t)0x00000100)

void initLSI() {
#if 0
  // setup LSI for RTC use   no HAL support

  // PRREG(RCC->CSR);
  // PRREG(RCC->BDCR);
  RCC->CSR |= RCC_LSI_ON;
  while ((RCC->CSR & RCC_LSI_RDY ) == 0) ;

  // Reset Backup domain
  RCC->BDCR |= RCC_BDCR_BDRST;
  RCC->BDCR &= ~RCC_BDCR_BDRST;

  RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_RTCCLKSOURCE_LSI;

  // PRREG(RCC->CSR);
  //  PRREG(RCC->BDCR);
#else
  // re enable LSE
  // RCC->BDCR |= RCC_BDCR_BDRST;
  // RCC->BDCR &= ~RCC_BDCR_BDRST;

  // RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_RTCCLKSOURCE_LSE;

#endif
}

uint32_t s0;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  pinMode(13, OUTPUT);
  delay(5500);
  // PRREG(RCC->BDCR);
  // PRREG(PWR->CR);
  PWR->CR |= PWR_CR_DBP;
  RCC->BDCR |= RCC_BDCR_BDRST;
  RCC->BDCR &= ~RCC_BDCR_BDRST;
  RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_BDCR_RTCSEL_0 | RCC_BDCR_LSEON;
  initLSI();
  RCC->BDCR |= RCC_BDCR_RTCEN;   // enable RTC
  // PRREG(RCC->BDCR);
  // PRREG(PWR->CR);

  rtc_init();

}

void logger() {
  // check drift with hostdrift
  static long cnt = 0;
  long ms;

  while (Serial.available() < 4);  // wait for host request
  ms = rtc_ms();
  Serial.read();
  Serial.read();
  Serial.read();
  Serial.read();
  Serial.write((uint8_t *)&ms, 4);
  cnt++;
  digitalWrite(13, cnt & 1);
}

void display() {
  uint32_t ms;
  char str[64];

  alarm_init(3);   // RTC alarm every 3 seconds

#if 1
  PRREG(RTC->TR);
  PRREG(RTC->DR);
  PRREG(RTC->SSR);
  PRREG(RTC->ISR);
  PRREG(RTC->CR);
  PRREG(RTC->PRER);
  PRREG(RTC->CALIBR);
  PRREG(RCC->BDCR);
  PRREG(RCC->CSR);
  delay(3000);
  PRREG(RTC->TR);
  PRREG(RTC->DR);
  PRREG(RTC->SSR);
  PRREG(RTC->ISR);
#endif
  while (1) {
    ms = rtc_ms();
    sprintf(str, "%d ticks %lu ms", ticks, ms);
    Serial.println(str);
    delay(5000);
  }
}

void loop() {
  //logger();
  display();
}
