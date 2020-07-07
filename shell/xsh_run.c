#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <prodcons_bb.h>  
#include <futures_test.h>
#include <stream_proc.h>
#include <stream_proc_futures.h>

// definition of array, semaphores and indices
int arr_q[5];
int prod_sig, cons_sig, mutex;
int read_ind, write_ind;


int my_function_1 (int nargs, char *args[]);

int my_function_1 (int nargs, char *args[]){
    printf("testing, attention please!\n");
    return OK;
}

void prodcons_bb(int nargs, char *args[]) {
  //create and initialize semaphores to necessary values
  prod_sig = semcreate(0);
  cons_sig = semcreate(1);
  mutex = semcreate(1);


  //initialize read and write indices for the queue
  read_ind = 0;
  write_ind = 0;
  
  /* temp code */
  int count = 200;

  char process_name[50];     // name buffer of process

  //create producer and consumer processes and put them in ready queue
  int numOfProd, numOfCons;    // [# of producer processes] [#  of consumer processes]
  int iterOfProd, iterOfCons;  // [iterations of producer process] [iterations of consumer process]
  int i, j;    // loop variables

  numOfProd = atoi(args[1]);
  numOfCons = atoi(args[2]);
  iterOfProd = atoi(args[3]);
  iterOfCons = atoi(args[4]);

  for (i=1; i<= numOfProd; i++){
    sprintf(process_name, "producer_%d", i);
    resume( create(producer_bb, 1024, 20, process_name, 1, iterOfProd));
  }

  for (j=1; j<= numOfCons; j++){
    sprintf(process_name, "consumer_%d", j);
    resume( create(consumer_bb, 1024, 20, process_name, 1, iterOfCons));
  }
  
  return (0);
}

shellcmd xsh_run(int nargs, char *args[]) {
    if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0)) {
      printf("my_function_1\n");
      printf("my_function_2\n");
      printf("prodcons_bb");
      return OK;
    }

    /* This will go past "run" and pass the function/process name and its
    * arguments.
    */
    args++;
    nargs--;

    if(strncmp(args[0], "my_function_1", 13) == 0) {
      /* create a process with the function as an entry point. */
      resume (create((void *)my_function_1, 4096, 20, "my_func_1", 2, nargs, args));
    }

    else if(strncmp(args[0], "prodcons_bb", 11) == 0){
      resume(create((void *)prodcons_bb, 4096, 20, "prodcons_bb", 2, nargs, args));
    }

    else if(strncmp(args[0], "futures_test", 12) == 0)
    {
      if (strncmp(args[1], "-pc", 3) == 0)
      {
        future_exclusive_test(nargs, args);
      }

      else if (strncmp(args[1], "-fq1", 4) == 0){
        future_queue_test1(nargs, args);
      }

      else if (strncmp(args[1], "-fq2", 4) == 0){
      future_queue_test2(nargs, args);
      }

       else if (strncmp(args[1], "-fq3", 4) == 0){
        future_queue_test3(nargs, args);
      }
      
      else if (strncmp(args[1], "-f", 2) == 0)
      {
         future_shared_test(nargs, args);
      }
      

      // make sure all the process finishing running before returing to terminal
      sleepms(100);
      return(OK);
    }

    else if (strncmp(args[0], "tscdf_fq", 8) == 0){
      resume(create(stream_proc_futures, 1024, 20, "stream_proc_futures", 2, nargs, args));
    }

    else if (strncmp(args[0], "tscdf", 5) == 0){
      resume(create(stream_proc, 1024, 20, "stream_proc",2, nargs, args));
    }

    else{
      printf("%s: can't run an undefined function\n", args[0]);
      return 1;
    }
    
    
    return 0;
}