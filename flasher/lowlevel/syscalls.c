/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "uart_console.h"



#ifndef NULL
#define NULL 0
#endif

/* Forward prototypes.  */
int     _system     _PARAMS ((const char *));
int     _rename     _PARAMS ((const char *, const char *));
int     _isatty		_PARAMS ((int));
clock_t _times		_PARAMS ((struct tms *));
int     _gettimeofday	_PARAMS ((struct timeval *, void *));
void    _raise 		_PARAMS ((void));
int     _unlink		_PARAMS ((const char *));
int     _link 		_PARAMS ((void));
int     _stat 		_PARAMS ((const char *, struct stat *));
int     _fstat 		_PARAMS ((int, struct stat *));
caddr_t _sbrk		_PARAMS ((int));
int     _getpid		_PARAMS ((int));
int     _kill		_PARAMS ((int, int));
void    _exit		_PARAMS ((int));
int     _close		_PARAMS ((int));
int     _swiclose	_PARAMS ((int));
int     _open		_PARAMS ((const char *, int, ...));
int     _swiopen	_PARAMS ((const char *, int));
int 	_write		_PARAMS ((int, const char *, unsigned int));

int     _swiwrite	_PARAMS ((int, char *, int));
int     _lseek		_PARAMS ((int, int, int));
int     _swilseek	_PARAMS ((int, int, int));
int 	_read		_PARAMS ((int, char *, unsigned int));

int     _swiread	_PARAMS ((int, char *, int));
void    initialise_monitor_handles _PARAMS ((void));

static int	wrap		_PARAMS ((int));
static int	error		_PARAMS ((int));
static int	get_errno	_PARAMS ((void));
static int	remap_handle	_PARAMS ((int));

#ifdef ARM_RDI_MONITOR
static int	do_AngelSWI	_PARAMS ((int, void *));
#endif

static int 	findslot	_PARAMS ((int));

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr __asm ("sp");


/* following is copied from libc/stdio/local.h to check std streams */
extern void   _EXFUN(__sinit,(struct _reent *));
#define CHECK_INIT(ptr) \
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
    }						\
  while (0)

/* Adjust our internal handles to stay away from std* handles.  */
#define FILE_HANDLE_OFFSET (0x20)

static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

/* Struct used to keep track of the file position, just so we
   can implement fseek(fh,x,SEEK_CUR).  */
typedef struct
{
  int handle;
  int pos;
}
poslog;

#define MAX_OPEN_FILES 20
static poslog openfiles [MAX_OPEN_FILES];

static int
findslot (int fh)
{
  int i;
  for (i = 0; i < MAX_OPEN_FILES; i ++)
    if (openfiles[i].handle == fh)
      break;
  return i;
}

#ifdef ARM_RDI_MONITOR

static inline int
do_AngelSWI (int reason, void * arg)
{
  int value;
  asm volatile ("mov r0, %1; mov r1, %2; " AngelSWIInsn " %a3; mov %0, r0"
       : "=r" (value) /* Outputs */
       : "r" (reason), "r" (arg), "i" (AngelSWI) /* Inputs */
       : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
		/* Clobbers r0 and r1, and lr if in supervisor mode */);
                /* Accordingly to page 13-77 of ARM DUI 0040D other registers
                   can also be clobbered.  Some memory positions may also be
                   changed by a system call, so they should not be kept in
                   registers. Note: we are assuming the manual is right and
                   Angel is respecting the APCS.  */
  return value;
}
#endif /* ARM_RDI_MONITOR */

/* Function to convert std(in|out|err) handles to internal versions.  */
static int
remap_handle (int fh)
{
  CHECK_INIT(_REENT);

  if (fh == STDIN_FILENO)
    return monitor_stdin;
  if (fh == STDOUT_FILENO)
    return monitor_stdout;
  if (fh == STDERR_FILENO)
    return monitor_stderr;

  return fh - FILE_HANDLE_OFFSET;
}

void
initialise_monitor_handles (void)
{

}

static int
get_errno (void)
{

  return 0;

}

static int
error (int result)
{
  errno = get_errno ();
  return result;
}

static int
wrap (int result)
{
  if (result == -1)
    return error (-1);
  return result;
}

/* Returns # chars not! written.  */
int
_swiread (int file,
	  char * ptr,
	  int len)
{
  int fh = remap_handle (file);

  return 0;

}

/******************************************************************************
* Function Name: _read
* Description  : GNU interface to low-level I/O read
* Arguments    : int file_no
*              : const char *buffer
*              : unsigned int n
* Return Value : none
******************************************************************************/
int _read(int file_no , char *buffer , unsigned int n)
{
	//todo: console via uart
    return 0; //SioRead(file_no , buffer , n);
}

int
_swilseek (int file,
	   int ptr,
	   int dir)
{
return 0;
}

int
_lseek (int file,
	int ptr,
	int dir)
{
  return wrap (_swilseek (file, ptr, dir));
}

/* Returns #chars not! written.  */
int
_swiwrite (
	   int    file,
	   char * ptr,
	   int    len)
{

  return 0;
}

/******************************************************************************
* Function Name: _write
* Description  : GNU interface to low-level I/O write
* Arguments    : int file_no
*              : const char *buffer
*              : unsigned int n
* Return Value : none
******************************************************************************/
int _write(int file_no , const char *buffer , unsigned int n)
{
	//todo: add uart write
	uartc_write(buffer,n);
    return n;//SioWrite(file_no , buffer , n);
}

