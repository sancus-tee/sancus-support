#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

typedef void (*idle_callback)(int);
typedef void (*tick_callback)(void);

void event_loop_start(tick_callback tick);

#endif

