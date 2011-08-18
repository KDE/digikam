/*
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
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

#include <cilk.h>
#include <cilk-internal.h>
#include <stdio.h>

#ifdef CILK_USE_PERFCTR
volatile const struct vperfctr_state *__cilk_vperfctr_init(int);
#endif

static char *smart_sprint_time(double x)
{
     static char buf[128];

     if (x < 1.0E-3)
	  sprintf(buf, "%f us", x * 1.0E6);
     else if (x < 1.0)
	  sprintf(buf, "%f ms", x * 1.0E3);
     else
	  sprintf(buf, "%f s", x);

     return buf;
}

/*
 * The statistics printer
 */
static void print_all_statistics(CilkContext *const context)
{

     if (USE_PARAMETER1(options->statlevel) >= 1) {
	  /* Print Header line for statistics */
	  fprintf(USE_PARAMETER1(infofile),
		  "\nRUNTIME SYSTEM STATISTICS:\n"
		  "\n");
	  /* level 1 and above: print wall clock, work and CP */
	  fprintf(USE_PARAMETER1(infofile),
		  "Wall-clock running time on %d processor%s: %s\n",
		  USE_PARAMETER1(active_size),
		  USE_PARAMETER1(active_size) > 1 ? "s" : "",
		  smart_sprint_time(Cilk_wall_time_to_sec(
		       Cilk_get_wall_time() - USE_PARAMETER1(start_time))));
	  WHEN_CILK_TIMING({
	       fprintf(USE_PARAMETER1(infofile), "Total work = %s\n",
		       smart_sprint_time(Cilk_compute_work(context)));
	  });

	  /*
	   * hack: we don't consider the total work if it is too
	   * small.  The trick allows us to compile the rts with
	   * CILK_CRITICAL_PATH=something and the user program with
	   * CILK_CRITICAL_PATH=0
	   */
	  WHEN_CILK_TIMING({
	       if (Cilk_time_to_sec(USE_SHARED1(total_work)) > 0.001) {
		    fprintf(USE_PARAMETER1(infofile),
			    "Total work (accumulated) = %s\n",
			    smart_sprint_time(
				 Cilk_time_to_sec(USE_SHARED1(total_work))));
		    fprintf(USE_PARAMETER1(infofile),
			    "Span = %s\n",
			    smart_sprint_time(
				 Cilk_time_to_sec(USE_SHARED1(critical_path))));
		    fprintf(USE_PARAMETER1(infofile),
			    "Parallelism = %f\n",
			    (double) USE_SHARED1(total_work) /
			    (double) USE_SHARED1(critical_path));
	       }
	  });
     }

     if (USE_PARAMETER1(options->statlevel) >= 2) {
	  /*
	   *  level 2 and above: print statistics on work stealing etc
	   */
	  Cilk_print_rts_statistics(context);
	  WHEN_CILK_TIMING({
	       if (Cilk_time_to_sec(USE_SHARED1(total_work)) > 0.001
		   && USE_PARAMETER1(num_threads) > 0) {
		    fprintf(USE_PARAMETER1(infofile),
			    "AVERAGE THREAD LENGTH = %s\n",
			    smart_sprint_time(
				 Cilk_time_to_sec(
				      USE_SHARED1(total_work)) /
				 (double) USE_PARAMETER1(num_threads)));
	       }

	       if (Cilk_time_to_sec(USE_SHARED1(total_work)) > 0.001
		   && USE_PARAMETER1(num_steals) > 0) {
		    fprintf(USE_PARAMETER1(infofile),
			    "AVERAGE SUBCOMPUTATION LENGTH = %s\n",
			    smart_sprint_time(
				 Cilk_time_to_sec(
				      USE_SHARED1(total_work)) /
				 ((double) USE_PARAMETER1(num_steals) + 1.0)));
	       }
		fprintf(USE_PARAMETER1(infofile),
		       "MAX STACK DEPTH = %d\n",
		       USE_PARAMETER1(max_stack_depth));

	  });
     }
     if (USE_PARAMETER1(options->statlevel) >= 3) {
	  /* level 3 and above: memory statistics */
	  Cilk_internal_malloc_print_statistics(context);
     }
     if (USE_PARAMETER1(options->statlevel) >= 4) {
	  /* level 4 and above: print work done by each processor */
	  Cilk_print_time_statistics(context);
     }
     if (USE_PARAMETER1(options->statlevel) >= 5) {
	  /* level 5 and above: print summary breakout of time */
	  Cilk_summarize_time_statistics(context);
     }
     if (USE_PARAMETER1(options->statlevel) >= 6) {
	  /* level 6 and above: print detailed breakout of time */
	  Cilk_print_detailed_time_statistics(context);
     }

}


