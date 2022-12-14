#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "globals.h"

void freeMemory(ProgInfo* pi) {
	Label* pLabel = pi->firstLabel, * pLabel2;
	while (pLabel != NULL) {
		pLabel2 = pLabel; 
		pLabel = pLabel->next; /*we must move to next label before the pointer is freed*/
		free(pLabel2);
	}
	pi->firstLabel = NULL;
}



