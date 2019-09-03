/*******************************************************************************
*
* evserv.c
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
*******************************************************************************/

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "evserv.h"

/* ---- xfer_all

  Common: transfer_all -- ensure all data is sent or received
 
  rtn:  -1 on failure, 0 on success
  prm:  skt - remote host socket,
        buf - buffer containing payload,
        expected - payload size,
        flags - flags for send/recv calls
        direction - flag for determining whether to send or recv
  pre:  skt is valid socket,
        buf is size of expected
  pst:  none

*/
int xfer_all(int skt, char* buf, int expected, int flags, char* direction){
    int sent = 0, current, remaining = expected;
    
    while(sent < expected){
        if(strcmp(direction, "send") == 0){
            current = send(skt, buf + sent, remaining, flags);        
        }else{
            current = recv(skt, buf + sent, remaining, flags);        
        }
        if(current == -1){ break; }
        sent += current;
        remaining -= current;
    }
    
    return (current == -1) ? -1 : 0;
}

/* ---- err

  Common: general error message
 
  rtn:  none
  prm:  msg - error message,
        ex - exit status
  pre:  none
  pst:  none

*/
void err(char* msg, int ex){
    fprintf(stderr, "%s\n", msg);
	if(ex){ exit(ex); }
}

/* ---- setup_serv

  Server: establish server address information and listening socket
 
  rtn:  none
  prm:  listfd - address of listening socket,
        port - address of server port
  pre:  none
  pst:  fd is established as listening socket at port

*/
void setup_serv(int* listfd, char* port){
    Addr h;
    memset(&h, 0, sizeof(h));
    h.ai_family = AF_UNSPEC;
    h.ai_socktype = SOCK_STREAM;
    h.ai_flags = AI_PASSIVE;
    if(bind_serv_port(&h, port, listfd) == NULL){ err("Err: bind", 1); }
    if(listen(*listfd, MAXCONNS) < 0){ err("Err: listen", 1); }
}