/*
 * allocate some chunk of shared memory for global variables
 */
void Cilk_create_global_state(CilkContext *const context)
{
     context->Cilk_global_state =
	  Cilk_malloc_fixed(sizeof (CilkGlobalState));
     CILK_CHECK(context->Cilk_global_state,
		(context, NULL, "Cannot allocate global state\n"));
}

void Cilk_free_global_state(CilkContext *const context)
{
     Cilk_free(context->Cilk_global_state);
}

/*
 * allocate some chunk of memory for context
 */
void Cilk_create_context(CilkContext ** context)
{
     *context =
	  Cilk_malloc_fixed(sizeof (CilkContext));
     CILK_CHECK(*context,
		(*context, NULL, "Cannot allocate Cilk context object\n"));

     /* setup the address space */
     Cilk_create_global_state(*context);

     (*context)->Cilk_RO_params =
	  Cilk_malloc_fixed(sizeof (CilkReadOnlyParams));
     CILK_CHECK((*context)->Cilk_RO_params,
		(*context, NULL, "Cannot allocate Cilk context object\n"));

     (*context)->Cilk_RO_params->options =
	  Cilk_malloc_fixed(sizeof (Cilk_options));
     CILK_CHECK((*context)->Cilk_RO_params->options,
		(*context, NULL, "Cannot allocate Cilk context object\n"));

     (*context)->Cilk_RO_params->Cilk_init_global_hooks = NULL_HOOK;
     (*context)->Cilk_RO_params->Cilk_init_per_worker_hooks = NULL_HOOK;
}

void Cilk_free_context(CilkContext *const context)
{
     Cilk_free(context->Cilk_RO_params->options);
     Cilk_free(context->Cilk_RO_params);
     Cilk_free(context);
}

static void init_variables(CilkContext *context)
{
     /* initialize all shared_ro variables declared above */
     INIT_PARAMETER1(active_size, USE_PARAMETER1(options->nproc));
     INIT_PARAMETER1(infofile, (FILE *)0); /*pointer to the stats. output file)*/
     INIT_PARAMETER1(pthread_stacksize, USE_PARAMETER1(options->pthread_stacksize));
     INIT_PARAMETER1(assertion_failed_msg,
		    "Assertion failed: %s line %d file %s\n"
		    "This is either a Cilk bug, or your program\n"
		    "has corrupted some Cilk internal data structure.\n");
     INIT_PARAMETER1(stack_overflow_msg,
		    "Cilk runtime system: Stack overflow.\n"
		    "Your program may have infinite recursion.\n");
}

static void init_variables_2( CilkContext *const context )
{
     /* initialize all shared_ro variables declared above */
     INIT_PARAMETER1(start_time, Cilk_get_wall_time());
     INIT_PARAMETER1(num_threads, 0); /*for stats */
     INIT_PARAMETER1(num_steals, 0); /*for stats */
#if CILK_STATS
     INIT_PARAMETER1(max_stack_depth, 0);
#endif

}

/* global init */
static void Cilk_global_init(CilkContext *const context)
{
     init_variables(context);

     /* debug module */
     Cilk_debug_init(context);

     /* architecture-specific module */
     Cilk_arch_specific_init(); /* global init or init per cilk invokation ? */

     /* initialize barrier code */
     Cilk_barrier_init(context);

     /* run initialization hooks for library functions */
     Cilk_run_hooks(USE_PARAMETER1(Cilk_init_global_hooks)); /**???????? - where to put it ? What's that ? */

     /* Initialize stats code */
     Cilk_stats_init(context);

     /* Initialize timing */
     Cilk_timing_init(context);
}

void Cilk_terminate(CilkContext *const context)
{
     /* Terminate the children threads */
     Cilk_terminate_children(context);

     /* everybody is done: print stats and return (used to be exit) */
     print_all_statistics(context);

     /* Terminate the scheduler*/
     Cilk_scheduler_terminate(context);

     /* Terminate timing */
     Cilk_timing_terminate(context);

     /* Terminate stats code */
     Cilk_stats_terminate(context);

     /* Terminate barrier code */
     Cilk_barrier_terminate(context);

     /* Terminate debug code */
     Cilk_debug_terminate(context);

     /* Free the address space */
     Cilk_free_global_state(context);

     Cilk_free_context(context);
}

