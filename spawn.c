/*
Spawn Library
Copyright 2016 Russell Leidich
http://spawnthread.blogspot.com

This collection of files constitutes the Spawn Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Spawn Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Spawn Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Spawn Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
void
spawn_free(void *base){
/*
To maximize portability and debuggability, this is the only place where Spawn frees memory.

In:

  base is the return value of spawn_malloc(). May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  free(base);
  return;
}

void *
spawn_malloc(ULONG size_minus_1){
/*
To maximize portability and debuggability, this is the only place where Spawn allocates memory.

In:

  size_minus_1 is the number of bytes to allocate, less 1 to ensure that we don't invite stupidity.

Out:

  base is the base of a memory allocation of (size_minus_1+1) bytes, or NULL if allocation failed.
*/
  void *base;
  ULONG size;

  size=size_minus_1+1;
  base=NULL;
  if(size){
    base=malloc((size_t)(size));
  }
  return base;
}
#ifdef PTHREAD
  void
  spawn_multi_pthread_join(spawn_simulthread_t *simulthread_base){
/*
Force pthread_join() to succeed, or don't return. Do not call from outside Spawn.

In:

  simulthread_base->pthread corresponds to the simulthread to be flushed.

Out:

  The indicated simulthread has flushed successfully.
*/
   int pthread_status;

    do{
      pthread_status=pthread_join(simulthread_base->pthread,NULL);
/*
No matter what, do not exit this loop until the pthread_join has succeeded. If we hang, we hang. Better that, than corrupting memory and risking uncontrolled OS calls due to freeing memory in use by another thread.
*/
    }while(pthread_status);
    return;
  }

  u8
  spawn_multi_one(spawn_t *spawn_base,ULONG unique_idx){
/*
Spawn a thread asynchronously.

In:

  unique_idx is a thread index used once until spawn_multi_retire_all() is called, after which it may be reused. It's used to address the reason for which this function is called instead of spawn_multi(), namely, that the thread indexes of interest aren't known ahead of time. For example, a master thread could browse a list, looking for work to do. Then some of the items in the list would invoke slave threads, whereas others would not. In this example, a linearly increasing thread index would be of little use to the target function. unique_idx gets around this problem by permitting thread indexes to be sparse, and even out-of-order.

  *spawn_base is as returned by spawn_multi_init().

Out:

  Returns 1 on failure, else 0. Success means that the thread was launched (but might not have retired). Failure will only be returned in the case of a fatal error, as opposed to a temporary failure caused by the OS being overloaded with threads.

  Regardless of the return value, the caller must not call any other Spawn function, except this one, until spawn_multi_retire_all() has been called -- unless the call involves purely orthogonal writable data structures, including a separate *spawn_base.
*/
    void (*function_base)(spawn_simulthread_context_t *);
    int pthread_status;
    u8 simulthread_active_status;
    spawn_simulthread_t *simulthread_base;
    u32 simulthread_idx_max;
    u32 simulthread_launch_idx;
    u8 simulthread_launched_status;
    spawn_simulthread_t *simulthread_list_base;
    u32 simulthread_retire_idx;
    u8 status;

    function_base=spawn_base->function_base;
    simulthread_list_base=spawn_base->simulthread_list_base;
    simulthread_idx_max=spawn_base->simulthread_idx_max;
    simulthread_launch_idx=spawn_base->simulthread_launch_idx;
    simulthread_retire_idx=spawn_base->simulthread_retire_idx;
    simulthread_active_status=spawn_base->simulthread_active_status;
    if(simulthread_active_status&&(simulthread_launch_idx==simulthread_retire_idx)){
/*
We're maxed out on simulthreads. Retire one so we can launch one.
*/
      simulthread_base=&simulthread_list_base[simulthread_retire_idx];
      spawn_multi_pthread_join(simulthread_base);
      simulthread_retire_idx++;
      if(simulthread_retire_idx>simulthread_idx_max){
        simulthread_retire_idx=0;
      }
/*
We were maxed out on simulthreads, then retired one of them. The only way that there could be no more active simulthreads is if simulthread_idx_max==0.
*/
      simulthread_active_status=(!!simulthread_idx_max);
    }
/*
Launch the new simulthread with thread_idx==unique_idx.
*/
    simulthread_base=&simulthread_list_base[simulthread_launch_idx];
    simulthread_base->context.thread_idx=unique_idx;
    do{
      pthread_status=pthread_create(&simulthread_base->pthread,NULL,(void *)(function_base),&simulthread_base->context);
      if(pthread_status){
        status=1;
        simulthread_launched_status=0;
        if((pthread_status==EAGAIN)||((pthread_status==ENOMEM)&&simulthread_active_status)){
/*
We may or may not have active simulthreads, but the OS is too busy to launch another thread. (If we have ENOMEM but no active simulthreads, that's a fatal error because there's no indication that the OS will ever be able to launch another thread. In that case, we can't arrive here.) If any simulthreads are active, then retire one of them, so we can hopefully launch another one.
*/
          status=0;
          if(simulthread_active_status){
            spawn_multi_pthread_join(simulthread_base);
            simulthread_retire_idx++;
            if(simulthread_retire_idx>simulthread_idx_max){
              simulthread_retire_idx=0;
            }
            simulthread_active_status=(simulthread_launch_idx!=simulthread_retire_idx);
          }
        }
      }else{
        status=0;
        simulthread_launched_status=1;
        simulthread_active_status=simulthread_launched_status;
        simulthread_launch_idx++;
        if(simulthread_launch_idx>simulthread_idx_max){
          simulthread_launch_idx=0;
        }
      }
    }while(!(status|simulthread_launched_status));
    spawn_base->simulthread_launch_idx=simulthread_launch_idx;
    spawn_base->simulthread_retire_idx=simulthread_retire_idx;
    spawn_base->simulthread_active_status=simulthread_active_status;
    return status;
  }

  u8
  spawn_multi(spawn_t *spawn_base,ULONG thread_idx_max){
/*
Keep the OS thread engine as busy as possible with pending threads, within the specified simultaneous thread limit. Make sure your threads are long enough that the typical launch latency (perhpas 1 ms) isn't significant.

In:

  This function must not be recursed or nested within spawn_multi_one(). Otherwise it's possible that OS thread handles will be exhausted.

  thread_idx_max is the maximum thread number.

  *spawn_base is as returned by spawn_multi_init().

Out:

  Returns 1 if not all threads were launched successfully, else 0. Failure will only be returned in the case of a fatal error, as opposed to a temporary failure caused by the OS being overloaded with threads.

  Regardless of the return value, the caller must not call any other Spawn function, including this one, until spawn_multi_retire_all() has been called -- unless the call involves purely orthogonal writable data structures, including a separate *spawn_base.
*/
    ULONG i;
    u8 status;

    i=0;
    do{
      status=spawn_multi_one(spawn_base,i);
    }while((!status)&&((i++)!=thread_idx_max));
    return status;
  }

  void
  spawn_multi_retire_all(spawn_t *spawn_base){
/*
Retire all pending threads.

In:

  *spawn_base is as returned by spawn_multi_init().

Out:

  All pending threads, if any, have finished. The caller must, in general, call spawn_multi_rewind(), but can sometimes avoid that step (see its documentation). If all work is done, then the caller can directly call spawn_multi_free() without calling spawn_multi_rewind().
*/
    u8 simulthread_active_status;
    spawn_simulthread_t *simulthread_base;
    u32 simulthread_idx_max;  
    u32 simulthread_launch_idx;
    spawn_simulthread_t *simulthread_list_base;
    u32 simulthread_retire_idx;

    simulthread_active_status=spawn_base->simulthread_active_status;
    if(simulthread_active_status){
      simulthread_list_base=spawn_base->simulthread_list_base;
      simulthread_idx_max=spawn_base->simulthread_idx_max;
      simulthread_launch_idx=spawn_base->simulthread_launch_idx;
      simulthread_retire_idx=spawn_base->simulthread_retire_idx;
      do{
        simulthread_base=&simulthread_list_base[simulthread_retire_idx];
        spawn_multi_pthread_join(simulthread_base);
        simulthread_retire_idx++;
        if(simulthread_retire_idx>simulthread_idx_max){
          simulthread_retire_idx=0;
        }
      }while(simulthread_retire_idx!=simulthread_launch_idx);
    }
    spawn_base->simulthread_launch_idx=0;
    spawn_base->simulthread_retire_idx=0;
    spawn_base->simulthread_active_status=0;
    return;
  }

  void
  spawn_multi_free(spawn_t *spawn_base){
    if(spawn_base){
      spawn_free(spawn_base->simulthread_list_base);
      spawn_free(spawn_base);
    }
    return;
  }

  void
  spawn_multi_rewind(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base,spawn_t *spawn_base){
/*
Reset the Spawn engine without changing the memory map. It is not necessary to call this function unless a new function_base or readonly_string_base is to be used in a subsequent spawn_multi() or spawn_multi_one(), in which case, it must be called after spawn_multi_retire_all().

In:

  function_base is as defined in spawn_multi_init():In.

  readonly_string_base is as defined in spawn_multi_init():In.

  *spawn_base is as returned by spawn_multi_init().
*/
    u32 i;
    u32 simulthread_idx_max;
    spawn_simulthread_t *simulthread_list_base;

    spawn_base->function_base=function_base;
    spawn_base->simulthread_launch_idx=0;
    spawn_base->simulthread_retire_idx=0;
    spawn_base->simulthread_active_status=0;
    simulthread_list_base=spawn_base->simulthread_list_base;
    simulthread_idx_max=spawn_base->simulthread_idx_max;
    i=0;
    do{
      simulthread_list_base[i].context.readonly_string_base=readonly_string_base;
    }while((i++)!=simulthread_idx_max);
    return;
  }

  spawn_t *
  spawn_multi_init(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base,u32 simulthread_idx_max){
/*
Initialize the Spawn engine for multithreaded mode.

In:

  function_base is the base of a function which accepts a (spawn_simulthread_context_t *) and has no return value (because all communication occurs via memory). Typically, this is a bridge function which then invokes the "real" thread code.

  readonly_string_base is NULL, or the base of a string to which all threads shall be given read access, via spawn_simulthread_context_t.readonly_string_base.

  simulthread_idx_max is 1 less than the maximum allowable number of simultaneous threads ("simulthreads") in flight. All values are valid. Useful to limit resource consumption and kernel overhead due to excessive simulthreads. The simulthread index gets copied to the spawn_simulthread_context_t.simulthread_idx, which will never exceed spawn_simulthread_context_t.thread_idx. (In monothreaded mode, simulthread_idx is always 0.) Both values can be read via the pointer passed to the function at function_base.

Out:

  Returns NULL on failure, else a (spawn_t *) for use with future calls to the Spawn engine.
*/
    u32 i;
    spawn_simulthread_t *simulthread_list_base;
    u64 simulthread_list_size;
    spawn_t *spawn_base;

    simulthread_list_size=simulthread_idx_max;
    simulthread_list_size++;
    simulthread_list_size*=sizeof(spawn_simulthread_t);
    simulthread_list_base=NULL;
    if(simulthread_list_size<=ULONG_MAX){
      simulthread_list_base=(spawn_simulthread_t *)(spawn_malloc((ULONG)(simulthread_list_size-1)));
    }
    spawn_base=NULL;
    if(simulthread_list_base){
      spawn_base=(spawn_t *)(spawn_malloc(sizeof(spawn_t)-1));
      if(spawn_base){
        spawn_base->function_base=function_base;
        spawn_base->simulthread_list_base=simulthread_list_base;
        spawn_base->simulthread_idx_max=simulthread_idx_max;
        spawn_base->simulthread_launch_idx=0;
        spawn_base->simulthread_retire_idx=0;
        spawn_base->simulthread_active_status=0;
        i=0;
        do{
          simulthread_list_base[i].context.readonly_string_base=readonly_string_base;
          simulthread_list_base[i].context.simulthread_idx=i;
        }while((i++)!=simulthread_idx_max);
      }else{
        spawn_free(simulthread_list_base);
      }
    }
    return spawn_base;
  }
