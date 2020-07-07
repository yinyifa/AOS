# Does your program output any garbage? If yes, why?

Yes. Because two processes are created and started, and the execution of code can swap between the two processes, both processes are racing to display the data, which outputs garbage.

# Are all the produced values getting consumed? 

No.

# Functions in the project

## produce.c

```
#include <xinu.h>
#include <prodcons.h>

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
```

## consume.c

```
#include <xinu.h>
#include <prodcons.h>

void consumer(int count) {
    // reads the value of the global variable 'n'
    // 'count' times.
    // print consumed value e.g. consumed : 8

    int32 i;
    for (i = 0; i <= count; i++){
        printf("consumed %d\n", n);
    }
}
```

## xsh_prodcons.c

```
#include <xinu.h>
#include <prodcons.h>
#include <stdio.h>
#include <stdlib.h>

int n;                 //Definition for global variable 'n'
/*Now global variable n will be on Heap so it is accessible all the processes i.e. consume and produce*/

shellcmd xsh_prodcons(int nargs, char *args[])
{
  //Argument verifications and validations
  int count = 2000;             //local varible to hold count
  
  //check args[1] if present assign value to count
  if (nargs > 2){
      fprintf(stderr, "%s: too many arguments\n", args[0]);
	  return 1;
  }


  if (nargs == 2){
      count = atoi(args[1]);
  }


  //create the process producer and consumer and put them in ready queue.
  //Look at the definations of function create and resume in the system folder for reference.      
  resume( create(producer, 1024, 20, "producer", 1, count));
  resume( create(consumer, 1024, 20, "consumer", 1, count));
  return (0);
}
```
