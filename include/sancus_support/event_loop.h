#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

typedef void (*idle_callback)(int);

void event_loop_start(idle_callback set_idle);

#endif

