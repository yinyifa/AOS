// declare globally shared array
extern int arr_q[];

// declare globally shared semaphores
extern int prod_sig, cons_sig, mutex;

// declare globally shared read and write indices
extern int read_ind, write_ind;

// function prototypes
void consumer_bb(int count);
void producer_bb(int count);
void prodcons_bb(int nargs, char *args[]);