static void Cilk_global_init_2(CilkContext *const context)
{
     init_variables_2(context);

     USE_SHARED1(critical_path) = 0;
     USE_SHARED1(total_work) = 0;

     /* initialize the statistics gatherer and timer  - ALSO ALLOCATING EACH TIME,
	  MAKE SURE THAT IT IS RELEASED EACH TIME> TODO - SEPARATE ALLOCATION AND INIT TO ZERO */
     Cilk_event_gathering_init(context);
     Cilk_time_gathering_init(context);

}

static CilkWorkerState *create_worker_state(CilkContext *const context, long id)
{
     CilkWorkerState *const ws = Cilk_malloc_fixed(sizeof(CilkWorkerState));

     ws->self = id;
     
     ws->context = context;

#ifdef CILK_USE_PERFCTR
     ws->perfctr_kstate = __cilk_vperfctr_init(0);
#endif

     Cilk_barrier_per_worker_init(ws);

     Cilk_arch_specific_per_worker_init();

     /* initialize scheduler per worker */
     Cilk_scheduler_per_worker_init(ws);

     /* run initialization hooks for library functions */
     Cilk_run_hooks(USE_PARAMETER(Cilk_init_per_worker_hooks));

     Cilk_internal_malloc_per_worker_init(ws);

     return ws;

}

/*
 * this function is the entry point for child processes
 */
static void Cilk_child_main(CilkChildParams *const childParams)
{
     int local_terminating = 0;

     int id = childParams->id;
     CilkContext *const context = childParams->context;

     /* Init worker state object */
     CilkWorkerState *const ws = create_worker_state(context, id);

     while( ! local_terminating ) {
     	/* Now wait for invocation */
	Cilk_worker_wait_for_invocation(context, id, &local_terminating);

 	if(! local_terminating){
     		if (id == 0)
		     Cilk_scheduler(ws, USE_PARAMETER(invoke_main));
		else 
		     Cilk_scheduler(ws, (Closure *) NULL);

		Cilk_worker_is_done(context, &local_terminating);
	}
    }

    /* Terminate */

    Cilk_internal_malloc_per_worker_cleanup(ws);

    Cilk_scheduler_per_worker_terminate(ws);

    Cilk_free(ws);
}

CilkContext* Cilk_init( int* argc, char *argv[] )
{

     CilkContext* context;

     Cilk_create_context(&context);

     /* parse command-line arguments - CILK command line arguments are relevant to all the process*/
     if (Cilk_parse_command_line(USE_PARAMETER1(options), argc, argv)) {
	  fprintf(stderr, "Failed parsing command line\n");
	  Cilk_free_context(context);
	  return NULL;
     }
     
#ifdef HAVE_SCHED_SETAFFINITY
     /*default setting*/
     if( USE_PARAMETER1(options)->pinned_mask > 1023 ) {
	  /*do nothing (use the mask that was inherited) */
     } else {
	  if( sched_setaffinity(0, sizeof( USE_PARAMETER1(options)->pinned_mask ), &( USE_PARAMETER1(options)->pinned_mask ) ) )
	  {
	       fprintf(stderr, "Failed pinning process, continuing on default mask...\n");
	   }
     }
#endif

     /* initialize global part of system */
     Cilk_global_init(context);

     Cilk_scheduler_init(context);

     /* create threads for the children */
     Cilk_create_children(context, Cilk_child_main);

     return context;
}

void Cilk_start(CilkContext *const context,
		void (*main)(CilkWorkerState *const ws, void *args),
		void *args,
		int return_size )
{
     Cilk_global_init_2(context);

     /* initialize the scheduler */
     Cilk_scheduler_init_2(context);

     USE_PARAMETER1(invoke_main) =
	  Cilk_create_initial_thread(context, main, args, return_size);

     /*When this returns we are done */
     Cilk_wakeup_workers(context);

     Cilk_internal_malloc_global_cleanup(context);

     /* clean scheduler */
     Cilk_scheduler_terminate_2(context);

}

void Cilk_really_exit_1(CilkWorkerState *const ws,
			int res) {
     Cilk_exit_from_user_main(
	  ws, USE_PARAMETER(invoke_main), res);
}

/* magic consistency check - for compilation/link*/
int CILK_MAGIC_NAME;
