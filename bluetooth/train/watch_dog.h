void setUpWDT() {

  // 32 KHz OSCULP32K
  // GENDIV 100
  // Timeout 8
  // GCLK 3
  // GCLK_WDT = 0x03
  // Disable and clear any pending watchdog timer interrupts
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);

  // Set priority and enable watchdog timer interrupts
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  // Configure and enable WDT GCLK
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0x80) | GCLK_GENDIV_ID(0x3);
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(3) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC(0x03) | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY);
  
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(0x5) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID(0x3);

  // Configure and enable WDT
  WDT->CONFIG.reg = 0x8;
}

void enableWDT() {
  WDT->CTRL.reg = WDT_CTRL_ENABLE;
  while (WDT->STATUS.bit.SYNCBUSY);
}

void WDT_Handler() {
  WDT->INTFLAG.reg = WDT_INTFLAG_EW;
  #ifdef TESTING
  Serial.println("Watchdog reset might happen");
  #endif
}

void petWDT () {
  WDT->CLEAR.reg = 0xA5;
}
