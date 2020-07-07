#ifndef _FUTURE_H_
#define _FUTURE_H_

typedef enum {
  FUTURE_EMPTY,
  FUTURE_WAITING,
  FUTURE_READY
} future_state_t;

typedef enum {
  FUTURE_EXCLUSIVE,
  FUTURE_SHARED,
  FUTURE_QUEUE
} future_mode_t;

typedef struct my_queue {
  uint16 capacity;
  uint16 length;
  uint16 read_top;
  uint16 write_top;
  pid32* the_queue;
} my_queue;

typedef struct future_t {
  char *data;
  uint32 size;
  future_state_t state;
  future_mode_t mode;
  pid32 pid;
  my_queue* set_queue;
  my_queue* get_queue;

  // new fields for interal data array
  uint16 max_elems;
  uint16 count;
  uint16 head;
  uint16 tail;

  uint16 num_consumed;
} future_t;



/* Functions for FIFO operation of my_queue */
struct my_queue* init_my_queue(uint capacity);
void my_enqueue(struct my_queue* queue, pid32 item);
pid32 my_dequeue(struct my_queue* queue);

/* Interface for the Futures system calls */
future_t* future_alloc(future_mode_t mode, uint size, uint nelems);
syscall future_free(future_t*);
syscall future_get(future_t*, char*);
syscall future_set(future_t*, char*);

#endif /* _FUTURE_H_ */