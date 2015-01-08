/*****************************************************************************
*
* CAN Test-APP
*
* Compiler : avr-gcc 4.3.0 / avr-libc 1.6.2 / AVR Studio 4.14
*
* AVR Memory Usage
* ----------------
* Device: at90can128
*
* Program:    1198 bytes (0.9% Full)
* (.text + .data + .bootloader)
*
* Data:         26 bytes (0.6% Full)
* (.data + .bss + .noinit)
*
*
* Build succeeded with 0 Warnings...
*
*
* by       : Thomas Fuchs, Wolfsburg, Germany
*            linux@cc-robotics.de
*
* License  : Copyright (c) 2009 Thomas Fuchs
*
* Tested with AT90CAN128
****************************************************************************/
#include "config.h"
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "utils.h"

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "uart/uart.h"
#include "parser.h"
//  CPU speed
//Connect the streaming with usart to use pritf/scanf etc.
FILE uart_str = FDEV_SETUP_STREAM(uart0_putch, uart0_getc, _FDEV_SETUP_RW);
//-----
// CAN
//-----
//  bitrates
#define	_10KBPS		0	// untested
#define	_20KBPS		1	// untested
#define	_50KBPS		2	// untested
#define	_100KBPS    3
#define	_125KBPS    4
#define	_250KBPS    5
#define	_500KBPS    6
#define	_1MBPS		7	// untested

//  global structs and variables
//  Struktur der CAN-Botschaft
typedef struct
	{
		uint32_t id;			//  MSG ID 11 Bit
		uint16_t msk;        	//  Mask for ID 11 Bit
		uint8_t length;   		//  DLC length
		uint8_t data[8];  		//  Data field 8 Byte
	}CAN_messageType;           //  CAN 2.0 A message 11 Bit

//CAN_messageType msg;
extern CAN_messageType recMsg;
volatile static CAN_messageType msg;

//  external functions
//  externe Funktionen
extern bool initCAN(uint8_t bitrate);;
extern void can_tx(CAN_messageType msg);
extern void configRX();

/***************************************************************************/

//delay routine (milliseconds)
//Funktion um x Millisekunden zu warten
void delay_ms(uint16_t period)
{
	for(unsigned int i=0; i<=period; i++)
		_delay_ms(1);
}

// initializes all recommended interfaces
// Initialisierung
static bool initialize()
{

  // Init CAN interface
  // Init-Funktion für CAN aufrufen mit 100KBPS
  // mit Sicherheitsabfrage ob erfolgreich
U16 eepromm_CAN_timming = eeprom_read_word((uint16_t*)BAUDRATE_ADD); //read CAN baudrate from eeprom to change on the fly

  if(initCAN(eepromm_CAN_timming));
  else if (initCAN(_100KBPS));
  else return false;
  // Empfang konfigurieren
  configRX();

  // alle globalen Interrupts nach der Initialisierung einschalten
  sei();	//activate global interrups

  return true;
}

// main, progstart
// Start des Hauptprogramms
int main()
{
	wdt_enable(WDTO_2S);

  //  Initialisieren
	uart0_init();
   DDRE = (1<<DDE4); //LED embedded on the olimex development Board

  while(!(initialize()));
  stdout = stdin = &uart_str;
  receivedFrames *S;  // make a pointer to Struct to access the CAN frame which we need to send to the BUS
  //  Testnachricht
  //---------------
  //  send: Test OK
  msg.length = 4;
  msg.id = 0x5dd;
  msg.data[0] = 0x54;
  msg.data[1] = 0x65;
  msg.data[2] = 0x73;
  msg.data[3] = 0x74;
  msg.data[4] = 0x20;
  msg.data[5] = 0x4F;
  msg.data[6] = 0x4B;

  // Funktion zum Senden mit der erstellten Nachricht aufrufen
 printf("%s","TEST");
  while (1)
  {
	U8 i=0;
    wdt_reset();  //RESET Watchdog
    //if we have data on serial buffer ring , proceed the data and sort in the right position
    	  if(uart0_available()>0) {
       	  	  	  		   	  while(i<uart0_available()) {
       	  	  	  		  	 	  	U8 temp = uart0_getc();
       	  	  	  		   	 	  	SortTheCharachters(temp);
       	  	  	  		   	  	  	i++;
       	  } }
    	  // if we have a new command then we execute this
    	  if(ReturnStateofCOMMANDREADY()==TRUE) {process_command(); }
    	  ///Prepare to send a CAN frame to the BUS
    	   S = returnAddressOfStr();

    	   // if we have a CAN frame to send , we access this with pointer and send it
    	   if(returnFlagtoSend()==TRUE) {
    	         	 // --- Init Tx Commands
    	         	 cli();
    	         	resetFlagtoSend();
    	         	 // for(i=0; i<(S->DLC_stru); i++) {tx_buffer[i]=S->Data[i]};

    	         	 for(U8 indx=0; indx< (S->DLC_stru); indx++)
    	         	 { msg.data[indx] = *(S->Data + indx); }
    	         	 msg.id = S->id;
    	         	 //tx_msg.ctrl.ide = S->ext;
    	         	 //tx_msg.ctrl.rtr = S->rmt;
    	         	 msg.length = S->DLC_stru;
    	         	 //tx_msg.cmd = CMD_TX_DATA;
    	         	 // --- Tx Command
    	         	 can_tx(msg);

    	         	 sei();
    	    	  	  }
  }
}