int
_swiopen (const char * path,
	  int          flags)
{
  return 0;
}

int
_open (const char * path,
       int          flags,
       ...)
{
  return wrap (_swiopen (path, flags));
}

int
_swiclose (int file)
{

  return 0;

}

int
_close (int file)
{
  return wrap (_swiclose (file));
}

int
_kill (int pid, int sig)
{

  return 0;
}

void
_exit (int status)
{
  /* There is only one SWI for both _exit and _kill. For _exit, call
     the SWI with the second argument set to -1, an invalid value for
     signum, so that the SWI handler can distinguish the two calls.
     Note: The RDI implementation of _kill throws away both its
     arguments.  */
  _kill(status, -1);
  while(1)
  {
      /* exit occurred */
  };
}

volatile uint32_t malloccount=0;

int
_getpid (int n)
{
  (void)(n);
  return 1;
}
# if 1
caddr_t
_sbrk (int incr)
{
  extern void * program_stack_start;
  extern void * __HeapLimit;
  extern void * __end__;
  extern void * _heap_start;
  extern char   end __asm ("end");	/* Defined by the linker.  */
  static char * heap_end;

  char *        prev_heap_end;

#if 0

  prev_heap_end = pvPortMalloc(incr);
  if(!prev_heap_end)
	  errno = ENOMEM;
  return (caddr_t) prev_heap_end;
#else
  if (heap_end == NULL)
    heap_end = &__end__;

  prev_heap_end = heap_end;

  if (heap_end + incr > &__HeapLimit)
    {
      /* Some of the libstdc++-v3 tests rely upon detecting
	 out of memory errors, so do not abort here.  */
	  uartc_writes("alloc failed, total = ");
	  uartc_puth(malloccount);
	  uartc_writes("\r\n");
#if 0
      extern void abort (void);

      _write (1, "_sbrk: Heap and stack collision\n", 32);

      abort ();
#else
      errno = ENOMEM;
      return (caddr_t) -1;
#endif
    }

  malloccount+=incr;
  heap_end += incr;

 // uartc_writes("alloc ");
//  uartc_puth(incr);
//  uartc_writes(" @ ");
//  uartc_puth(prev_heap_end);
//  uartc_writes("\r\n");


  return (caddr_t) prev_heap_end;
#endif
}
#endif

int
_fstat (int file, struct stat * st)
{
  (void)file;
  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFCHR;
  st->st_blksize = 1024;
  return 0;
}

int _stat (const char *fname, struct stat *st)
{
  int file;

  /* The best we can do is try to open the file readonly.  If it exists,
     then we can guess a few things about it.  */
  if ((file = _open (fname, O_RDONLY)) < 0)
    return -1;

  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFREG | S_IRWXU;
  st->st_blksize = 1024;
  _swiclose (file); /* Not interested in the error.  */
  return 0;
}

int
_link (void)
{
  return -1;
}

int
_unlink (const char *path)
{
#ifdef ARM_RDI_MONITOR
  int block[2];
  block[0] = path;
  block[1] = strlen(path);
  return wrap (do_AngelSWI (AngelSWI_Reason_Remove, block)) ? -1 : 0;
#else
  return -1;
#endif
}

void
_raise (void)
{
  return;
}

int
_gettimeofday (struct timeval * tp, void * tzvp)
{
  struct timezone *tzp = tzvp;
  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}

/* Return a clock that ticks at 100Hz.  */
clock_t
_times (struct tms * tp)
{
  clock_t timeval;

  if (tp)
    {
      tp->tms_utime  = timeval;	/* user time */
      tp->tms_stime  = 0;	/* system time */
      tp->tms_cutime = 0;	/* user time, children */
      tp->tms_cstime = 0;	/* system time, children */
    }

  return timeval;
};


int
_isatty (int fd)
{
#ifdef ARM_RDI_MONITOR
  int fh = remap_handle (fd);
  return wrap (do_AngelSWI (AngelSWI_Reason_IsTTY, &fh));
#else
  return (fd <= 2) ? 1 : 0;  /* one of stdin, stdout, stderr */
#endif
}

int
_system (const char *s)
{
#ifdef ARM_RDI_MONITOR
  int block[2];
  int e;

  /* Hmmm.  The ARM debug interface specification doesn't say whether
     SYS_SYSTEM does the right thing with a null argument, or assign any
     meaning to its return value.  Try to do something reasonable....  */
  if (!s)
    return 1;  /* maybe there is a shell available? we can hope. :-P */
  block[0] = s;
  block[1] = strlen (s);
  e = wrap (do_AngelSWI (AngelSWI_Reason_System, block));
  if ((e >= 0) && (e < 256))
    {
      /* We have to convert e, an exit status to the encoded status of
         the command.  To avoid hard coding the exit status, we simply
	 loop until we find the right position.  */
      int exit_code;

      for (exit_code = e; e && WEXITSTATUS (e) != exit_code; e <<= 1)
	continue;
    }
  return e;
#else
  if (s == NULL)
    return 0;
  errno = ENOSYS;
  return -1;
#endif
}

int
_rename (const char * oldpath, const char * newpath)
{
#ifdef ARM_RDI_MONITOR
  int block[4];
  block[0] = oldpath;
  block[1] = strlen(oldpath);
  block[2] = newpath;
  block[3] = strlen(newpath);
  return wrap (do_AngelSWI (AngelSWI_Reason_Rename, block)) ? -1 : 0;
#else
  errno = ENOSYS;
  return -1;
#endif
}

void _fini()
{


}
