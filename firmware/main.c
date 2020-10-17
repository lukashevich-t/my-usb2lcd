/* Name: main.c
 * Project: LCD2USB; lcd display interface based on AVR USB driver
 * Author: Till Harbaum
 * Creation Date: 2006-01-20
 * Tabsize: 4
 * Copyright: (c) 2005 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: main.c,v 1.2 2007/01/14 12:12:27 harbaum Exp $
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include <util/delay.h>

#include "lcd.h"

// use avrusb library
#include "usbdrv.h"
#include "oddebug.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 11
#define VERSION_STR "1.11"
// change USB_CFG_DEVICE_VERSION in usbconfig.h as well

#define LED_PORT C
#define LED_PIN 2

#define DDR(x) (*(&x - 1))      /* address of data direction register of port x */ 

/* bitmask of detected lcd controllers */
/////uchar controller = 0;
// EEMEM wird bei aktuellen Versionen der avr-lib in eeprom.h definiert
// hier: definiere falls noch nicht bekannt ("alte" avr-libc)
#ifndef EEMEM
// alle Textstellen EEMEM im Quellcode durch __attribute__ ... ersetzen
#define EEMEM  __attribute__ ((section (".eeprom")))
#endif

/* ------------------------------------------------------------------------- */
/* PWM units are used for contrast and backlight brightness */

/* contrast and brightness are stored in eeprom */
uchar eeprom_valid EEMEM;
uchar eeprom_contrast EEMEM;
uchar eeprom_brightness EEMEM;
// флаг запуска bootloader'а (пока не использую никак)
uchar eeprom_bootload EEMEM;

// таблица перекодировки русских символов
uchar translation_table[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
        0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43,
        0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b,
        0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
        0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b,
        0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3,
        0xa4, 0xa5, 0xa6, 0xa7, 162 /*Ё*/, 0xa9, 0xaa, 200 /*«*/, 201 /*»*/,
        0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
        181 /*ё*/, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0x41 /*А*/,
        160 /*Б*/, 0x42 /*В*/, 161 /*Г*/, 224 /*Д*/, 0x45/*Е*/, 163 /*Ж*/,
        164 /*З*/, 165 /*И*/, 166 /*Й*/, 0x4b /*К*/, 167 /*Л*/, 0x4d /*М*/,
        0x48 /*Н*/, 0x4f /*О*/, 168 /*П*/, 0x50 /*Р*/, 0x43 /*С*/, 0x54 /*Т*/,
        169 /*У*/, 170 /*Ф*/, 0x58 /*Х*/, 225 /*Ц*/, 171 /*Ч*/, 172 /*Ш*/,
        226/*Щ*/, 173 /*Ъ*/, 174 /*Ы*/, 0x62 /*Ь*/, 175 /*Э*/, 176 /*Ю*/,
        177/*Я*/, 0x61 /*а*/, 178 /*б*/, 179 /*в*/, 180 /*г*/, 227 /*д*/,
        0x65 /*е*/, 182 /*ж*/, 183 /*з*/, 184 /*и*/, 185 /*й*/, 186 /*к*/,
        187 /*л*/, 188 /*м*/, 189 /*н*/, 0x6f /*о*/, 190 /*п*/, 0x70 /*р*/,
        0x63/*с*/, 191 /*т*/, 0x79 /*у*/, 228 /*ф*/, 0x78 /*х*/, 229 /*ц*/,
        192 /*ч*/, 193 /*ш*/, 230 /*щ*/, 194 /*ъ*/, 195 /*ы*/, 196 /*ь*/,
        197 /*э*/, 198 /*ю*/, 199 /* я */
};

static void initForUsbConnectivity(void) {
    uchar i = 0;

#if F_CPU == 12800000
    TCCR0 = 3;          /* 1/64 prescaler */
#endif
    usbInit();
    cli();
    /* enforce USB re-enumerate: */
    usbDeviceDisconnect(); /* do this while interrupts are disabled */
    do { /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    } while (--i);
    usbDeviceConnect();
    sei();
}

void pwm_init(void) {

    /* check if eeprom is valid and set default values if not */
    /* initial values: full contrast and full brightness */
    if (eeprom_read_byte(&eeprom_valid) != 0x42) {

        /* write magic "eeprom is valid" marker 0x42 and default values */
        eeprom_write_byte(&eeprom_valid, 0x42);
        eeprom_write_byte(&eeprom_contrast, 0xff);
        eeprom_write_byte(&eeprom_brightness, 0xff);
    }

    /* PortB: set DDB1 and DDB2 => PORTB1 and PORTB2 are output */
    DDRB |= _BV(1) | _BV(2);

    /* Set Timer1:
     - Fast PWM,8bit => Mode 5 (WGM13=0,WGM12=1,WGM11=0,WGM10=1)
     - Output-Mode: lower voltage is higher contrast
     => Set OC1A on Compare Match, clear OC1A at BOTTOM, (inverting mode)
     COM1A1=1,COM1A0=1
     higher voltage is higher brightness
     => Clear OC1B on Compare Match, set OC1B at BOTTOM, (non-inverting mode)
     COM1B1=1,COM1B0=0
     - Timer runs at inernal clock with no prescaling => CS12=0,CS11=0,CS10=1
     */
    TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS10);

    TIMSK &= ((~_BV(2)) & (~_BV(3)) & (~_BV(4)) & (~_BV(5)));

    OCR1A = eeprom_read_byte(&eeprom_contrast);
    OCR1B = eeprom_read_byte(&eeprom_brightness);
}

