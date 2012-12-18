/*
  Weak, a chess perft calculator derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Weak is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Weak is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <pthread.h>

#include "weak.h"

bool
CreateThread(void *(*thread)(void*), void *arg)
{
  pthread_attr_t attr;
  pthread_t      posixThreadId;

  if(pthread_attr_init(&attr)) {
    return false;
  }

  if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
    return false;
  }

  return !!pthread_create(&posixThreadId, &attr, thread, arg);
}
