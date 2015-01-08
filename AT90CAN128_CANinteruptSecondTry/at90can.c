/*******************************************************************
* CAN Test-APP
* - CAN functions definded for AT90CAN
*
* Compiler : avr-gcc 4.3.0 / avr-libc 1.6.2 / AVR Studio 4.14
* size     : 6,18KB
* by       : Thomas Fuchs, Wolfsburg, Germany
*            linux@cc-robotics.de
*
* License  : Copyright (c) 2009 Thomas Fuchs
*
* Tested with AT90CAN128
*******************************************************************/
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "at90can.h"

/* to enhance the readability:
 * - MOb = Message Object
 */

// returns the mob which has the message
uint8_t getmob( uint32_t bitmask)
{
  uint8_t mob;

  if( bitmask==0) return NOMOB;

  for( mob=0; (bitmask & 0x01)==0; bitmask >>= 1, ++mob);

  //  security check
  if ( mob > 14 ) return NOMOB;
    else return mob;
}

// initializes CAN interface
bool initCAN( uint8_t bitrate )
{
  CAN_DDR |= _BV(CAN_TX);
  CAN_DDR &= ~_BV(CAN_RX);

  //  security check
  if (bitrate >= 8) return false;

  //  number of the MOb
  uint8_t mob;

  CANGCON |= (1<<SWRES);		// reset the CAN controller

  CANGCON = 0x00;				// reset the general control register (CONFIG mode)

  // reset the interrupt registers
  CANSIT2 = 0x00;
  CANSIT1 = 0x00;
  CANGIT  = 0x00;
  CANGIE  = 0x00;
  CANEN1  = 0x00;
  CANEN2  = 0x00;
  CANIE1  = 0x00;
  CANIE2  = 0x00;

  // set auto inc to Data Buffer Index (DBI)
  CANPAGE &= ~(1<<AINC);

  // set all MObs to 0
  for (mob = 0; mob < NR_MOBS; mob++)
  {
    CANPAGE  = (mob << 4);
	CANIDT1 = 0x00;  //  reset ID-Tag
	CANIDT2 = 0x00;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;

	CANIDM1 = 0x00;  //  reset ID-Mask
	CANIDM2 = 0x00;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;

    CANSTMOB = 0x00;  //  reset MOb status
    CANCDMOB = 0x00;  //  disable MOb
  }

  // set CAN Bit Timing,(see datasheet page 260)
  CANBT1 = pgm_read_byte(&_at90can_cnf[bitrate][0]);
  CANBT2 = pgm_read_byte(&_at90can_cnf[bitrate][1]);
  CANBT3 = pgm_read_byte(&_at90can_cnf[bitrate][2]);

  // set config to MObs 1 and 2
  // MOb 1,3,4,5,6 are receiving the CAN messages
  // MOb 2 send the CAN messages
  for (mob = 1; mob < 7; mob++)
  {
    CANPAGE  = (mob << 4);
	CANSTMOB = 0x00;  //  reset MOb status
	switch (mob)
	{
      case 1:case 3:case 4:case 5: case 6:
	    CANCDMOB = 0x80;  //  RX
	  	CANIDT1  = 0x00;  //  set ID-Tag
	    CANIDT2  = 0x00;

	    CANIDM1  = 0x00;  //  set ID-Mask, receive all
	    CANIDM2  = 0x00;
        break;

	  case 2:
	  	CANIDT1  = 0x00;  //  set ID-Tag
	    CANIDT2  = 0x00;
        break;

	  default:
	    return false;
	}
  }

  //  Enable all required interrupts
  CANGIE  = 0xB0;  // ENIT, ENRX, ENTX
  CANIE2  = 0xFF;  //0x06 MOb 1	~ 8 active

  //  switch CAN on
  CANGCON |= (1<<ENASTB);

  //  wait for EnableFlag
  while (!(CANGSTA & (1<<ENFG)));

  return true;
}

