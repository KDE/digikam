/*
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2002 Massachusetts Institute of Technology
 * Copyright (c) 2002 Bradley C. Kuszmaul
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or (at
 *  your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 *
 */

#ifndef _CILK_H
#define _CILK_H

#ifdef __CILK__
/* gcc-builtin.h is now compiler-specific builtins for other compilers too.   It is poorly named... */
/* This must be early to make cilk2c happy about the MIPSPRO intrinsics such __synchronize() */
/* It must even be before cilk-sysdep.h and sys/types.h because there are things in the sys/types.h that use the builtins in some systems (Such as Mac Tiger).
 * See Bug 198. */
#include <gcc-builtin.h>
#endif

#include <sys/types.h>

/****************************************************************\
 * Architecture- and compiler-specific definitions and functions
\****************************************************************/
#include <stddef.h> /* ptrdiff_t */

#include <cilk-sysdep.h>

FILE_IDENTITY(cilk_h_ident,
	      "$HeadURL: https://bradley.csail.mit.edu/svn/repos/cilk/5.4.3/runtime/cilk.h $ $LastChangedBy: bradley $ $Rev: 2672 $ $Date: 2005-12-20 13:30:02 -0500 (Tue, 20 Dec 2005) $");

/***********************************************************\
 * Cilk configuration options (profiling, etc)
\***********************************************************/

#ifdef __CILK2C__
#pragma nd -
#endif

#include <cilk-conf.h>

/***********************************************************\
 * Types used by cilk2c output
\***********************************************************/

/*
 * CilkProcInfo fields:
 *          |       NORMAL           |               INLET
 * ---------+------------------------+---------------------------------------
 *     size | size of return value   | size of returned (by SPAWN) value
 *    index | index to put ret. val. | index to put ret. (by INLET_CALL) val.
 *    inlet | 0                      | pointer to inlet function
 *  argsize | 0                      | size of inlet's arguments in frame
 * argindex | ???                    | index of inlet's arguments in frame
 *
 * Hack: the field "inlet" for entry 0 contains the address of 
 * the slow function.  The field "size" for entry 0 contains the size
 * of the value returned by the function as a whole (as opposed to the
 * other entries, which contain the size of the value returned by that
 * spawn/sync statement).
 */

typedef struct {
     int size;
     ptrdiff_t index;
     void (*inlet) ();
     int argsize;
     ptrdiff_t argindex;
} CilkProcInfo;

/*
 * a linked list of blocks allocated by Cilk_alloca.
 */
struct cilk_alloca_header {
     struct cilk_alloca_header *next;
     size_t size;
};

/***********************************************************\
 * Hooks
\***********************************************************/
typedef void (*HookT)(void);

typedef struct hook {
     HookT fn;
     struct hook *next;
} HookList;

extern void Cilk_add_hook(HookList **listp, HookT fn);
extern void Cilk_run_hooks(HookList *list);

#define NULL_HOOK ((HookList *) 0)

/*
 * various types of hooks
 * This list is incomplete and should be updated as needed
 */
extern __CILKSAFE__ HookList *Cilk_init_global_hooks;
extern __CILKSAFE__ HookList *Cilk_init_per_worker_hooks;



/*
 * type of disjoint set data structure for the Nondeterminator
 */
WHEN_CILK_ND(
typedef unsigned int DisjointSetMemberT;
)

/* 
 * a stack frame consists of this header and of a procedure-specific
 * part
 */
typedef struct {
     int entry;
     void *receiver;   /* pointer to the receiver of the outstanding spawn,
 			  unless otherwise specified in CilkProcInfo->index */
     CilkProcInfo *sig;
     WHEN_CILK_ALLOCA(struct cilk_alloca_header *alloca_h;)
     WHEN_CILK_TIMING(Cilk_time mycp;)
     WHEN_CILK_TIMING(Cilk_time cp;)
     WHEN_CILK_TIMING(Cilk_time work;)
     WHEN_CILK_ND(DisjointSetMemberT proc_id;)
     WHEN_CILK_ND(DisjointSetMemberT inlet_id;)
     WHEN_CILK_ND(DisjointSetMemberT s_set;)
     WHEN_CILK_ND(DisjointSetMemberT p_set;)
     WHEN_CILK_DEBUG(volatile unsigned int magic;)
} CilkStackFrame;

