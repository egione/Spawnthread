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
/*
Demo for Spawn

This is a fake global maximization problem, implemented alternately using SPAWN() and SPAWN_ONE(). Both methods should of course produce the same output.

Learn by reading comments and tracing the code. Relax. It's only a stupid demo. But it's a template for porting Spawn quickly and easily.
*/
#include "flag.h"
#include "unix_include.h"
#include "constant.h"
#include "spawn.h"
#include "spawn.c"
/*
We have a readonly (global) string which fakes out a data table. FAKE_DATA_SIZE is its size.
*/
#define FAKE_DATA_SIZE 39
/*
simulthread_local_t is scratch space used by a thread during its execution, but not preserved after it returns. This sort of structure is useful for large temporary object storage which is too big for the stack.
*/
TYPEDEF_START
  u32 some_temporary_variable;
TYPEDEF_END(simulthread_local_t)
/*
thread_local_t is stuff that is initialized by a thread, and must be preserved after it terminates. In this case, each thread finds some maximum of a variable, here, fake_x_max, and stores that maximum in this structure. At the end of the day, the maximum such maximum can be found by the root thread, which is then the global maximum.
*/
TYPEDEF_START
  u64 fake_x_max;
  i32 other_thready_stuff;
TYPEDEF_END(thread_local_t)
/*
thread_global_t is what Spawn refers to as *readonly_string_base. It's a readonly string shared by all threads. It points to lists containing (simulthread_local_t)s and (thread_local_t)s, but could also contain other data as well.
*/
TYPEDEF_START
  simulthread_local_t *simulthread_local_list_base;
  thread_local_t *thread_local_list_base;
  u16 *fake_data_base;
TYPEDEF_END(thread_global_t)
/*
thread_execute is the worker thread that Spawn launches, via SPAWN() or SPAWN_ONE().
*/
void
thread_execute(spawn_simulthread_context_t *spawn_simulthread_context_base){
  u16 *fake_data_base;
  u64 fake_x;
  u64 fake_x_max;
  u32 i;
  u32 simulthread_idx;
  simulthread_local_t *simulthread_local_base;
  thread_global_t *thread_global_base;
  ULONG thread_idx;
  thread_local_t *thread_local_base;
/*
Fetch the base of the global data structure.
*/
  thread_global_base=(thread_global_t *)(spawn_simulthread_context_base->readonly_string_base);
/*
Find out what thread index this is, so we know what work to do.
*/
  thread_idx=spawn_simulthread_context_base->thread_idx;
/*
Find the simulthread index as well, which will tell us which simulthread_local_t we're allowed to modify.
*/
  simulthread_idx=spawn_simulthread_context_base->simulthread_idx;
/*
Initialize our scratch space.
*/
  simulthread_local_base=&thread_global_base->simulthread_local_list_base[simulthread_idx];
  simulthread_local_base->some_temporary_variable=0;
/*
Find the base of the fake readonly data string. In reality, this might be a database or a matrix for analysis.
*/
  fake_data_base=thread_global_base->fake_data_base;
/*
Load some fake X variable with the thread index. This is simply a means by which to map the thread index to a task to do. In this case, the task is simply to iterate the thread index, looking for the maximum value that we encounter. In reality, we might use the thread index to mutate a molecular structure in a unique way, for example.
*/
  fake_x=spawn_simulthread_context_base->thread_idx;
  fake_x_max=0;
  fake_x=thread_idx;
  for(i=0;i<FAKE_DATA_SIZE;i++){
    fake_x=(0xFE001000ULL*(u32)(fake_x))+(fake_x>>U32_BITS)+fake_data_base[i];
    if(fake_x>fake_x_max){
      fake_x_max=fake_x;
    }
/*
Use our temporary space because the stack is too small. Yeah, it's only 1 variable, but in reality it might be a gigabyte.
*/
    simulthread_local_base->some_temporary_variable+=fake_data_base[i];
  }
/*
Pretend to read from our temporary space in order to adjust the calculated maximum X value. After this, we don't need the temporary space anymore, so it will be clobbered by some other thread as soon as we return.
*/
  fake_x_max|=simulthread_local_base->some_temporary_variable;
/*
Record the maximum value that we saw in the loop above. Maybe this corresponds to the maximum fuel efficiency of a particular type of engine which we just simulated. The root thread can then determine if it's the global max.
*/
  thread_local_base=&thread_global_base->thread_local_list_base[thread_idx];
  thread_local_base->fake_x_max=fake_x_max;
  return;
}

