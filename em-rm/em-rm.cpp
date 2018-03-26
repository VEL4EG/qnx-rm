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

#include <iostream>

using namespace std;

#include "..\dev.h"

#define THREAD_POOL_PARAM_T dispatch_context_t

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

int errCount = 0;

int	io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

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
    if((tpp = thread_pool_create(&pool_attr,
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
	// 1) see if it's a standard devctl()
	int res;
    if ((res = iofunc_devctl_default (ctp, msg, ocb)) != _RESMGR_DEFAULT)
    	return res;
    ESDataExchangeStruct *data;
    data= reinterpret_cast<ESDataExchangeStruct *>(_DEVCTL_DATA(msg->i));
    
    int response;
    int nbytes = 0;
    switch (msg->i.dcmd)
    {
        case RM_EM_CTL_CODE_ADD_FAX:
            {
                cout << "RM here\n";
                data->outData = data->inWord;
                
                nbytes = sizeof(ESDataExchangeStruct);
                
                break;
            }

        default:
            cout << "Not there\n";
            return -1;
    }

    memset(&(msg->o), 0, sizeof(msg->o));
    msg->o.nbytes = nbytes;
    SETIOV(ctp->iov, &(msg->o), sizeof(msg->o) + nbytes);
    return (_RESMGR_NPARTS(1));
}

