
int	main(void) {
	debug_flash();
  wdt_enable(WDTO_1S);
	
  /* let debug routines init the uart if they want to */
  odDebugInit();


  /* all outputs except INT0 and RxD/TxD */
  /////DDRD = ~(_BV(2) | _BV(1) | _BV(0));  
  DDRD &= ~ (_BV(2) | _BV(1) | _BV(0)); // RxD, TxD, Int0 как входы
  DDR(LCD_RS_PORT)    |= _BV(LCD_RS_PIN);  // а RS, D4..D7, идущие на индикатор - как выходы
  DDR(LCD_DATA0_PORT)    |= _BV(LCD_DATA0_PIN);
  DDR(LCD_DATA1_PORT)    |= _BV(LCD_DATA1_PIN);
  DDR(LCD_DATA2_PORT)    |= _BV(LCD_DATA2_PIN);
  DDR(LCD_DATA3_PORT)    |= _BV(LCD_DATA3_PIN);


  //PORTD = 0;
  PORTD &= ~ (_BV(2) | _BV(1) | _BV(0)); // ќ“ Ћё„ить pullup на PD0/RxD, PD1/TxD, PD2/INT0
  LCD_RS_PORT &= ~(_BV(LCD_RS_PIN));
  LCD_DATA0_PORT &= ~(_BV(LCD_DATA0_PIN));
  LCD_DATA1_PORT &= ~(_BV(LCD_DATA1_PIN));
  LCD_DATA2_PORT &= ~(_BV(LCD_DATA2_PIN));
  LCD_DATA3_PORT &= ~(_BV(LCD_DATA3_PIN));
  
//  PORTC = 0;		   /* no pullups on USB pins */
  USB_DDRPORT(USB_CFG_IOPORTNAME) &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT)); // USB data как входы...
  USB_OUTPORT(USB_CFG_IOPORTNAME) &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT)); // ...и без подт€жки
//  DDRC = ~0;		   /* output SE0 for USB reset */
  USB_DDRPORT(USB_CFG_IOPORTNAME) |= (_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT)); // USB data как выходы
  USB_OUTPORT(USB_CFG_IOPORTNAME) &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT)); // выдадим на USB data нули
  /* USB Reset by device only required on Watchdog Reset */
  _delay_loop_2(40000);   // 10ms


  //DDRC = ~USBMASK;	   /* all outputs except USB data */
  USB_DDRPORT(USB_CFG_IOPORTNAME) &= ~USBMASK; // USB data - теперь входы
  usbInit();


  pwm_init();
  led_init();
  led_on();
//  translate_init();
  DDRB &= ~_BV(3);         /* input S1 */
  PORTB |= _BV(3);         /* with pullup */
  DDRB &= ~_BV(0);         /* input S2 */
  PORTB |= _BV(0);         /* with pullup */

  /* try to init two controllers */
  lcd_init(LCD_DISP_ON);
  /////if(lcd_init(LCD_CTRL_0)) controller |= LCD_CTRL_0;
  /////if(lcd_init(LCD_CTRL_1)) controller |= LCD_CTRL_1;

  /* put string to display (line 1) with linefeed */
  /////if(controller & LCD_CTRL_0)
  /////  lcd_puts("LCD2USB V" VERSION_STR);
  lcd_puts("LCD2USB V" VERSION_STR);
  uchar c;
  for (c = 0; c < 16; c++) {
	lcd_putc(c + 128);
  }
  lcd_putc_translated('\n');
  for (c = 0; c < 16; c++) {
	lcd_putc(c + 224 + 16);
  }
/////  if(controller & LCD_CTRL_1)
/////    lcd_puts_translated("2nd ctrl");

/////  if((controller & LCD_CTRL_0) && (controller & LCD_CTRL_1))
/////    lcd_puts_translated(" both!");

  sei();
  for(;;) {	/* main event loop */
    wdt_reset();
    usbPoll();
  }
  return 0;
}

