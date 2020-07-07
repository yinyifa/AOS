#include <xinu.h>
#include <stream_proc_futures.h>
#include "tscdf.h"

uint pcport;
int num_streams, work_queue_depth, time_window, output_time;

void stream_consumer_future(int32 id, future_t *f){
    int i;
    struct tscdf* p_tscdf = tscdf_init(time_window);
    char output[50];

    for (i = 0; i < n_input; i++)
    {   
        de cons_data;
        future_get(f, &cons_data);

        if (cons_data.time == 0){
            ptsend(pcport, getpid()); 
            return;
        }

        tscdf_update(p_tscdf, cons_data.time, cons_data.value);
        f->num_consumed++;

        if (f->num_consumed == output_time)
        {
          int32* qarray = tscdf_quartiles(p_tscdf);

          if(qarray == NULL) {
            kprintf("tscdf_quartiles returned NULL\n");
            continue;
          }
          sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
          kprintf("%s\n", output);
          freemem((char *)qarray, (6*sizeof(int32)));
          f->num_consumed = 0;
        }
       
    }
}

int stream_proc_futures(int nargs, char* args[]) {
  // Parse arguments 
  int i;
  char* ch;
  char c;
  
  num_streams = 10;
  work_queue_depth = 10;

  ulong secs, msecs, time;
  secs = clktime;
  msecs = clkticks;
  
  if((pcport = ptcreate(num_streams)) == SYSERR) {
      printf("ptcreate failed\n");
      return(-1);
  }

  // Parse arguments
  char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";

  /* Parse arguments out of flags */
  /* if not even # args, print error and exit */
  if (!(nargs % 2)) {
    printf("%s", usage);
    return(-1);
  }
  else {
    i = nargs - 1;
    while (i > 0) {
      ch = args[i-1];
      c = *(++ch);
      
      switch(c) {
      case 's':
        num_streams = atoi(args[i]);
        break;

      case 'w':
        work_queue_depth = atoi(args[i]);
        break;

      case 't':
        time_window = atoi(args[i]);
        break;
        
      case 'o':
        output_time = atoi(args[i]);
        break;

      default:
        printf("%s", usage);
        return(-1);
      }

      i -= 2;
    }

    // Create array to hold `n_streams` number of futures
    future_t **farray;
    if ((farray = (future_t **)getmem(sizeof(future_t *) * num_streams))
                == (future_t **) SYSERR) {
        printf("getmem failed\n");
        return -1;
        }

  // Create consumer processes and allocate futures
  // Use `i` as the stream id.
  // Future mode = FUTURE_QUEUE
  // Size of element = sizeof(struct data_element)
  // Number of elements = work_queue_depth
  char name_buffer[50];
  for (i = 0; i < num_streams; i++) {
      if((farray[i] = future_alloc(FUTURE_QUEUE, sizeof(de), work_queue_depth)) == (future_t *) SYSERR) {
              printf("future_alloc failed\n");
              return -1;
            }
      pid32 con_process = create(stream_consumer_future, 1024, 20, "stream_consumer_future", 2, i, farray[i]);
      resume(con_process);
      sprintf(name_buffer, "stream_consumer_future id:%d (pid:%d)", i, con_process);
      kprintf("%s\n", name_buffer);
      
  }

  // Parse input header file data and set future values
   char* a;
  int con_id,ts,v;
  for (i = 0; i < n_input; i++){
    a = (char *)stream_input[i];
    con_id = atoi(a);
    while (*a++ != '\t');
    ts = atoi(a);
    while (*a++ != '\t');
    v = atoi(a);
    de val_set = {ts, v};
    future_set(farray[con_id], (char*) &val_set);
  }

  // Wait for all consumers to exit
  for(i=0; i < num_streams; i++) {
      uint32 pm;
      pm = ptrecv(pcport);
      kprintf("process %d exited\n", pm);
  }

  ptdelete(pcport, 0);

  time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
  kprintf("time in ms: %u\n", time);

  // free futures array
  for (i = 0; i < num_streams; i++){
    future_free(farray[i]);
  }
  freemem(farray, sizeof(future_t*) * num_streams);
  return 0;
}
}