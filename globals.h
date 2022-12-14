#ifndef _globals_h
#define _globals_h
#define IC_INIT_VAL 100
#define DC_INIT_VAL 0
#define MAX_LINE_LEN 80


typedef enum {
	False, True
}BOOL;

/*the three instruction types*/
typedef enum {
	instructionR, instructionI, instructionJ
}InstructionType;

/*each parameter is expected to be a register, a literal or a labelname. 
the jmp instruction expects a regiter or a label*/
typedef enum {
	None, Register, Literal, LabelOperand, RegisterOrLabel
}OperandType;


typedef struct Label {
	char name[31];
	int offset;
	int isExternal;
	int labelType; /*instruction = 0 , data = 1*/
	struct Label* next;
}Label;

/*defines an entry in the list of instructions that our Assemler suppurts*/
typedef struct {
	const char *name;
	int opcode;
	int funct;
	InstructionType instType;
	int numOfOperands;
	OperandType operandType[3];
}Instruction;

/**/
typedef struct
{
	int instructionIndex;
	int operands[3];
	char label[31];
} Command;


typedef struct {
	
	char data[300];
	int ic, dc, entryCnt;
	char entries[50][20];
	struct Label* firstLabel;
	Command commands[300];
} ProgInfo;


#endif
