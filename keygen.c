/************************************************************
*
* keygen.c
*
* -----------------
# EvVikare
# evvikare@protonmail.com
* CS344 -- Summer 2017
* Program 4 Assignment
* -----------------
*
* A utility for generating a one-time pad 
*
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void usage(){
    fprintf(stderr, "usage: ./keygen {len}\nlength is a positive integer\n");
	exit(1);
}

/* ---- kgen

  Generates random string to serve as key for one-time pad
 
  rtn:  none
  prm:  keylen - the length of the key to be produced
  pre:  keylen is a positive integer
  pst:  random keystring with a newline is sent to stdout

*/
void kgen(int keylen){
    char buf[keylen + 2];
    int k, rchoice;

    for(k = 0; k < keylen; k++){
        rchoice = rand() % 27;
        buf[k] = (char) rchoice ? rchoice + 64 : 32;
    }

    buf[keylen] = '\n', buf[keylen + 1] = '\0';
    fprintf(stdout, "%s", buf);
}

int main(int argc, char* argv[]){
    srand(time(0));
    (argc > 1 && atoi(argv[1]) > 0) ? kgen( atoi(argv[1]) ) : usage();
    return 0;
}
