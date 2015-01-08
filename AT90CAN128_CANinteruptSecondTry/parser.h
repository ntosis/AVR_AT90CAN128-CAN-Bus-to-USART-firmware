/*
 * parser.h
 *
 *  Created on: Oct 24, 2014
 *      Author: Tosis Nikolaos
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdint.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#define TRUE 1
#define FALSE 0

#define HardwareID 0xFBBF //ID for oprional function
#define BAUDRATE_ADD 0x01 //eepromm address of Baudrate variable
// CAN frame Struct to send at the BUS
typedef struct {

	uint32_t id;
	uint8_t ext;
	uint8_t std;
	uint8_t rmt;
	uint8_t DLC_stru;
	uint8_t Data[8];

} receivedFrames;

//-----------DECLARATION
static receivedFrames R;

static uint8_t *addresOfStruct;
static uint8_t flagtosendCANmsg=0;


extern void SortTheCharachters(uint8_t data);
extern void process_command();
extern void CANframeSortToStruct(uint8_t input[35]);
extern void SetBAUDRATEforCAN(uint8_t input[20]);
extern void AnswerForCorrectCOMPort(uint8_t input[20]);
extern uint8_t ReturnStateofCOMMANDREADY();
extern uint16_t hex2int(char *a, unsigned int len);

extern uint8_t* returnAddressOfStr(void);
extern uint8_t returnFlagtoSend(void);
extern void resetFlagtoSend(void);

#endif /* PARSER_H_ */
