#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void consumer(int count) {
    // reads the value of the global variable 'n'
    // 'count' times.
    // print consumed value e.g. consumed : 8

    int32 i;
    for (i = 0; i <= count; i++){
        printf("consumed %d\n", n);
    }
}

void consumer_bb(int count) {
  // Iterate from 0 to count and for each iteration read the next available value from the global array `arr_q`
  // print consumer process name and read value as,
  // name : consumer_1, read : 8
  int waited = 0;
  int32 i;
  for (i = 0; i < count; i++){
      
      wait(mutex);
     
      // when the buffer is empty, singal other process, put the consumer process in waiting queue
      while (read_ind >= write_ind){
          signal(mutex);
          wait(mutex);
      }

      int tmp = arr_q[(read_ind++)%5];
      printf("name: %s, read: %d, in index: %d\n", proctab[getpid()].prname, tmp, (read_ind-1)%5);
    
      signal(mutex);
      

  }


}