/*
 * a stack is an array of frame pointers 
 */
typedef CilkStackFrame **CilkStack;

/*
 * The way head and tail work is a bit tricky.  Conceptually,
 * head and tail belong to Closure; however, when a steal
 * occurs, the Closure of a certain queue is replaced by
 * another Closure.  There is no way to initialize the
 * new Closure with the appropriate head and tail, and install
 * it atomically, unless we agree to lock the victim processor
 * (and we don't want to).  Therefore, the closure contains
 * *pointers* to head and tail.  The actual head and tail are allocated
 * on a per-processor basis.  
 *
 * We allocate this little structure to keep the values of head
 * and tail.  It also caches other important quantities used by
 * the cilk2c output, that should reside in the Closure but
 * are cached here for efficiency reasons.
 */
typedef struct {
     volatile CilkStackFrame **head, **tail;
     /* see sched.c for a description of the exception-handling machinery */
     volatile CilkStackFrame **exception;  
     CilkStack stack;
     CILK_CACHE_LINE_PAD;
} CilkClosureCache;

/* descriptor for an internal_malloc bucket */
struct Cilk_im_descriptor {
     void *free_list;        /* pointer to free list */
     int length;             /* length of the free list */
     /*
      * number of elements that can be freed before a global 
      * batch free is necessary.
      */
     int count;            
};

/* statistics for Cilk_internal_malloc, used mainly for debugging */
struct Cilk_im_stats {
     int used;         /* bytes used; the sum must be 0 at the end */
     int nmalloc;      /* # malloc - # free; sum must be 0 at the end */
     int in_free_lists;   /* bytes in free lists at the end of the execution */

     /* number of elements in each bucket */
     int length[CILK_INTERNAL_MALLOC_BUCKETS];
     CILK_CACHE_LINE_PAD;   /* pad to cache line to avoid false sharing */
};


typedef struct Closure_s Closure; /* FWD declaration */
typedef struct Cilk_options_s Cilk_options;/* FWD declaration */
/*
 * ``parameters'' are set by the global process and they are only read
 * by workers.  It would be nice to map these in a segment which is
 * actually read only.
 */
typedef struct {
 /*
 	* pointer to an array containing various internal-malloc statistics,
 	* in particular the algebraic sum of the memory used by each
 	* processor.  The sum of the array must be 0 at the end of the
 	* program.
 	*/
  	struct Cilk_im_stats *im_info;

	/* globally-visible options (read-only in child processes) */
	Cilk_options *options;

	/*
 	* this string is printed when an assertion fails.  If we just inline
 	* it, apparently gcc generates many copies of the string.
 	*/
	const char *assertion_failed_msg;
	const char *stack_overflow_msg;

	/* Number of processors Cilk is running on */
	__CILKSAFE__ int active_size;
        __CILKSAFE__ int pthread_stacksize;

	/*
 	* HACK: this should be FILE *; but using a void * simplifies the
 	* automatic generation of the file containing all shared/private/ro
 	* variables.
 	*/
	void *infofile;

	/* Timing */
#if CILK_TIMING
	struct StateInfo * timer_state_info;
#endif

	/* stats */
#if CILK_STATS
	struct Stats *stat_array;
#endif

	/*
 	* variables that children need to update (typically at the end
 	* of the program
 	*/
	unsigned int num_threads;
	unsigned int num_steals;
#if CILK_STATS
	unsigned int max_stack_depth;
#endif

	/* dynamically-allocated array of deques, one per processor */
	struct ReadyDeque *deques;

	Cilk_time start_time;

	Closure *invoke_main;

	/* declaration of the various hooks */
	HookList *Cilk_init_global_hooks;
	HookList *Cilk_init_per_worker_hooks;

} CilkReadOnlyParams;


