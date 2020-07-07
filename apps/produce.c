#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>


void producer(int count) {
    // Iterates from 0 to count, setting
    // the value of the global variable 'n'
    // each time.
    //print produced value e.g. produced : 8

    int32 i;
    for (i = 0; i <= count; i++){
        n = i;
        printf("produced: %d\n", n);
        
    }
    
}

void producer_bb(int count) {
  // Iterate from 0 to count and for each iteration add iteration value to the global array `arr_q`, 
  // print producer process name and written value as,
  // name : producer_1, write : 8
  int waited = 0;
  int32 i;
  for (i = 0; i < count; i++){
     
     wait(mutex);

    // when the buffer is full, signal other process, then put the write process in waiting queue
     while ((write_ind - read_ind)>=5){
            signal(mutex);
            wait(mutex);
     }
      arr_q[(write_ind++)%5] = i;
      printf("name: %s, write: %d, in index: %d\n", proctab[getpid()].prname, i, (write_ind-1)%5);
    
      signal(mutex);

  }
}
