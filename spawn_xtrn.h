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
#ifdef PTHREAD
  extern u8 spawn_multi_one(spawn_t *spawn_base,ULONG unique_idx);
  extern u8 spawn_multi(spawn_t *spawn_base,ULONG thread_idx_max);
  extern void spawn_multi_retire_all(spawn_t *spawn_base);
  extern void spawn_multi_free(spawn_t *spawn_base);
  extern void spawn_multi_rewind(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base,spawn_t *spawn_base);
  extern spawn_t *spawn_multi_init(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base,u32 simulthread_idx_max);
#else
  extern u8 spawn_mono_one(spawn_t *spawn_base,ULONG unique_idx);
  extern u8 spawn_mono(spawn_t *spawn_base,ULONG thread_idx_max);
  extern void spawn_mono_free(spawn_t *spawn_base);
  extern void spawn_mono_rewind(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base,spawn_t *spawn_base);
  extern spawn_t *spawn_mono_init(void (*function_base)(spawn_simulthread_context_t *),u8 *readonly_string_base);
#endif
