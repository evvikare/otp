/************************************************************
*
* otp_enc_d.c
*
* -----------------
# EvVikare
# evvikare@protonmail.com
* CS344 -- Summer 2017
* Program 4 Assignment
* -----------------
*
* A daemon-like server for encoding messages using a one-time pad 
*
*************************************************************/

#include <stdlib.h>
#include "evserv.h"

void usage(){
    err("usage: ./otp_enc_d {listen}\n\tlisten is a valid TCP port", 1);
}

int main(int argc, char* argv[]){
    (argc > 1 && atoi(argv[1]) > 1023) ? event_loop(argv) : usage();
    return 0;
}
