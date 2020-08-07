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
TYPEDEF_START
  u8 *readonly_string_base;
  ULONG thread_idx;
  u32 simulthread_idx;
TYPEDEF_END(spawn_simulthread_context_t)

TYPEDEF_START
  spawn_simulthread_context_t context;
#ifdef PTHREAD
  pthread_t pthread;
#endif
TYPEDEF_END(spawn_simulthread_t)

TYPEDEF_START
  void (*function_base)(spawn_simulthread_context_t *);
  spawn_simulthread_t *simulthread_list_base;
  u32 simulthread_idx_max;
  u32 simulthread_launch_idx;
  u32 simulthread_retire_idx;
  u8 simulthread_active_status;
TYPEDEF_END(spawn_t)

#ifdef PTHREAD
  #define SPAWN(spawn_base,thread_idx_max) spawn_multi(spawn_base,thread_idx_max)
  #define SPAWN_FREE(spawn_base) spawn_multi_free(spawn_base)
  #define SPAWN_INIT(function_base,readonly_string_base,simulthread_idx_max) spawn_multi_init(function_base,readonly_string_base,simulthread_idx_max)
  #define SPAWN_ONE(spawn_base,unique_idx) spawn_multi_one(spawn_base,unique_idx)
  #define SPAWN_RETIRE_ALL(spawn_base) spawn_multi_retire_all(spawn_base)
  #define SPAWN_REWIND(function_base,readonly_string_base,spawn_base) spawn_multi_rewind(function_base,readonly_string_base,spawn_base)
#else
  #define SPAWN(spawn_base,thread_idx_max) spawn_mono(spawn_base,thread_idx_max)
  #define SPAWN_FREE(spawn_base) spawn_mono_free(spawn_base)
  #define SPAWN_INIT(function_base,readonly_string_base,simulthread_idx_max) spawn_mono_init(function_base,readonly_string_base)
  #define SPAWN_ONE(spawn_base,unique_idx) spawn_mono_one(spawn_base,unique_idx)
  #define SPAWN_RETIRE_ALL(spawn_base)
  #define SPAWN_REWIND(function_base,readonly_string_base,spawn_base) spawn_mono_rewind(function_base,readonly_string_base,spawn_base)
#endif
