/*
 * parser.c
 *
 *  Created on: Oct 24, 2014
 *      Author: Nikolaos Tosis
 */
#include "parser.h"
#include <string.h>

static volatile uint8_t pointer=0;

volatile uint8_t command_ready;
volatile uint8_t command_in[36];
//Return a pointer to Struct
uint8_t* returnAddressOfStr() { return addresOfStruct; }
//return the value of $flagtosendCANmsg , it's a flag to Send or not a CAN frame
uint8_t returnFlagtoSend() { return flagtosendCANmsg; }

//Put the incoming bytes to $command_in array and if at the end exists an ASCII '*' sets the $command_ready to TRUE
 void SortTheCharachters(uint8_t data)
 {
	 command_in[pointer]=data;
	 pointer++;
	 if (data == '*') {
	command_ready = TRUE;
	pointer=0;
	 }
 }

 //Recognized the first ASCII character of the $command_in array and execute the appropriate function
 void process_command()
 {
 switch (command_in[0]) {
 case '#': CANframeSortToStruct(command_in);
 break;

 case '$': SetBAUDRATEforCAN(command_in);
 break;

 case '!': AnswerForCorrectCOMPort(command_in);
 break;

 default:
 printf("NOT RECOGNISED");
 break;
 }
 }
//this function store to Struct a CAN frame from the PC software and later sends the frame to the CAN bus.
//look at the file CANtoUART_Specification.ods for the PC software's CAN frame format
void CANframeSortToStruct(uint8_t input[35]) {
	char temp_id[5], temp_id2[5];
	char *pch; // receivedFrames R;
	char copyarray[35];
	addresOfStruct = &R;

	memset(temp_id,'\0',5);
	memset(temp_id2,'\0',5);

	pch = strchr(input, '#');
	strncpy(copyarray, pch+1,33);

	//**************************
	uint16_t bit,bit2;
	strncpy(temp_id,&copyarray,4);
	strncpy(temp_id2,&copyarray[4],4);

	bit= hex2int(temp_id,4);
    bit2 =hex2int(temp_id2,4);

    R.id = (((uint32_t)bit) << 16) | (uint32_t)bit2 ;
    //**************************
    char ext[3];
    memset(ext,'\0',3);
    strncpy(ext,&copyarray[8],2);
    R.ext = hex2int(ext,2);
    //**************************
    char std[3];
    memset(std,'\0',3);
    strncpy(std,&copyarray[10],2);
    R.std = hex2int(std,2);
    //**************************
    char rmt[3];
    memset(rmt,'\0',3);
    strncpy(rmt,&copyarray[12],2);
    R.rmt = hex2int(rmt,2);
    //**************************
    char DLC_s[3];
    memset(DLC_s,'\0',3);
    strncpy(DLC_s,&copyarray[14],2);
    R.DLC_stru = hex2int(DLC_s,2);
    //**************************
    char data[3];
    memset(data,'\0',3);
    uint8_t j=0,i=0;
    for(i =0; i<(R.DLC_stru*2); i=i+2) { strncpy(data,&copyarray[16+i],2); R.Data[j] = hex2int(data,2); j++;  }
    //***************************

    command_ready = FALSE;
    flagtosendCANmsg = TRUE;


 }
void SetBAUDRATEforCAN(uint8_t input[20]) {
	char string[8]="BAUDRATE";
	char *pch;
	uint8_t temp[4];
	uint16_t Received_BaudRate;
	//BAUDRATE=500 ,this command comes from PC software
	pch = strchr(input, '='); //find the '='

	strncpy(temp, pch+1,4);  //copy to variable temp the 4 ASCII characters after '='
	Received_BaudRate= atoi(temp); // ASCII to int
	switch(Received_BaudRate) {
	case 100: Received_BaudRate=3;  break;
	case 250: Received_BaudRate=5;	break;
	case 500: Received_BaudRate=6;	break;
	case 1000: Received_BaudRate=7;	break;
	default : Received_BaudRate=3; break;
	}
	eeprom_write_word((uint16_t*)BAUDRATE_ADD,Received_BaudRate); //Store the integer to eepromm

	command_ready = FALSE; //reset the flag
	cli();
	while(1);  //Make Hard reset with watchdog
}
//Optional function to return an ID, The PC software can find automatic the right serial port.
void AnswerForCorrectCOMPort(uint8_t input[20]) {

	char str1[6];
	char str2[6];
	int ret;

	   memcpy(str1,&input[1], 6);
	   memcpy(str2, "LISTEN", 6);

	   ret = memcmp(str1, str2, 6); // 6 h 5 ???

	   if (ret==0) { printf("%02X",HardwareID); }

	   command_ready = FALSE;

}

//Return  an ASCII character from HEX number to decimal
uint16_t hex2int(char *a, unsigned int len)
{
    int i;
    uint16_t val = 0;

    for(i=0;i<len;i++)
       if(a[i] <= 57)
    	val += (a[i]-48)*(1<<(4*(len-1-i)));
       else
    	val += (a[i]-55)*(1<<(4*(len-1-i)));
    return val;
}
// function to read the state of private variable $command_ready.
uint8_t ReturnStateofCOMMANDREADY() {
	return command_ready;
}
// function to have access to private variable $flagtosendCANmsg, we set the variable to state FALSE
void resetFlagtoSend() {
flagtosendCANmsg = FALSE;
}
