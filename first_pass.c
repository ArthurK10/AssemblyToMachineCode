#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "globals.h"
#include "first_pass.h"

extern Instruction instructions[];
extern int numOfInstructions;
/*parse a line from the assembler program*/
unsigned int firstPass_line_process(char line[], ProgInfo *pi) {

	char* p, * begin = line;
	int index, i, j, instIx, opIx, operandNeedsHandling;
	int len = strlen(line);
	Label* pLabel= NULL;
	int minValue, maxValue, num;
	unsigned int uNum;
	printf("%s", line);
	/*if the line contains only \r \n or begins with semicolon we return without error. the line is skipped*/
	if (len <=2 || line[0] == ';') {
		return 1;
	}
	/*the \r\n are trimmed*/
	if (line[len - 1] == '\n') {
		len--;
		line[len] = '\0';
		if(line[len - 1] == '\r'){
			len--;
			line[len] = '\0';
		}
	}
	char* endLine = line + len;
	/*if the line begins with a dot, it must be .entry or .extern. anything else in an error*/
	if (line[0] == '.') {
		if (memcmp(line + 1, "entry", 5) == 0 && (line[6] == ' ' || line[6] == '\t')) {
			/*find the beginning and the end of the entry name, and add it to the array of entries*/
			index = strspn(line + 7, " \t");
			begin = line + 7 + index;
			if (*begin == '\0') {
				printf("entry without label\n");
				return 0;
			}
			i = strcspn(begin, " \t");
			begin[i] = '\0';
			strcpy(pi->entries[pi->entryCnt], begin);
			pi->entryCnt++;
			return 1;
		}
		if (memcmp(line + 1, "extern", 6) == 0 && (line[7] == ' ' || line[7] == '\t')) {
			/*find the beginning and the end of the external name, and add it to the linked list of labels,
			marked as external*/
			index = strspn(line + 8, " \t");
			begin = line + 8 + index;
			if (*begin == '\0') {
				printf("extern without label\n");
				return 0;
			}
			i = strcspn(begin, " \t");
			begin[i] = '\0';
			pLabel = (Label*)malloc(sizeof(Label));
			pLabel->next = pi->firstLabel;
			pi->firstLabel = pLabel;
			pLabel->isExternal = 1;
			pLabel->labelType = 0;

			strcpy(pLabel->name, begin);
			pLabel->offset = 0;
			return 1;
		}
		printf("invalid dot(.) command\n");
		return 0;
	}

	begin= line;

	if (line[0] != ' ') {
		/*the line begins with non-blank non-dot. it must be a label*/
		index = strcspn(line, ": \t");/*finding the end of label*/
		if (index == 0 || line[index] != ':') {/*if it's not a colon, it's an error*/
			printf("invalid label\n");
			return 0;
		}
		/*adding the label to the linked list*/
		line[index] = '\0';
		pLabel = (Label*)malloc(sizeof(Label));
		pLabel->next = pi->firstLabel;
		pi->firstLabel = pLabel;
		pLabel->isExternal = 0;

		strcpy(pLabel->name, line);
		pLabel->offset = pi->ic*4; /*the offset of the label if it is an intruction label*/
		pLabel->labelType = 0;/*marked as instruction label*/
		begin = line + index + 1;/*positioning after the end of the label*/

	}
	/*whether there was a label or there wasn't, we go on with the rest of the line*/	
	i = strspn(begin, " \t");/*find the first non-blank*/
	if(begin[i]=='\0'){/*the first non-blank was the end of the line.*/
		return 1;/*quit without error*/
	}
	begin = begin + i;/*position on the first non-blank*/
	if (begin[0] == '.') {/*it must be a data literal definition*/
		if (pLabel != NULL) {/*if there was a label in that line*/
			pLabel->offset = pi->dc;/*we set its offset to the data offset*/
			pLabel->labelType = 1;/*marked as data label*/
		}
		if (memcmp(begin + 1, "asciz ", 6) == 0) {
			/*a string constant surrounded by double quotes*/
			i = strspn(begin + 6, " \t");/*skip spaces*/
			begin = begin + 6 + i;
			if (begin[0] != '"') {
				printf("double quote expected\n");
				return 0;
			}
			begin++;
			p = strchr(begin, '"');
			if (p == NULL) {
				printf("missing closing quote\n");
				return 0;
			}
			*p = '\0';
			strcpy(pi->data + pi->dc, begin);
			pi->dc = pi->dc + (p - begin) + 1;/*increment the data count by the size of the string plus one*/
			return 1;
		}
		/*if it's not .asciz, it must be .db, .dw, .dh*/
		if (begin[1] != 'd' || begin[3] != ' ') {
			printf("invalid command\n");
			return 0;
		}
		switch (begin[2])
		{
		case 'b':
			j = 1;/*length is 1 byte*/
			minValue = CHAR_MIN;
			maxValue = CHAR_MAX;
			break;
		case 'h':
			j = 2;
			minValue = SHRT_MIN;
			maxValue = SHRT_MAX;
			break;
		case 'w':
			j = 4;
			minValue = INT_MIN;
			maxValue = INT_MAX;
			break;
		default:
			printf("invalid command\n");
			return 0;
		}
		begin += 3;/*end of the '.d?'*/
		i = strspn(begin, " \t");
		begin += i;
		if (*begin == '\0') {
			printf("missing parameter\n");
			return 0;
		}
		/*there might be a unlimited number of literals*/
		while (begin < endLine)
		{
			i = strspn(begin, "-0123456789");/*finds the first non-digit*/
			if (!(begin[i] == ' ' || begin[i] == '\0' || begin[i] == ',' || begin[i] == '\n' || begin[i] == '\t')) {
				printf("non numeric parameter: %s\n" ,begin);
				return 0;
			}
			num = atoi(begin); /*convert to an integer until the first non-digit*/
			if (num < minValue || num > maxValue) {
				printf("value is not in range\n");
				return 0;
			}
			if (num >= 0) {
				uNum = num;
			}
			else {
				/*for negative numbers, we want two-complement only in the right sixteen bits*/
				uNum = UINT_MAX + num + 1;
			}
			/*based on th lengh we move the value byte by byte*/
			switch (j) {
				case 1:
					pi->data[pi->dc] = (char)uNum;
					break;
				case 2:
					pi->data[pi->dc] = (char)(uNum % 256);
					pi->data[pi->dc + 1] = (char)(uNum / 256);
					break;
				case 4:
					pi->data[pi->dc] = (char)(uNum % 256);
					uNum /= 256;
					pi->data[pi->dc + 1] = (char)(uNum % 256);
					uNum /= 256;
					pi->data[pi->dc + 2] = (char)(uNum % 256);
					uNum /= 256;
					pi->data[pi->dc + 3] = (char)(uNum % 256);
					break;
			}
			/*skiping spaces. if we find a comma we skip it too and the folowing spaces.
			we end up in the begining of the next literal or the end of the line*/
			pi->dc += j;/*increment the data count*/
			begin += i;
			begin+= strspn(begin, " \t");
			if (*begin == ',') {
				begin = begin + 1 + strspn(begin + 1, " \t");
			}
			else {
				/*if there is no comma, there must not be anything else on that line*/
				if (*begin != '\0') {
					printf("invalid syntax\n");
					return 0;
				}
			}
			/*the loop continue with the next literal*/
		}
		return 1;
	}
	/*the token does not begin with a dot. its must be an assembler instruction*/
	i = strcspn(begin, " \t");/*find the end of the command*/
	p = begin + i;
	*p = '\0';/*change it to a null charcter*/
	/*search for the instruction in the array*/
	for (instIx = 0; instIx < numOfInstructions && strcmp(instructions[instIx].name, begin)!=0 ; instIx++) {
	}
	if (instIx >= numOfInstructions) {/*not found*/
		printf("invalid instruction: %s\n", begin);
		return 0;
	}
	begin = p + 1;
	/*building a command entry in the array of commands*/
	pi->commands[pi->ic].instructionIndex= instIx;
	pi->commands[pi->ic].label[0] = '\0';
	pi->commands[pi->ic].operands[0] = pi->commands[pi->ic].operands[1]= pi->commands[pi->ic].operands[2]= 0;
	/*searching for the operands based on the number and types of the operands for that intruction*/
	for (opIx = 0; opIx < instructions[instIx].numOfOperands; opIx++) {
		operandNeedsHandling = 1;
		begin += strspn(begin, " \t");
		if (begin >= endLine) {
			printf("missing operand\n");
			return 0;
		}
		if (*begin == ',') {
			if (opIx == 0) {
				printf("superfluous comma\n");
				return 0;
			}
			begin++;
			begin += strspn(begin, " \t");
		}
		else {
			if (opIx > 0) {
				printf("missing comma\n");
				return 0;
			}
		}
		if (instructions[instIx].operandType[opIx]== Register || instructions[instIx].operandType[opIx] == RegisterOrLabel)
		{
			/*register operand must begin with a dollar sign. jump instrucion can have a dollar or a label*/
			if (*begin != '$')
			{
				if (instructions[instIx].operandType[opIx] == Register) {
					printf("register operand must begin with $\n");
					return 0;
				}
			}
			else {
				begin++;
				num = atoi(begin);/*the register number*/
				if (num < 0 || num >31) {
					printf("invalid register number\n");
					return 0;
				}
				pi->commands[pi->ic].operands[opIx] = num;/*save the operand value*/
				begin += strcspn(begin, " ,\t");
				operandNeedsHandling = 0;
			}
		}
		if ((instructions[instIx].operandType[opIx] == LabelOperand || 
			 instructions[instIx].operandType[opIx] == RegisterOrLabel) && operandNeedsHandling) {
			if (*begin == '$') {
				if (instructions[instIx].operandType[opIx] == LabelOperand) {
					printf("invalid label\n");
					return 0;
				}
			}
			else {
				/*saves the label name*/
				i = strcspn(begin, " ,\t");
				memcpy(pi->commands[pi->ic].label, begin, i);
				pi->commands[pi->ic].label[i] = '\0';
				pi->commands[pi->ic].operands[opIx] = -1;
				begin += i;
			}
		}
		if (instructions[instIx].operandType[opIx] == Literal) {
			num = atoi(begin);
			if (num<SHRT_MIN || num> SHRT_MAX) {
				printf("number is not in range\n");
				return 0;
			}
			if (num < 0) {
				num = USHRT_MAX + num + 1;
			}
			pi->commands[pi->ic].operands[opIx] = num;
			begin += strcspn(begin, " ,\t");
		}
	}
	/*after the last expected parameter the rest of the line must be empty*/
	if (begin < endLine) {
		begin = begin + strspn(begin, " \t");
		if (begin < endLine) {
			printf("too many parameters\n");
			return 0;
		}
	}
	pi->ic++;
	return 1;
 }
