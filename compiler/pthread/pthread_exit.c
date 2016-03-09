/*
  Copyright (C) 2014 Szilard Biro

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "pthread_intern.h"
#include "debug.h"

void pthread_exit(void *value_ptr)
{
    pthread_t thread;
    ThreadInfo *inf;
    CleanupHandler *handler;

    D(bug("%s(%p)\n", __FUNCTION__, value_ptr));

    thread = pthread_self();
    inf = GetThreadInfo(thread);
    inf->ret = value_ptr;

    // execute the clean-up handlers
    while ((handler = (CleanupHandler *)RemTail((struct List *)&inf->cleanup)))
        if (handler->routine)
            handler->routine(handler->arg);

    longjmp(inf->jmp, 1);
}