
#ifndef _SEGMENT_DEBUG_H_
#define _SEGMENT_DEBUG_H_


extern int system_debug_step;

#define SYSTEM_DEBUG_STEP(x)	do {system_debug_step=x;} while(0)

void sys_signal_setup(void);


#endif
