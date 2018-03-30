#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>

#include "..\dev.h"

using namespace std;


#define THREAD_POOL_PARAM_T dispatch_context_t

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

int errCount = 0;
int currentIndex = 0;
const unt syncWord = 0b11110101 << 8;
uint words[WORDS_COUNT]; 

int	io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
void addWord(ESDataExchangeStruct *data, uint (*makeWordFunc)(ushort inWord));
inline uint getLastWord();
uint makeFaxWord(ushort inWord);
uint makeVoice1Word(ushort inWord);
uint makeVoice2Word(ushort inWord);
uint makeDialupword(ushort inWord);
void clearWords();

int main(int argc, char **argv)
{
    /* структуры администратора ресурсов */
    thread_pool_attr_t   pool_attr;
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    thread_pool_t        *tpp;
    dispatch_context_t   *ctp;
    int                  id;

    /* инициализация диспетчеризации */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr,
                "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* инициализация атрибутов */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    /* инициализация функций обработки сообщений */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);

    /* инициализация атрибутов устройства */
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    io_funcs.devctl = io_devctl;

    /* регистрация АР */
    id = resmgr_attach(
						dpp,            /* dispatch handle        */
						&resmgr_attr,   /* resource manager attrs */
						"/dev/em-device",  /* device name            */
						_FTYPE_ANY,     /* open type              */
						0,              /* flags                  */
						&connect_funcs, /* connect routines       */
						&io_funcs,      /* I/O routines           */
						&attr);         /* handle                 */
    if(id == -1)
	{
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* инициализация атрибутов пула потоков */
    memset(&pool_attr, 0, sizeof pool_attr);
    pool_attr.handle = dpp;
    pool_attr.context_alloc = dispatch_context_alloc;
    pool_attr.block_func = dispatch_block;
    pool_attr.unblock_func = dispatch_unblock;
    pool_attr.handler_func = dispatch_handler;
    pool_attr.context_free = dispatch_context_free;
    pool_attr.lo_water = 2;
    pool_attr.hi_water = 4;
    pool_attr.increment = 1;
    pool_attr.maximum = 50;

    /* пул потоков - создание */
    if ((tpp = thread_pool_create(&pool_attr,
                                 POOL_FLAG_EXIT_SELF)) == NULL) {
        fprintf(stderr, "%s: Unable to initialize thread pool.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* запуск пула потоков */
    thread_pool_start(tpp);

    return 0;
}

int	io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
	int res;
    if ((res = iofunc_devctl_default (ctp, msg, ocb)) != _RESMGR_DEFAULT)
    	return res;
    
    ESDataExchangeStruct *data;
    data = reinterpret_cast<ESDataExchangeStruct *>(_DEVCTL_DATA(msg->i));
    
    int response;
    int nbytes = 0;

    // mutex lock
    switch (msg->i.dcmd)
    {
        case RM_EM_CTL_CODE_ADD_FAX:
            makeFaxWord(data);
            break;

        case RM_EM_CTL_CODE_ADD_VOICE1:
			makeVoice1Word(data);
			break;

        case RM_EM_CTL_CODE_ADD_VOICE2:
			makeVoice2Word(data);
            break;

        case RM_EM_CTL_CODE_ADD_DIALUP:
			makeDialupword(data);
			break;

        case RM_EM_CTL_CODE_CLEAR:
			clearWord();
            break;

        default:
            cout << "Not there\n";
            return -1;
    }

    // mutex unlock
    //
    memset(&(msg->o), 0, sizeof(msg->o));
    msg->o.nbytes = nbytes;
    SETIOV(ctp->iov, &(msg->o), sizeof(msg->o) + nbytes);
    return (_RESMGR_NPARTS(1));
}

uint getLastWord()
{
	return (currentIndex == 0) ? 0 : words[currentIndex];
}

void addWord(ESDataExchangeStruct *data, uint (*makeWordFunc)(ushort inWord))
{
    uint newWord = makeWordFunc(data->inWord);
    
    if (currentIndex > WORDS_COUNT)
    {
        clearWords();
        words[0] = newWord;
        currentIndex = 1;
    
        data->errorCode = -1;
	    
        return;
    }
        
    words[currentIndex] = newWord;
    currentIndex++;
    
    data->errorCode = 0;

    return;
}

uint makeFaxWord(ushort inWord)
{
    int controlChannel = 0b1100;
    int word = getLastWord();

    word &= 0xC1FFFF00;

    inWord &= 0b11111;
    inWord <<= 25;

    word |= controlChannel;
    word |= inWord;
    word |= synchWord;
    
    return word;
}

uint makeVoice1Word(ushort inWord)
{
	int controlChannel = 0b11000000;
    int word = getLastWord();

	word &= 0x7FFFFF00;

	inWord &= 0b1;
	inWord <<= 31;

	word |= controlChannel;
	word |= inWord;
	word |= sWord;
    
    return word;
}

uint makeVoice2Word(ushort inWord)
{
	int controlChannel = 0b00110000;
    int word = getLastWord();

	word &= 0xBFFFFFFF;

	inWord &= 0b1;
	inWord <<= 30;

	word |= controlChannel;
	word |= inWord;
	word |= sWord;

    return word;
}

uint makeDialupWord(ushort inWord)
{
	int controlChannel = 0b11;
    int word = getLastWord();

	word &= 0xFE0FFFFF;

	inWord &= 0b11111;
	inWord <<= 20;

	word |= controlChannel;
	word |= inWord;
	word |= sWord;

    return word;
}

void clearWord()
{
	for (int i = 0; i < 10; i++)
	{
		data->outData[i] = 0;
	}
}

