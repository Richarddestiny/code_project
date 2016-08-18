#include "buffer.h"
#include <semaphore.h>

static int err_init = 0;
static pthread_once_t initonce = PTHREAD_ONCE_INIT;

static sem_t semitems;
static sem_t semslots;

/*
** call this function exactly ONCE
** before getitem() and putitem()
*/
int buffer_init(void)
{
	if(sem_init(&semitems, 0, 0) != 0)
		return errno;

	if(sem_init(&semslots, 0, SIZE) != 0){
		err_init = errno;
		/*
		** free the other semaphore
		*/
		sem_destroy(&semitems);
		return err_init;
	}

	return 0;
}

void initialization(void)
{
	err_init = buffer_init();
}

int buffer_initmutex(void)
{
	int err;

	/*
	** initialize buffer at most ONCE
	*/
	if((err=pthread_once(&initonce, initialization)) != 0)
		return err;

	return err_init;
}