int
main(int argc, char *argv[]){
  u16 *fake_data_base;
  u64 fake_x_max_max;
  u32 i;
  u32 simulthread_idx_max;
  simulthread_local_t *simulthread_local_list_base;
  spawn_t *spawn_base;
  u8 status;
  ULONG thread_idx_max;
  thread_global_t thread_global;
  thread_local_t *thread_local_list_base;

  printf("Spawn build %d\nDemo code template\n\n",SPAWN_BUILD_ID);
/*
Let's do 1843 threads, because that's a nice number. So the maximum thread index is 1842.
*/
  thread_idx_max=1842;
/*
...and at most 320 simultaneous threads. Generally, the number of simultaneously executing threads (of which (simulthread_idx_max+1) is the maximum value) is at most equal to the number of threads. But this is not required.
*/
  simulthread_idx_max=319;
/*
Allocate some space for our fake readonly string.
*/
  fake_data_base=(u16 *)(spawn_malloc((FAKE_DATA_SIZE-1)<<U16_SIZE_LOG2));
  if(!fake_data_base){
    printf("No memory\n");
    exit(1);
  }
/*
Allocate Spawn data structures and perform minimal required initialization.
*/
  spawn_base=SPAWN_INIT(thread_execute,(u8 *)(&thread_global),simulthread_idx_max);
/*
Allocate per-thread storage.
*/
  thread_local_list_base=(thread_local_t *)(spawn_malloc(((thread_idx_max+1)*sizeof(thread_local_t))-1));
/*
Allocate per-simulthread storage.
*/
  simulthread_local_list_base=(simulthread_local_t *)(spawn_malloc(((simulthread_idx_max+1)*sizeof(simulthread_local_t))-1));
  if(!(spawn_base&&thread_local_list_base&&simulthread_local_list_base)){
    printf("No memory\n");
    exit(1);
  }
/*
Save pointers to the structures above inside the global data structure, so that threads can find their spaces.
*/
  thread_global.fake_data_base=fake_data_base;
  thread_global.thread_local_list_base=thread_local_list_base;
  thread_global.simulthread_local_list_base=simulthread_local_list_base;
/*
Initialize the readonly data string. In reality, this might be a table of atomic weights, or a table of mathematical constants.
*/
  for(i=0;i<=(FAKE_DATA_SIZE-1);i++){
    fake_data_base[i]=i*i*i;
  }
/*
Initialize the thread local max values to 0, even though the thread initializes it anyway, just to be paranoid.
*/
  for(i=0;i<=thread_idx_max;i++){
    thread_local_list_base[i].fake_x_max=0;
  }
/*
Launch (thread_idx_max+1) instances of thread_execute().
*/
  status=SPAWN(spawn_base,thread_idx_max);
/*
As many as (simulthread_idx_max+1) threads are still in flight. Be careful! No matter what happens, we need to call SPAWN_RETIRE_ALL() before exiting. Beware error paths that could cause you to fail to do this. In the meantime, we are not alloed to call any other Spawn code. So it's probably a good time to do housekeeping in the root thread, or better yet, kick of some network or harddrive IO that's going to happen in parallel and take a long time. But in this case, we have nothing better to do, so let's go ahead and flush all pending threads.
*/
  SPAWN_RETIRE_ALL(spawn_base);
  if(status){
    printf("SPAWN() returned bad status\n");
    exit(1);
  }
/*
Find the global max, by maximizing the local maxes computed in each thread. Note that it's only safe to do this because the threads are all retired, meaning that we can read their local storage safely.
*/
  fake_x_max_max=0;
  for(i=0;i<=thread_idx_max;i++){
    if(thread_local_list_base[i].fake_x_max>fake_x_max_max){
      fake_x_max_max=thread_local_list_base[i].fake_x_max;
    }
  }
/*
Display the global max, and comment whether it's right or wrong.
*/
  printf("spawn_fake_global_max=%08X%08X\n",(u32)(fake_x_max_max>>U32_BITS),(u32)(fake_x_max_max));
  if(fake_x_max_max==0xFDFFD009862C72FDULL){
    printf("^ Correct!\n");
  }else{
    printf("^ Wrong!\n");
  }
/*
Flush the printf's to the screen, just in case something goes wrong later.
*/
  fflush(stdout);
/*
We don't really need to rewind in this example, but this is useful in cases where the target function or readonly_string_base change between SPAWN() events, but you don't want to create memory map entropy by doing SPAWN_FREE() followed by another SPAWN_INIT(). So always do SPAWN_REWIND before doing SPAWN() or SPAWN_ONE() with a *spawn_base that was used with a previous SPAWN() or SPAWN_ONE(). (We don't need to do this before the first SPAWN() after a SPAWN_INIT().)
*/
  SPAWN_REWIND(thread_execute,(u8 *)(&thread_global),spawn_base);
/*
Do it all over again, but this time, using SPAWN_ONE() instead of SPAWN().
*/
  for(i=0;i<=thread_idx_max;i++){
    thread_local_list_base[i].fake_x_max=0;
  }
/*
Alias the functionality of SPAWN() using (thread_idx_max+1) invokations of SPAWN_ONE(). In reality, this would occur because don't know ahead of time how many threads we'll need, and just have to launch them opportunistically. Again, we're not allowed to call anything but SPAWN_ONE() until the next SPAWN_RETIRE_ALL(). As with SPAWN() above, beware error paths that could cause your code to forget to do this!
*/
  for(i=0;i<=thread_idx_max;i++){
    status=SPAWN_ONE(spawn_base,i);
    if(status){
      printf("SPAWN_ONE() returned bad status\n");
      SPAWN_RETIRE_ALL(spawn_base);
      exit(1);
    }
  }
/*
Flush all pending threads. We would usually be doing IO for a while before this occurs, in order to exploit parallelism between the CPU and peripheral devices.
*/
  SPAWN_RETIRE_ALL(spawn_base);
/*
Find the global max, as above.
*/
  fake_x_max_max=0;
  for(i=0;i<=thread_idx_max;i++){
    if(thread_local_list_base[i].fake_x_max>fake_x_max_max){
      fake_x_max_max=thread_local_list_base[i].fake_x_max;
    }
  }
/*
Display the global max, as above.
*/
  printf("spawn_one_fake_global_max=%08X%08X\n",(u32)(fake_x_max_max>>U32_BITS),(u32)(fake_x_max_max));
  if(fake_x_max_max==0xFDFFD009862C72FDULL){
    printf("^ Correct!\n");
  }else{
    printf("^ Wrong!\n");
  }
  fflush(stdout);
/*
Mandatory memory cleanup. We don't need to SPAWN_REWIND() because we're done with *spawn_base. But if we wanted to do another SPAWN(), or another or several other instances of SPAWN_ONE(), then SPAWN_REWIND() would be required if either (1) the target function changed to something other than thread_execute or (2) the readonly string base changed to something other than &thread_global. Changing the number of simulthreads requires SPAWN_INIT() because it's assumed that this value will change rarely, as its optimum is essentially a function of the hardware and the OS, and nothing else.
*/
  SPAWN_FREE(spawn_base);
  return 0;
}