void set_contrast(uchar value) {
    /* store value in eeprom if it actually changed */
    if (value != eeprom_read_byte(&eeprom_contrast))
        eeprom_write_byte(&eeprom_contrast, value);

    OCR1A = value;  // lower voltage is higher contrast
}

void set_brightness(uchar value) {
    /* store value in eeprom if it actually changed */
    if (value != eeprom_read_byte(&eeprom_brightness))
        eeprom_write_byte(&eeprom_brightness, value);

    OCR1B = value;  // higher voltage is higher brightness
}

/* ------------------------------------------------------------------------- */

uchar usbFunctionSetup(uchar data[8]) {
    static uchar replyBuf[4];
    usbMsgPtr = replyBuf;
    uchar len = (data[1] & 3) + 1;       // 1 .. 4 bytes 
    uchar target = (data[1] >> 3) & 3; // target 0 .. 3
    uchar i;

    // request byte:

    // 7 6 5 4 3 2 1 0
    // C C C T T R L L

    // TT = target bit map 
    // R = reserved for future use, set to 0
    // LL = number of bytes in transfer - 1 

    switch (data[1] >> 5) {

    case 0: // echo (for transfer reliability testing)
        replyBuf[0] = data[2];
        replyBuf[1] = data[3];
        return 2;
        break;

    case 1: // command
        for (i = 0; i < len; i++)
            lcd_command(data[2 + i]);
        break;

    case 2: // data
        for (i = 0; i < len; i++)
            // Возможно, следующая запись будет глючить
            // возможно, эти данные являются не только символами, но и параметрами команд...
            // короче, нужно еще тестировать в реале...
            lcd_data(translation_table[data[2 + i]]);
        //lcd_data(data[2+i]);
        break;

    case 3: // set
        switch (target) {

        case 0:  // contrast
            set_contrast(data[2]);
            break;

        case 1:  // brightness
            set_brightness(data[2]);
            break;

        default:
            // must not happen ...
            break;
        }
        break;

    case 4: // get
        switch (target) {
        case 0: // version
            replyBuf[0] = VERSION_MAJOR;
            replyBuf[1] = VERSION_MINOR;
            return 2;
            break;

        case 1: // keys
            replyBuf[0] = ((PINB & _BV(0)) ? 0 : 1) | ((PINB & _BV(3)) ? 0 : 2);
            replyBuf[1] = 0;
            return 2;
            break;

        case 2: // controller map
            replyBuf[0] = 1 << 0;
            replyBuf[1] = 0;
            return 2;
            break;

        default:
            // must not happen ...
            break;
        }
        break;
    case 5: // initialize bootloader
        startBootloader();
        break;
    default:
        // must not happen ...
        break;
    }

    return 0;  // reply len
}

/* ------------------------------------------------------------------------- */

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] =
        { /* USB report descriptor */
        0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
                0x09, 0x01,                    // USAGE (Vendor Usage 1)
                0xa1, 0x01,                    // COLLECTION (Application)
                0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
                0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
                0x75, 0x08,                    //   REPORT_SIZE (8)
                0x95, 0x08,                    //   REPORT_COUNT (8)
                0x09, 0x00,                    //   USAGE (Undefined)  
                0x82, 0x02, 0x01,              //   INPUT (Data,Var,Abs,Buf)
                0x95, HIDSERIAL_INBUFFER_SIZE, //   REPORT_COUNT (32)
                0x09, 0x00,                    //   USAGE (Undefined)        
                0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
                0xc0                           // END_COLLECTION
        };

void lcd_putc_translated(uchar c) {
    lcd_putc(translation_table[c]);
}

void lcd_puts_translated(const uchar *s) {
    register char c;

    while ((c = *s++)) {
        lcd_putc_translated(c);
    }

}/* lcd_puts_translated */

// опрос кнопки и действие по ее нажатию:
void pollButton() {
    if ((PINB & (1 << 3)) == 0) {
        startBootloader();
    }
}

void startBootloader() {
    lcd_clrscr();
    lcd_puts("bootloader starting");
    // eeprom_write_byte(&eeprom_bootload, 0x0); // пишем в ячейку EEPROM нулевое число
    // _delay_ms(50);		// ждём завершения операции записи
    cli();
    while (1) {
    };
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void) {
    char buf[16];
    wdt_enable (WDTO_1S);
//	debug_flash();

    PORTB = _BV(0) | _BV(3); /* activate pull-up for key *//*кнопки на пинах PB0 и PB3 */
    odDebugInit();
    DBG1(0x00, 0, 0);
    wdt_reset();
    pwm_init();
    wdt_reset();
    initForUsbConnectivity();
    lcd_init (LCD_DISP_ON);
    set_contrast(0xbf);
    set_brightness(0x7f);
    wdt_reset();
    lcd_puts_translated("LCD2USB " VERSION_STR "\nЛукашевич Т.В.");
    sei();

    for (;;) { /* main event loop */
        wdt_reset();
        usbPoll();
        pollButton();
    }
}

/* ------------------------------------------------------------------------- */
