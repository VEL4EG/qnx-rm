#include <devctl.h>
#include <sys/neutrino.h>

#define EM_CTL_CODE_ADD_FAX			0x1
#define EM_CTL_CODE_ADD_VOICE1		0x2
#define EM_CTL_CODE_ADD_VOICE2		0x3
#define EM_CTL_CODE_ADD_DIALUP		0x4
#define EM_CTL_CODE_CLEAR			0x5

#define OUT_WORDS_COUNT 10

struct ESDataExchangeStruct
{
	unsigned short inWord;
	unsigned int outWords[WORDS_COUNT]; 
	bool isOverflow; 		
};

const int RM_EM_CTL_CODE_ADD_FAX = __DIOTF(_DCMD_MISC, EM_CTL_CODE_ADD_FAX, ESDataExchangeStruct);
const int RM_EM_CTL_CODE_ADD_VOICE1 = __DIOTF(_DCMD_MISC, EM_CTL_CODE_ADD_VOICE1, ESDataExchangeStruct);
const int RM_EM_CTL_CODE_ADD_VOICE2 = __DIOTF(_DCMD_MISC, EM_CTL_CODE_ADD_VOICE2, ESDataExchangeStruct);
const int RM_EM_CTL_CODE_ADD_DIALUP = __DIOTF(_DCMD_MISC, EM_CTL_CODE_ADD_DIALUP, ESDataExchangeStruct);
const int RM_EM_CTL_CODE_CLEAR = __DIOTF(_DCMD_MISC, EM_CTL_CODE_CLEAR, ESDataExchangeStruct);
