#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include "6502emu.h"

extern char opcodes[57][4];
extern char** labels;
extern dbyte* labelVals;
extern int labelLen;
extern dbyte pc;
extern byte instruct[4];

int strequals(char* a, char* b);
int sequenceEqu(char* a, char* b, int len);
int findNextNewline(char* a);
int findOpcode(char* a);
dbyte parseNum(char type, char* num);
int findLabels(char* buff);
int identifyLabel(char* l);
int parseInstruction(int op, int ad, int arg, byte pass);
int parseLine(char* line, byte pass, byte* m);
int setUpLabels(char* buff);
int assemble(char* code, byte* res);


#endif