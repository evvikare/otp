/************************************************************
*
* otp_enc.c
*
* -----------------
# EvVikare
# evvikare@protonmail.com
* CS344 -- Summer 2017
* Program 4 Assignment
* -----------------
*
* A client for requesting message encoding via a daemon 
*
*************************************************************/

#include <sys/select.h>
#include "evserv.h"

void usage(){
    err("usage: ./otp_enc {plaintext} {keyfile} {port}\n", 1);
}

int main(int argc, char* argv[]){
    (argc > 3) ? make_req(argv) : usage();
    return 0;
}