#else
  u8
  spawn_mono_one(spawn_t *spawn_base,ULONG unique_idx){
/*
Monothreaded emulation of spawn_multi_one() for verification purposes or unicore environments.

In:

  unique_idx is as defined in spawn_multi_one():In.

  *spawn_base is as returned by spawn_mono_init().

Out:

  Returns 0 for compatibility with spawn_multi_one().

  The caller must not call any other Spawn function except this one, until spawn_mono_rewind() or spawn_mono_free() has been called.
*/
    void (*function_base)(spawn_simulthread_context_t *);
    spawn_simulthread_context_t *simulthread_context_base;
    spawn_simulthread_t *simulthread_list_base;

    function_base=spawn_base->function_base;
    simulthread_list_base=spawn_base->simulthread_list_base;
    simulthread_context_base=&simulthread_list_base->context;
    simulthread_context_base->thread_idx=unique_idx;
    function_base(simulthread_context_base);
    return 0;
  }

  u8
  spawn_mono(spawn_t *spawn_base,ULONG thread_idx_max){
/*
Monothreaded emulation of spawn_multi() for verification purposes or unicore environments.

In:

  thread_idx_max is as defined in spawn_multi():In.

  *spawn_base is as returned by spawn_mono_init().

Out:

  Returns 0 for compatibility with spawn_multi().
*/
    void (*function_base)(spawn_simulthread_context_t *);
    ULONG i;
    spawn_simulthread_context_t *simulthread_context_base;
    spawn_simulthread_t *simulthread_list_base;

    simulthread_list_base=spawn_base->simulthread_list_base;
    function_base=spawn_base->function_base;
    simulthread_context_base=&simulthread_list_base->context;
    i=0;
    do{
      simulthread_context_base->thread_idx=i;
      function_base((void *)(simulthread_context_base));
    }while((i++)!=thread_idx_max);
    return 0;
  }

  void
  spawn_mono_free(spawn_t *spawn_base){
    if(spawn_base){
      spawn_free(spawn_base->simulthread_list_base);
      spawn_free(spawn_base);
    }
    return;
  }

  void
  spawn_mono_rewind(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base,spawn_t *spawn_base){
/*
Reset the Spawn engine without changing the memory map. It is not necessary to call this function unless a new function_base or readonly_string_base is to be used in a subsequent spawn_mono() or spawn_mono_one().

In:

  function_base is as defined in spawn_mono_init():In.

  readonly_string_base is as defined in spawn_mono_init():In.

  *spawn_base is as returned by spawn_mono_init().
*/
    spawn_simulthread_t *simulthread_list_base;

    spawn_base->function_base=function_base;
    simulthread_list_base=spawn_base->simulthread_list_base;
    simulthread_list_base->context.readonly_string_base=readonly_string_base;
    return;
  }

  spawn_t *
  spawn_mono_init(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base){
/*
Initialize the Spawn engine for monothreaded mode.

In:

  function_base is as defined in spawn_multi_init():In.

  readonly_string_base is as defined in spawn_multi_init():In

Out:

  Returns NULL on failure, else a (spawn_t *) for use with future calls to the Spawn engine.
*/
    spawn_simulthread_t *simulthread_list_base;
    spawn_t *spawn_base;

    simulthread_list_base=(spawn_simulthread_t *)(spawn_malloc(sizeof(spawn_simulthread_t)-1));
    spawn_base=NULL;
    if(simulthread_list_base){
      spawn_base=(spawn_t *)(spawn_malloc(sizeof(spawn_t)-1));
      if(spawn_base){
        spawn_base->function_base=function_base;
        spawn_base->simulthread_list_base=simulthread_list_base;
        simulthread_list_base->context.readonly_string_base=readonly_string_base;
        simulthread_list_base->context.simulthread_idx=0;
      }else{
        spawn_free(simulthread_list_base);
      }
    }
    return spawn_base;
  }
#endif