typedef struct CilkGlobalState_s CilkGlobalState; /* Forward declaration*/

typedef struct {

        CilkReadOnlyParams *Cilk_RO_params;
	/*
 	* Cilk_global_state contains all shared variables internal to the
 	* runtime system
 	*/
	CilkGlobalState *Cilk_global_state;

}  CilkContext;


/* worker state */
typedef struct {
     CilkClosureCache cache;
     int self;
     struct Cilk_im_descriptor im_descriptor [CILK_INTERNAL_MALLOC_BUCKETS];
     size_t stackdepth;
     Cilk_time last_cp_time;
     Cilk_time cp_hack;
     Cilk_time work_hack;
     Cilk_time user_work;
     Cilk_time user_critical_path;
     unsigned int rand_next;
     int abort_flag;
     int barrier_direction;
     CILK_CACHE_LINE_PAD;
     CilkContext *context;
#ifdef CILK_USE_PERFCTR
     volatile const struct vperfctr_state *perfctr_kstate;
#endif
} CilkWorkerState;

typedef struct{

	CilkContext *context;
	int id;

} CilkChildParams;

/*
 * Functions defined in the architecture-specific file and used
 * in Cilk programs
 */
extern void Cilk_dprintf(CilkWorkerState *const ws, const char *fmt,...);
extern void Cilk_die_internal(CilkContext *const context, CilkWorkerState *const ws, const char *fmt,...);
extern void Cilk_unalloca_internal(CilkWorkerState *const ws,
				   CilkStackFrame *f);

/*
 * Functions defined by the scheduler and used in Cilk programs
 */
extern void *Cilk_internal_malloc(CilkWorkerState *const ws, size_t);
extern void Cilk_internal_free(CilkWorkerState *const ws, void *p, size_t size);

/***********************************************************\
 * Nondeterminator race detection
\***********************************************************/
#ifdef CILK_ND
#include <nd/cilk-nd.h>
#endif

/*
 * API Functions
*/
extern CilkContext *Cilk_init(int* argc,char** argv);
extern void Cilk_terminate(CilkContext *const context);

/***********************************************************\
 * Critical path elapsed time calculator
\***********************************************************/

static inline Cilk_time Cilk_get_elapsed_time(CilkWorkerState *const ws)
{
     Cilk_time then = ws->last_cp_time;
     Cilk_time now = Cilk_get_time();

     CILK_ASSERT(ws, now >= then);

     ws->last_cp_time = now;
     return now - then;
}

/* internal malloc stuff, used by cilk2c's output */

/*
 * this is written so that it can be __inline__d and partially
 * evaluated when size is a constant 
 */
#define CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, n) \
     if (size <= n && n >= CILK_CACHE_LINE) return n;

static inline int Cilk_internal_malloc_canonicalize(size_t size)
{
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 16);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 32);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 64);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 128);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 256);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 512);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 1024);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 2048);
     CILK_INTERNAL_MALLOC_CANONICALIZE_MACRO(size, 4096);
     return -1;  /* keep gcc happy */
}

#define CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, n, bucket) \
     if (size <= n && n >= CILK_CACHE_LINE) return bucket;

static inline int Cilk_internal_malloc_size_to_bucket(size_t size)
{
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 16, 0);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 32, 1);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 64, 2);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 128, 3);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 256, 4);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 512, 5);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 1024, 6);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 2048, 7);
     CILK_INTERNAL_MALLOC_SIZE_TO_BUCKET(size, 4096, 8);
     return -1;  /* keep gcc happy */
}

#define CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, n, bucket) \
     if (bucket == b) return n;

