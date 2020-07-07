#ifndef _STREAM_PROC_H_
#define _STREAM_PROC_H_

typedef struct data_element {
  int32 time;
  int32 value;
} de;

struct stream {
  sid32 spaces;
  sid32 items;
  sid32 mutex;
  int32 count;
  int32 head;
  int32 tail;
  struct data_element *queue;
};

int stream_proc(int nargs, char* args[]);
void stream_consumer(int32 id, struct stream *str);

#endif