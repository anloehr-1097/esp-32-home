#ifndef TIME_SYNC_TASK_H
#define TIME_SYNC_TASK_H
void time_sync_callback(struct timeval* tv);
void init_sntp(void);
#endif  // TIME_SYNC_TASK_H