static inline int Cilk_internal_malloc_bucket_to_size(int b)
{
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 16, 0);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 32, 1);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 64, 2);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 128, 3);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 256, 4);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 512, 5);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 1024, 6);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 2048, 7);
     CILK_INTERNAL_MALLOC_BUCKET_TO_SIZE(b, 4096, 8);
     return -1;  /* keep gcc happy */
}

/***********************************************************\
 * memory barriers
\***********************************************************/
/*
 *  Ensure that all previous memory operations are completed before
 *  continuing.
 */
static inline void Cilk_fence(void)
{
     CILK_MB();
}

/*
 *  Ensure that all previous writes are globally visible before any
 *  future writes become visible.
 */
static inline void Cilk_membar_StoreStore(void)
{
     CILK_WMB();
}

/*
 *  Ensure that all previous writes are globally visible before any
 *  future reads are performed.
 */
static inline void Cilk_membar_StoreLoad(void)
{
     CILK_RMB();
}


/************************************************************
 * Scheduler functions used in Cilk programs
 ************************************************************/
extern int Cilk_sync(CilkWorkerState *const ws);
extern int Cilk_exception_handler(CilkWorkerState *const ws, void *, int);
extern void Cilk_set_result(CilkWorkerState *const ws, 
			    void *resultp, int size);
extern void Cilk_after_sync_slow_cp(CilkWorkerState *const ws,
				    Cilk_time *work, Cilk_time *cp);
extern void Cilk_abort_standalone(CilkWorkerState *const ws);
extern void Cilk_abort_slow(CilkWorkerState *const ws);
extern void Cilk_event_new_thread(CilkWorkerState *const ws);
extern void Cilk_destroy_frame(CilkWorkerState *const ws,
			       CilkStackFrame *f, size_t size);

/***********************************************************
 *  cilk2c-only stuff
 ***********************************************************/
#ifdef __CILK__
/*
 * make sure the user compiles a file through cilk2c in order
 * to use cilk_main
 */
/* Ofra and Sivan: we re-enabled main->cilk_main, 19 June 2003 */
/* then we disabled it again */
/*#define main cilk_main*/

/* extern __nooutput__ cilk int main(); */

#include <cilk-cilk2c-pre.h>
#endif

/************************************************************
 * black magic to make sure that the RTS and the application
 * have the same flags
 ************************************************************/
#if CILK_DEBUG
#define CILK_NAME_DEBUG DEBUG
#else
#define CILK_NAME_DEBUG NODEBUG
#endif

#if CILK_TIMING
#define CILK_NAME_TIMING TIMING
#else
#define CILK_NAME_TIMING NOTIMING
#endif

#if CILK_STATS
#define CILK_NAME_STATS STATS
#else
#define CILK_NAME_STATS NOSTATS
#endif

#define CILK_MAGIC_NAME_MAGIC(a,b,c) \
    Cilk_flags_are_wrong_ ## a ## _ ## b ## _ ## c ## _please_recompile
#define CILK_MAGIC_NAME_MORE_MAGIC(a,b,c) CILK_MAGIC_NAME_MAGIC(a,b,c)
#define CILK_MAGIC_NAME  \
   CILK_MAGIC_NAME_MORE_MAGIC(CILK_NAME_DEBUG, CILK_NAME_TIMING, \
			      CILK_NAME_STATS)

extern __CILKSAFE__ int CILK_MAGIC_NAME;
static __CILKSAFE__ int *Cilk_check_flags_at_link_time =  &CILK_MAGIC_NAME;

static int UNUSED(Cilk_check_flags_at_link_time_hack(void));
static int Cilk_check_flags_at_link_time_hack(void) {
     return *Cilk_check_flags_at_link_time;
}

void Cilk_start(CilkContext *const context,
		void (*main)(CilkWorkerState *const ws, void *args),
		void *args,
		int return_size );
void Cilk_free(void *);
void *Cilk_malloc_fixed(size_t);

/* ??? Cilk_fake_lock and so forth probably need to be defined. */
#ifdef __CILK2C__
#pragma nd +
#endif
#endif				/* _CILK_H */
