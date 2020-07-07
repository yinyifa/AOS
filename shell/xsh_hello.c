/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_hello - print greeting message
 *------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char *args[]) {

	/* Check argument count */

	if (nargs > 2) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		return 1;
	}

	if (nargs < 2){
	fprintf(stderr, "%s: too less arguments\n", args[0]);
		return 1;
	}

	if (nargs == 2) {
		printf("Hello %s, Welcome to the world of Xinu!!\n",args[1]);
	}

	return 0;
}
