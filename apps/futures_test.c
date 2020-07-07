#include <xinu.h>
#include <futures_test.h>

uint future_prod(future_t* fut, char* value){
  int* nptr = (int*) value;
  future_set(fut, value);
  kprintf("Produced %d\n", *nptr);
  return OK;
}

uint future_cons(future_t* fut) {
  int i, status;
  status = (int) future_get(fut, (char*) &i);
  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }
  kprintf("Consumed %d\n", i);
  return OK;
}

int ffib(int n) {
  int zero = 0, one = 1;
  int minus1 = 0;
  int minus2 = 0;
  int this = 0;

  if (n == 0) {
    future_set(fibfut[0], (char*) &zero);
    return OK;
  }

  if (n == 1) {
    future_set(fibfut[1], (char*) &one);
    return OK;
  }

  int status = (int) future_get(fibfut[n-2], (char*) &minus2);

  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }

  status = (int) future_get(fibfut[n-1], (char*) &minus1);

  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }

  this = minus1 + minus2;

  future_set(fibfut[n], (char*) &this);

  return(0);
}

void future_exclusive_test(int nargs, char *args[]){
        int one = 1, two =2;
        future_t* f_exclusive, * f_shared;
        f_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
        f_shared    = future_alloc(FUTURE_SHARED, sizeof(int), 1);

        // Test FUTURE_EXCLUSIVE
        resume(create(future_cons, 1024, 20, "fcons1", 1, f_exclusive));
        resume(create(future_prod, 1024, 20, "fprod1", 2, f_exclusive, (char*) &one) );

        // Test FUTURE_SHARED
        resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
        resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
        resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) );
        resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
        resume( create(future_prod, 1024, 20, "fprod2", 2, f_shared, (char*) &two));
}

void future_shared_test(int nargs, char *args[]){
        int fib = -1, i;
        fib = atoi(args[2]);

        if (fib > -1) 
        { 
          int final_fib;
          int future_flags = FUTURE_SHARED; // TODO - add appropriate future mode here
          fibfut = 0;

          // create the array of future pointers
          if ((fibfut = (future_t **)getmem(sizeof(future_t *) * (fib + 1)))
              == (future_t **) SYSERR) {
              printf("getmem failed\n");
              return;
              }

          // get futures for the future array
          for (i=0; i <= fib; i++) {
            if((fibfut[i] = future_alloc(future_flags, sizeof(int), 1)) == (future_t *) SYSERR) {
              printf("future_alloc failed\n");
              return;
            }
          }

          // spawn fib threads and get final value
          // TODO - you need to add your code here
          for (i=0; i<=fib; i++){
            resume(create(ffib, 1024, 20, "ffib", 1, i));
          }
          future_get(fibfut[fib], (char*) &final_fib);
          

          for (i=0; i <= fib; i++) {
            future_free(fibfut[i]);
          }

          freemem((char *)fibfut, sizeof(future_t *) * (fib + 1));
          printf("Nth Fibonacci value for N=%d is %d\n", fib, final_fib);
        } 
}

void future_queue_test1 (int nargs, char *args[]) {
    int three = 3, four = 4, five = 5, six = 6;
    future_t* f_queue;

    f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

    // Test FUTURE_QUEUE Part1 
    resume(create(future_cons, 1024, 20, "fcons6", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons7", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons8", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons9", 1, f_queue));
    resume(create(future_prod, 1024, 20, "fprod3", 2, f_queue, (char *)&three));
    resume(create(future_prod, 1024, 20, "fprod4", 2, f_queue, (char *)&four));
    resume(create(future_prod, 1024, 20, "fprod5", 2, f_queue, (char *)&five));
    resume(create(future_prod, 1024, 20, "fprod6", 2, f_queue, (char *)&six));
    waitFor(100);
}

void future_queue_test2 (int nargs, char *args[]) {
    int seven = 7, eight = 8, nine=9, ten = 10, eleven = 11;
    future_t* f_queue;
    f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);
    // Test FUTURE_QUEUE Part2 
    resume(create(future_prod, 1024, 20, "fprod10", 2, f_queue, (char *)&seven));
    resume(create(future_prod, 1024, 20, "fprod11", 2, f_queue, (char *)&eight));
    resume(create(future_prod, 1024, 20, "fprod12", 2, f_queue, (char *)&nine));
    resume(create(future_prod, 1024, 20, "fprod13", 2, f_queue, (char *)&ten));
    resume(create(future_prod, 1024, 20, "fprod13", 2, f_queue, (char *)&eleven));

    resume(create(future_cons, 1024, 20, "fcons14", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons15", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons16", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons17", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons18", 1, f_queue));
     waitFor(100);
    
  }

void future_queue_test3 (int nargs, char *args[]){
    int three = 3, four = 4, five = 5, six = 6;
    future_t* f_queue;
    f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

    // Test FUTURE_QUEUE Part3 
    resume( create(future_cons, 1024, 20, "fcons6", 1, f_queue) );
    resume( create(future_prod, 1024, 20, "fprod3", 2, f_queue, (char*) &three) );
    resume( create(future_prod, 1024, 20, "fprod4", 2, f_queue, (char*) &four) );
    resume( create(future_prod, 1024, 20, "fprod5", 2, f_queue, (char*) &five) );
    resume( create(future_prod, 1024, 20, "fprod6", 2, f_queue, (char*) &six) );
    resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons8", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons9", 1, f_queue) );
     waitFor(100);
}

void waitFor (unsigned int wait_msecs) {
    ulong secs, msecs;
    secs = clktime;
    msecs = clkticks;
    while (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs) < wait_msecs);
   }