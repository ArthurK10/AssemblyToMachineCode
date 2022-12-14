#ifndef _FIRST_PASS_H
#define _FIRST_PASS_H
#include "globals.h"


extern Instruction instructions[];
extern int numOfInstructions;

unsigned int firstPass_line_process(char line[], ProgInfo* pi);


#endif