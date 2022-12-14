#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "globals.h"
#include "second_pass.h"


extern Instruction instructions[];
extern int numOfInstructions;

/*convert the command structure (intruction and paremeters) to machine code.*/
unsigned int secondPass_line_process(ProgInfo* pi, int cmdIx, FILE *externFile) {

	Command* pCmd = &pi->commands[cmdIx];
	Instruction* pInst = &instructions[pCmd->instructionIndex];
	unsigned int code;
	int offset;
	Label* pLabel;
	/*the bit layout is based on the istruction type (R,I,J)*/
	switch (pInst->instType) {
		case instructionR:
			/*in R instructions with two parameters the second parameter should be rd and not rt*/	
			if (pInst->numOfOperands == 2) {
				pCmd->operands[2] = pCmd->operands[1];
				pCmd->operands[1] = 0;
			}
			/*shift every field to the correct bit position*/
			code = (pInst->opcode << 26) + (pCmd->operands[0] << 21) + (pCmd->operands[1] << 16) +
				(pCmd->operands[2] << 11) + (pInst->funct << 6);
			break;

		case instructionI:
			code = (pInst->opcode << 26) + (pCmd->operands[0] << 21);/*the opcode and the first parameter*/
			if (pInst->operandType[1] == Register) {
				/*when the second parameter is a register we put it in the code and then look for the label (3rd parm)*/
				code += (pCmd->operands[1] << 16);
				for(pLabel=pi->firstLabel; pLabel!= NULL && strcmp(pLabel->name, pCmd->label) != 0; pLabel= pLabel->next){
				}
				if (pLabel == NULL) {
					printf("label %s not found\n", pCmd->label);
					return 0;
				}
				if (pLabel->labelType == 1) {/*we can only branch to an instruction label */
					printf("cannot branch to a data label %s\n", pCmd->label);
					return 0;
				}
				if (pLabel->isExternal) {
					printf("cannot branch to an external label %s\n", pCmd->label);
					return 0;
				}
				offset = pLabel->offset - (4 * cmdIx); /*the distance between the label and the current command*/
				if (offset < 0) { /*the two-compliment of a negative number should only use 16 bits*/
					offset = USHRT_MAX + offset + 1;
				}
				code += offset;/*include the offset in the machine code*/
			}
			else {/*the second paremeter is a literal. include it and the register in the 3rd parm*/
				code = code + (pCmd->operands[2] << 16) + pCmd->operands[1];
			}
			break;

		case instructionJ:
			code = (pInst->opcode << 26);/*the opcode*/
			if (pInst->numOfOperands > 0) {
				if (pCmd->label[0] == '\0') {/*if there is no label we mark 1 to indicate a register*/
					code += (1 << 25) + pCmd->operands[0];
				}
				else {
					/*search for the label*/	
					for (pLabel = pi->firstLabel; pLabel != NULL && strcmp(pLabel->name, pCmd->label) != 0;
						 pLabel = pLabel->next) {
					}
					if (pLabel == NULL) {
						printf("label %s not found\n", pCmd->label);
						return 0;
					}
					if (pLabel->isExternal == 0) {
						code += pLabel->offset + IC_INIT_VAL; /*absolute address of the label*/
						if (pInst->opcode == 31) { /*is it a "la" instruction?*/
							if (pLabel->labelType == 0) {
								printf("la command requires data label %s\n", pCmd->label);
								return 0;
							}
							code += pi->ic*4;/*the data labels are located after the end of the instructions*/
						}
						else {
							if (pLabel->labelType == 1) {
								printf("jmp/call requrire instruction label %s\n", pCmd->label);
								return 0;
							}
						}
					}
					else {
						/*write the location of the refernce to an external label*/
						fprintf(externFile, "%s %4d\n", pLabel->name, cmdIx * 4 + IC_INIT_VAL);
					}
				}
			}
	}
	return code;
}