/* ---- bind_serv_port

  Server: bind server process to a port
 
  rtn:  p - valid pointer to address information or NULL
  prm:  h - address info,
        port - server's port,
        sock - server's listening port
  pre:  none
  pst:  server is bound to given port

*/
Addr* bind_serv_port(Addr* h, char* port, int* sock){
    Addr* srv, *p;
    int on = 1;
    
    if (getaddrinfo(NULL, port, h, &srv) != 0){ err("Err: getaddrinfo", 1); }

    for(p = srv; p != NULL; p = p->ai_next){
        if((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            err("Err: socket", 0);
            continue;
        }
        
        if(setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0){
            err("Err: setsockopt", 1);
        }

        if(bind(*sock, p->ai_addr, p->ai_addrlen) < 0){
            close(*sock);
            err("Err: bind", 0);
            continue;
        }
        
        break;
    }
    
    free(srv);
    return p;
}

/* ---- init_fd_sets

  Server: initialize file descriptor sets
 
  rtn:  none
  prm:  master - master set of socket FDs,
        read_fds - read-in set of socket FDs,
        fdmax - address of highest FD,
        listfd - address of listener FD
  pre:  none
  pst:  FD max is set to listener FD,
        FD ste contains listener
*/
void init_fd_sets(fd_set* master, fd_set* read_fds, int* fdmax, int* listfd){
    FD_ZERO(master);
    FD_ZERO(read_fds);
    FD_SET(*listfd, master);
    *fdmax = *listfd;
}

/* ---- event_loop

  Server: event loop -- main function of otp_dec_d and otp_enc_d
 
  rtn:  none
  prm:  args - command line arguments
  pre:  none
  pst:  server listener socket is established

*/
void event_loop(char** args){
    int lis, max, i;
    fd_set mstr, fds;

    char d_e = (strstr(args[0], "enc") != NULL) ? 'e' : 'd';

    setup_serv(&lis, args[1]);
    init_fd_sets(&mstr, &fds, &max, &lis); 
    
    while(1){
        fds = mstr;
        if(select(max+1, &fds, NULL, NULL, NULL) < 0){ err("Err: select", 1); }

        for(i = 0; i <= max; i++){          
            if(FD_ISSET(i, &fds)){ req(&mstr, &max, i, lis, d_e, (i == lis)); }
        }
    }
}

/* ---- new_conn

  Server: accept new connection on a dedicated socket
 
  rtn:  none
  prm:  mster - master set of socket FDs,
        max - address of highest FD,
        lis - listener FD
  pre:  none
  pst:  FD is new max,
        FD is added to master set

*/
void new_conn(fd_set* mstr, int* max, int lis){
    Rhost rh;
    socklen_t rh_sz = sizeof(rh);
    int fd = accept(lis, (SAddr*) &rh, &rh_sz);
    if(fd < 0){ err("Err: accept", 0); return; }
    FD_SET(fd, mstr);
    if(fd > *max){ *max = fd; }
}

/* ---- req

  Server: request -- main function of otp_dec_d and otp_enc_d
 
  rtn:  none
  prm:  mstr - Master FD set,
        max - highest FD,
        fd - current FD,
        lis - listener FD,
        d_e - enc/dec flag,
        is_new - new connection flag
  pre:  fd is in mstr,
        fd is a valid socket,
        lis is a valid socket
  pst:  new connection is established and added to master set
        OR
        server receives data from valid connection, then closes it

*/
void req(fd_set* mstr, int* max, int fd, int lis, char d_e, int is_new){
    int sz;
    if(is_new){
        new_conn(mstr, max, lis);    
    }else{
        (verify(fd, &sz, d_e)) ? xcrypt(fd, sz, d_e) : err("Error: request", 0);
        close(fd);
        FD_CLR(fd, mstr);
    }
}

/* ---- get_payload_size

  Server: determine size of incoming payload
 
  rtn:  none
  prm:  sz - pointer to size of message payload,
        in - string containing payload's size in string form
  pre:  in contains valid size information
  pst:  sz is set to payload size

*/
void get_payload_size(int* sz, char* in){
    int pos;
    char payload_size[VERIFYSIZE];
    memset(payload_size, '\0', VERIFYSIZE);

    for(pos = EOCNAME; in[pos] && in[pos] != '\n'; pos++){
        memcpy(&payload_size[pos-EOCNAME], &in[pos], 1);
    }

    *sz = atoi(payload_size);
}

/* ---- verify

  Server: verify client and receive size of incoming payload
 
  rtn:  verified - flag indicating whether client follows protocol
  prm:  fd - fd number of client socket,
        sz - size of message payload,
        d_e - enc/dec flag
  pre:  fd is valid
  pst:  none

*/
int verify(int fd, int* sz, char d_e){
    int verified;
    char* rhost_id = (d_e == 'e') ? "enc" : "dec";
    char in_buf[VERIFYSIZE]; char out_buf[VERIFYSIZE];
    memset(in_buf, '\0', VERIFYSIZE); memset(out_buf, '\0', VERIFYSIZE);

    if(recv(fd, in_buf, VERIFYSIZE, 0) < 0){ err("Err: recv in verify", 0); }

    verified = (strstr(in_buf, rhost_id) != NULL);
    
    if(verified){
        get_payload_size(sz, in_buf);
        strncpy(out_buf, "ok", 2);
    }else{
        strncpy(out_buf, "no", 2);    
    }

    if(send(fd, out_buf, VERIFYSIZE, 0) < 0){ err("Err: send in verify", 0); }

    return verified; 
}

/* ---- xcrypt

  Server: encrypt/decrypt a message and send to client
 
  rtn:  none
  prm:  skt - client's socket,
        sz - message payload size,
        d_e - encrypt/decrypt flag 
  pre:  skt is valid connection to respective client,
        mtxt and ktxt are same length
  pst:  none

*/
void xcrypt(int skt, int sz, char d_e){
    int success = 0, md1, md2, sum, k;
    char key[sz]; char txt[sz];
    memset(key, '\0', sz); memset(txt, '\0', sz);

    success += xfer_all(skt, txt, sz, 0, "recv");   // Receive plain/cipher text
    success += xfer_all(skt, key, sz, 0, "recv");   // Receive key text
    
    for(k = 0; k < sz; k++){
        md1 = (txt[k] == 32) ? 0 : txt[k] - 64;         // Convert to 0-27 value
        md2 = (key[k] == 32) ? 0 : key[k] - 64;         // Convert to 0-27 value
        sum = (d_e == 'e') ? (md1 + md2) : (md1 - md2); // Encrypt/decrypt
        (sum < 0) ? (sum += 27) : (sum %= 27);          // "Wrap-around" effect
        txt[k] = (char) sum ? sum + 64 : 32;            // Convert to ASCII
    }   

    success += xfer_all(skt, txt, sz, 0, "send");   // Send plain/cipher text
    if(success < 0){ err("Err: xcrypt", 0); }
}

/* ---- setup_client

  Client: establish client address information and socket
 
  rtn:  none
  prm:  skt - client socket,
        port - server port,
        d_e - enc/dec flag (for error message)
  pre:  none
  pst:  none

*/
void setup_client(int* skt, char* port, char d_e){
    char* rhost = (d_e == 'e') ? "otp_enc_d" : "otp_dec_d";
    Addr h;
    memset(&h, 0, sizeof(h));
    h.ai_family = AF_UNSPEC;
    h.ai_socktype = SOCK_STREAM;
    if(call_serv(&h, port, skt) == NULL){
        fprintf(stderr, "Error: could not contact %s on port %s", rhost, port);
        err("", 2);
    }
}

/* ---- call_serv

  Client: connect with respective daemon
 
  rtn:  p - valid pointer to address information or NULL
  prm:  h - address info of remote host,
        port - server's port,
        sock - client's socket
  pre:  none
  pst:  none

*/
Addr* call_serv(Addr* h, char* port, int* sock){
    Addr* srv, *p;
    
    if (getaddrinfo(NULL, port, h, &srv) != 0){ p = NULL; }

    for(p = srv; p != NULL; p = p->ai_next){
        if((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            continue;
        }

        if(connect(*sock, p->ai_addr, p->ai_addrlen) < 0){
            close(*sock);
            continue;
        }
        
        break;
    }
    
    free(srv);
    return p;
}

/* ---- make_req

  Client: make request -- main function of otp_dec and otp_enc
 
  rtn:  none
  prm:  args - command line arguments
  pre:  none
  pst:  none

*/
void make_req(char** args){
    int skt;
    char d_e = (strstr(args[0], "enc") != NULL) ? 'e' : 'd' ; // Which client?
    char* mtxt = get_from_file(args[1]);    // Message text (plain/cipher)
    char* ktxt = get_from_file(args[2]);    // Key text
    check_key_length(mtxt, ktxt, args[2]);  // Match string lengths

    setup_client(&skt, args[3], d_e);
    get_verified(skt, args[0], strlen(mtxt) + 1, args[3], d_e);
    get_xcrypted(skt, mtxt, ktxt);
    free(mtxt), free(ktxt);
}

/* ---- get_from_file

  Client: get message or key text from file
 
  rtn:  f_str - pointer to valid text string
  prm:  fname - filename containing message or key text
  pre:  fname is a nonempty text file in the current directory
  pst:  none

*/
char* get_from_file(char* fname){
    int ch, idx, capacity = 128;
    char* f_str = calloc(capacity, sizeof(char));
    FILE* f;
    if((f = fopen(fname, "r")) < 0){ err("Error: opening file", 1); } 
       
    for(idx = 0; (ch = getc(f)) != '\n'; idx++){
        // Double capacity as needed
        if(idx == capacity){
            capacity *= 2;
            f_str = realloc(f_str, capacity);
            if(f_str == NULL){ err("Error: realloc", 1); }             
        }
        
        // Assign valid character to current index
        if(ch == 32 || (ch > 64 && ch < 91)){ 
            f_str[idx] = ch;
            continue;
        }
        
        // Anything other than a space or a capital A-Z results in termination
        free(f_str); 
        err("otp_enc error: input contains bad characters", 1);
        break;
    }

    f_str[idx] = '\0';
    fclose(f);

    return f_str;
}

/* ---- check_key_length

  Client: ensure key length is valid and matches message text
 
  rtn:  none
  prm:  m - message text,
        k - key text,
        kname - key filename 
  pre:  m and k are valid strings
  pst:  mtxt and ktxt are same length

*/
void check_key_length(char* m, char* k, char* kname){
    if(strlen(k) < strlen(m)){
        fprintf(stderr, "Error: key ‘%s’ is too short", kname);
        err("", 1);
    }else{
        k[strlen(m)] = '\0';
    }
}

/* ---- get_verified

  Client: verify with server and announce size of payload
 
  rtn:  none
  prm:  fd - fd number of socket,
        nm - name of client,
        sz - size of message payload,
        port - port string (for error message),
        d_e - enc/dec flag (for error message)
  pre:  fd is valid,
        sz is nonzero
  pst:  none

*/
void get_verified(int fd, char* nm, int sz, char* port, char d_e){
    char* rhost = (d_e == 'e') ? "otp_enc_d" : "otp_dec_d";
    char in_buf[VERIFYSIZE], out_buf[VERIFYSIZE];
    memset(in_buf, '\0', VERIFYSIZE); memset(out_buf, '\0', VERIFYSIZE);
    sprintf(out_buf, "%s%d", nm, sz);

    if(send(fd, out_buf, VERIFYSIZE, 0) < 0){ err("Err: send in verify", 1); }
    if(recv(fd, in_buf, VERIFYSIZE, 0) < 0){ err("Err: recv in verify", 1); }
    if(strncmp(in_buf, "ok", 2) != 0){
        fprintf(stderr, "Error: could not contact %s on port %s", rhost, port);
        err("", 2);
    }
}

/* ---- get_xcrypted

  Client: get encrypt/decrypted message and print to stdout
 
  rtn:  none
  prm:  skt - client's socket,
        mtxt - message text,
        ktxt - key text,
        d_e - encrypt/decrypt flag 
  pre:  skt is valid connection to respective daemon,
        mtxt and ktxt are same length
  pst:  none

*/
void get_xcrypted(int skt, char* mtxt, char* ktxt){
    int success = 0;
    int sz = strlen(mtxt) + 1;
    char rtxt[sz]; memset(rtxt, '\0', sz);

    success += xfer_all(skt, mtxt, sz, 0, "send");  // Send message text
    success += xfer_all(skt, ktxt, sz, 0, "send");  // Send key text
    success += xfer_all(skt, rtxt, sz, 0, "recv");  // Receive results
    
    if(success == 0){
        rtxt[sz] = '\0'; rtxt[sz - 1] = '\n';
        fprintf(stdout, "%s", rtxt);
    }else{
        err("Err: get_xcrypted", 0); 
    }
}//EOF
