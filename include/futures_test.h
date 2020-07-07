uint future_prod(future_t*, char*);
uint future_cons(future_t*);

future_t** fibfut;
int ffib(int);

void future_exclusive_test(int nargs, char *args[]);
void future_shared_test(int nargs, char *args[]);
void future_queue_test (int nargs, char *args[]);
void waitFor(unsigned int);