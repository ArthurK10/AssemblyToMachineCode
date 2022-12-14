#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H
#include "globals.h"
#include "first_pass.h"
#include "second_pass.h"
#include "useful_funcs.h"
#include <limits.h>

void IntToMachineCode(unsigned int num, char* hexCode, char* binaryCode);


Instruction instructions[] = 
{
	{"add", 0, 1, instructionR, 3, {Register,Register,Register}},
	{"sub", 0, 2, instructionR, 3, {Register,Register,Register}},
	{"and", 0, 3, instructionR, 3, {Register,Register,Register}},
	{"or", 0, 4, instructionR, 3, {Register,Register,Register}},
	{"nor", 0, 5, instructionR, 3, {Register,Register,Register}},
	{"move", 1, 1, instructionR, 2, {Register,Register,None}},
	{"mvhi", 1, 2, instructionR, 2, {Register,Register,None}},
	{"mvlo", 1, 3, instructionR, 2, {Register,Register,None}},
	{"addi", 10, 0, instructionI, 3, {Register,Literal,Register}},
	{"subi", 11, 0 , instructionI, 3, {Register,Literal,Register}},
	{"andi", 12, 0 , instructionI, 3, {Register,Literal,Register}},
	{"ori", 13, 0 , instructionI, 3, {Register,Literal,Register}},
	{"nori", 14, 0 , instructionI, 3, {Register,Literal,Register}},
	{"bne", 15, 0 , instructionI, 3, {Register,Register,LabelOperand}},
	{"beq", 16, 0 , instructionI, 3, {Register,Register,LabelOperand}},
	{"blt", 17, 0 , instructionI, 3, {Register,Register,LabelOperand}},
	{"bgt", 18, 0 , instructionI, 3, {Register,Register,LabelOperand}},
	{"lb", 19, 0 , instructionI, 3, {Register,Literal,Register}},
	{"sb", 20, 0 , instructionI, 3, {Register,Literal,Register}},
	{"lw", 21, 0 , instructionI, 3, {Register,Literal,Register}},
	{"sw", 22, 0 , instructionI, 3, {Register,Literal,Register}},
	{"lh", 23, 0 , instructionI, 3, {Register,Literal,Register}},
	{"sh", 24, 0 , instructionI, 3, {Register,Literal,Register}},
	{"jmp", 30, 0 , instructionJ, 1, {RegisterOrLabel,None,None}},
	{"la", 31, 0, instructionJ, 1, {LabelOperand,None,None}},
	{"call", 32, 0 , instructionJ, 1, {LabelOperand,None,None}},
	{"stop", 63, 0 , instructionJ, 0, {None,None,None}}
};


const char* binary[] = { "0000","0001","0010","0011","0100","0101","0110","0111","1000",
"1001","1010","1011","1100","1101","1110","1111" };

const char hexChar[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };


#endif
