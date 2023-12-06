void setUpWDT() {
  // Disable and clear any pending watchdog timer interrupts
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);

  // Set priority and enable watchdog timer interrupts
  NVIC_SetPriority(WDT_IRQn, 0); 
  NVIC_EnableIRQ(WDT_IRQn);

  // Configure and enable WDT GCLK
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0x5);
  GCLK->GENCTRL.reg |= GCLK_GENCTRL_GENEN;
  GCLK->GENCTRL.reg |= GCLK_GENCTRL_SRC(0x03);
  GCLK->GENCTRL.reg |= GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY);
  
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(0x5);
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_ID(0x03);

  // Configure and enable WDT
  WDT->CONFIG.reg = 0x9;
  WDT->EWCTRL.reg = 0x8;
  WDT->CTRL.reg = WDT_CTRL_ENABLE;
  while (WDT->STATUS.bit.SYNCBUSY);

  // Enable early warning interrupts on WDT
  WDT->INTENSET.reg = WDT_INTENSET_EW;
}

void WDT_Handler() {
  WDT->INTFLAG.reg = WDT_INTFLAG_EW;

  Serial.println("Watchdog reset might happen");
}

void petWDT () {
  WDT->CLEAR.reg = 0xA5;
}
