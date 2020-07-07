#include <xinu.h>
#include <stream_proc.h>
#include "tscdf.h"

uint pcport;
int num_streams, work_queue_depth, time_window, output_time;

int stream_proc(int nargs, char* args[]) {
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

  // Create streams
  struct stream* str;
  str = (struct stream*)getmem(sizeof(struct stream) * num_streams);
  str->mutex = semcreate(1);

  // Create consumer processes and initialize streams
  // Use `i` as the stream id.
  char name_buffer[50];
  for (i = 0; i < num_streams; i++) {
      str[i].spaces = semcreate(work_queue_depth);
      str[i].items = semcreate(0);  
      str[i].count = 0;
      str[i].head = 0;
      str[i].tail = 0;
      str[i].queue = (de*)(getmem(sizeof(de) * work_queue_depth));

      pid32 con_process = create(stream_consumer, 1024, 20, "stream_consumer", 2, i, str);
      resume(con_process);
      sprintf(name_buffer, "stream_consumer id:%d (pid:%d)", i, con_process);
      kprintf("%s\n", name_buffer);
  }

  // Parse input header file data and populate work queue
  char* a;
  int con_id,ts,v;
  for (i = 0; i < n_input; i++){
    a = (char *)stream_input[i];
    con_id = atoi(a);
    while (*a++ != '\t');
    ts = atoi(a);
    while (*a++ != '\t');
    v = atoi(a);

    wait(str[con_id].spaces);
    wait(str->mutex);

    str[con_id].queue[str[con_id].tail].time = ts;
    str[con_id].queue[str[con_id].tail].value = v;
    if (++str[con_id].tail >= work_queue_depth){
        str[con_id].tail = 0;
    }
    signal(str->mutex);
    signal(str[con_id].items);
    
  }

  for(i=0; i < num_streams; i++) {
      uint32 pm;
      pm = ptrecv(pcport);
      wait(str->mutex);
      kprintf("process %d exited\n", pm);
      signal(str->mutex);
  }

  ptdelete(pcport, 0);

  time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
  kprintf("time in ms: %u\n", time);

  for (i = 0; i < num_streams; i++){
    freemem(str[i].queue, sizeof(de) * work_queue_depth);
  }
  freemem(str, sizeof(struct stream) * num_streams);
  return 0;
  }
}

void stream_consumer(int32 id, struct stream *str){
    int i;
    struct tscdf* p_tscdf = tscdf_init(time_window);
    char output[50];

    for (i = 0; i < n_input; i++)
    {   
      
        wait(str[id].items);
        wait(str->mutex);
        int ts = str[id].queue[str[id].head].time;
        int v  = str[id].queue[str[id].head].value;

        if (++str[id].head >= work_queue_depth){
            str[id].head = 0;
        }

        if (ts == 0){
            printf("stream_consumer exiting\n");
            ptsend(pcport, getpid()); 
            signal(str->mutex);
            signal(str[id].spaces);
            return;
        }

        tscdf_update(p_tscdf, ts, v);
        str[id].count++;

        if (str[id].count == output_time)
        {
          int32* qarray = tscdf_quartiles(p_tscdf);

          if(qarray == NULL) {
            kprintf("tscdf_quartiles returned NULL\n");
            continue;
          }
          sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
          kprintf("%s\n", output);
          freemem((char *)qarray, (6*sizeof(int32)));
          str[id].count = 0;
        }

        signal(str->mutex);
        signal(str[id].spaces);
       
    }
}
