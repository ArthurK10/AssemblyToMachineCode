#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"

/*the number of instructions*/
int numOfInstructions = sizeof(instructions) / sizeof(instructions[0]);

/*convert a four-byte integer to eight hex digits and also 32 bits.
the bits are shown left to right. the hex charcters shows the bytes right to left but the two digits of each byte are left to right*/
void IntToMachineCode(unsigned int num, char* hexCode, char* binaryCode)
{
	int digit, i, shift=1;
	for (i = 0; i < 8; ++i) {
		digit = num % 16;
		hexCode[i + shift] = hexChar[digit];
		shift = - shift; /*i=0,1,2...7 and index= 1,0,3,2,5,4,7,6*/
		memcpy(binaryCode + 28 - 4 * i, binary[digit], 4);
		num /= 16;
	}
}

BOOL processFile(char* filename);

/*processes each of the programs passed in the command line*/
int main(int argc, char* argv[]) {
	int i;
	for (i = 1; i < argc; i++) {
		processFile(argv[i]);
	}
	return 0;
}

/*the central func that reads an assembler program and generates the three output files (object, entries, externals)*/
BOOL processFile(char* progName) {

	int cmdIx, entryIx, dataIx, charIx;
  	int hexNum;
	ProgInfo progInfo;
	progInfo.ic = 0;
	progInfo.dc = 0;
	progInfo.entryCnt = 0;
	char templine[MAX_LINE_LEN + 2];
	FILE* curr_file;
	int compileError = 0;
	Label* pLabel;
	FILE* externFile;


	char fileName[50];
	sprintf(fileName, "%s", progName);
	curr_file = fopen(fileName, "r");
	if (!curr_file) {
		printf("Can't open the file. Moving to the next\n");
		return False;
	}
	/*read a line from the Assembler program and preform the first pass. If we encounter compile errors 
	we save an indication but continue checking the following lines.*/
	while (fgets(templine, MAX_LINE_LEN + 2, curr_file) != NULL) {

		if (!firstPass_line_process(templine, &progInfo)) {
			compileError = 1;
		}
	}
	fclose(curr_file);
	/*If there was at least one compile error we stop the process*/
	if (compileError) {
		printf("\n\nthere were compile errors\n");
		freeMemory(&progInfo);
		return 1;
	}
	
	/*We go over the entries and assert that for each entry there is an actual label. If not, it's a compile error.*/
	FILE* entryFile;
	char entryFileName[50];
	char externFileName[50];
	sprintf(entryFileName, "%s.ent", progName);
	entryFile = fopen(entryFileName, "w+");
	for (entryIx = 0; entryIx < progInfo.entryCnt; entryIx++) {
		for (pLabel = progInfo.firstLabel; pLabel != NULL && strcmp(pLabel->name, 
			progInfo.entries[entryIx]) != 0; pLabel= pLabel->next) {
		}
		if (pLabel == NULL) {
			printf("entry %s does not have a label\n", progInfo.entries[entryIx]);
			compileError = 1;
			entryIx = progInfo.entryCnt;
		}
		else {
			fprintf(entryFile, "%s %4d\n", pLabel->name, pLabel->offset + IC_INIT_VAL 
				+ (pLabel->labelType == 0 ? 0 : progInfo.ic * 4));
		}
	}
	fclose(entryFile);
	if (compileError) {
		printf("there were compile errors\n");
		freeMemory(&progInfo);
		fclose(entryFile);
		remove(entryFileName);
		return 0;
	}
	/*No compile errors were detected. We proceed with the second pass and build the object file and the external file*/
	unsigned int code;
	char hexCode[9]= {'\0'};
	char binaryCode[33] = {'\0'};
	sprintf(fileName, "%s.ob", progName);
	curr_file = fopen(fileName, "w+");
	sprintf(externFileName, "%s.ext", progName);
	externFile = fopen(externFileName, "w+");
	fprintf(curr_file, "%7d %d\n", progInfo.ic * 4, progInfo.dc);
	/*perform second pass for each command.*/
	for (cmdIx = 0; cmdIx < progInfo.ic; cmdIx++) {

		code = secondPass_line_process(&progInfo, cmdIx, externFile);
		if(code == 0){ /*second pass detected error*/
			compileError = 1;/*remember that there has been an error*/
		}
		else {
			IntToMachineCode(code, hexCode, binaryCode); /*convert the machine code to printable hex and binary formats*/
			/*write the command offset and hex code to the object file*/
			fprintf(curr_file, "%4d %s\n", cmdIx * 4 + IC_INIT_VAL, hexCode);
		}
	}
	if(compileError == 0){ /*write the data literals to the object file after all the instructions*/
		for (dataIx = 0; dataIx < progInfo.dc; dataIx += 4) {
			fprintf(curr_file, "%4d ", progInfo.ic *4 + dataIx+ IC_INIT_VAL);
			for (charIx = 0; charIx < 4 && dataIx+charIx < progInfo.dc; charIx++) {
				hexNum = (int)progInfo.data[dataIx + charIx];
				if (hexNum < 0) {
					hexNum = UCHAR_MAX + hexNum + 1;
				}
				fprintf(curr_file, "%c%c", hexChar[hexNum / 16], hexChar[hexNum % 16]);
			}
			fprintf(curr_file,"\n");
		}
	}
	fclose(curr_file);
	fclose(externFile);
	freeMemory(&progInfo);
	if(compileError){/*we don't want output files in case of compile errors*/
		remove(entryFileName);
		remove(externFileName);
		remove(fileName);
	}
	return True;
}