// sends CANmsg
void can_tx (CAN_messageType msg)
{
  uint8_t i, mob;

  //  MOb Sender is Nr 2
  mob=2;

  //  enable MOb number mob, auto increment index, start with index = 0
  CANPAGE = (mob<<4);

  //  set IDE bit, length = 8
  CANCDMOB = (0<<IDE) | (msg.length);		//ide = 1: extendet, ide = 0: normal

  //  set ID
  CANIDT2 = (unsigned char) ((msg.id<<5)&0xE0);
  CANIDT1 = (unsigned char) (msg.id>>3);

  //  write data to MOb
  for (i=0; i<8; i++)
  CANMSG = msg.data[i];

  //  wait for tx busy
  while (CANGSTA & (1<<TXBSY));

  //  send message
  CANCDMOB |= (1<<CONMOB0);
}


// config the receive mob
void configRX()
{
  uint8_t mob;

  // receive in this MOb
  mob = 1;

  CANPAGE = (mob<<4);  // rotate 4 bits left

  // define the IDTags to get only one ID (Filter)
  CANIDT4 = 0;
  CANIDT3 = 0;
  CANIDT2 = 0;
  CANIDT1 = 0;

  // define the IDMasks to activate the Tags (Maske)
  // 1 means filter, 0 means uncared

  CANIDM1  = 0x00;  //filter off, don't care
  CANIDM2  = 0x00;

  //  activate receiving messages with 11 bit id's
  CANCDMOB = (1<<CONMOB1);
}

// interrupt service for CAN
ISR(CANIT_vect)
{

  // CAN_messageType recMsg; this is a global variable
  uint8_t save_canpage = CANPAGE;		//  actual CANPAGE
  uint8_t mob;
  uint8_t i;
  uint8_t readmodwrite;

  // check in which MOb the INT was initiated
  mob = getmob(CANSIT2 | (CANSIT1 << 8));
	if( mob == NOMOB )   return;

  // select concerned MOb page
  CANPAGE = (mob << 4);

  // Send was OK!!
  if (CANSTMOB & (1<<TXOK))
  {
    readmodwrite = CANSTMOB;
    readmodwrite &= ~(1<<TXOK);	//  reset INT reason
	CANSTMOB = readmodwrite;
  }
  // Message is received without problems
  else if (CANSTMOB & (1<<RXOK))
  {

  	// get id of selected MOb
    recMsg.id = 0;
    recMsg.id |= ((uint16_t) CANIDT1<<8);
    recMsg.id |= (CANIDT2&0xE0);
    recMsg.id >>= 5;

    // get length of selected MOb
    recMsg.length = (CANCDMOB&0x0F);
    // clear memory
	for (i=0; i<8; i++)
	{
	  recMsg.data[i] = 0;
	}

    // get data of selected MOb
	for (i=0; i<recMsg.length; i++)
	{
	  recMsg.data[i] = CANMSG;
	}

	//Normal mode!!!!!!!!!!
	spy_printing('R',&recMsg);
	//printf("%08lX",recMsg.id);
    readmodwrite = CANSTMOB;
    readmodwrite &= ~(1<<RXOK);  //  reset interrupt
    CANSTMOB = readmodwrite;
	CANCDMOB = (1<<CONMOB1);  //  stay tuned!
  }

  CANPAGE = save_canpage;  //  restore CANPAGE
}
void spy_printing(uint8_t rxtx, CAN_messageType* msg)
{
	uint8_t indx;
unsigned char startofframe = 35;
  unsigned char endofframe = 42;
    printf("%01c%08lX %02X %02X %02X %02X ",startofframe,msg->id,0,1,0,msg->length);

    {
        for(indx=0; indx<(msg->length); indx++)
        {
        	printf ("%02X ", *(msg->data + indx));

        }
        unsigned short time_stamp = 0xAB3D;
        printf("%02X%01c",time_stamp,endofframe);
    }

}
