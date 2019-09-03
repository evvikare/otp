/************************************************************
*
* evserv.h
*
* -----------------
# EvVikare
# evvikare@protonmail.com
* CS344 -- Summer 2017
* Program 4 Assignment
* -----------------
*
* A library for OTP-ENC/DEC client and server applications 
*
*************************************************************/

#ifndef EVSERV_H
#define EVSERV_H

#define MAXCONNS 5
#define VERIFYSIZE 32
#define EOCNAME 7

typedef struct addrinfo Addr;
typedef struct sockaddr SAddr;
typedef struct sockaddr_storage Rhost;

// Common Functions
int xfer_all(int skt, char* buf, int expected, int flags, char* direction);
void err(char* msg, int ex);

// Server Functions
void setup_serv(int* listfd, char* port);
Addr* bind_serv_port(Addr* h, char* port, int* sock);
void init_fd_sets(fd_set* master, fd_set* read_fds, int* fdmax, int* listfd);
void event_loop(char** args);
void new_conn(fd_set* mstr, int* max, int lis);
void req(fd_set* mstr, int* max, int fd, int lis, char d_e, int is_new);
void get_payload_size(int* sz, char* in);
int verify(int fd, int* sz, char d_e);
void xcrypt(int skt, int sz, char d_e);

// Client Functions
void setup_client(int* skt, char* port, char d_e);
Addr* call_serv(Addr* h, char* port, int* sock);
void make_req(char** args);
char* get_from_file(char* fname);
void check_key_length(char* p, char* k, char* kname);
void get_verified(int fd, char* nm, int sz, char* port, char d_e);
void get_xcrypted(int skt, char* ptxt, char* ktxt);

#endif
