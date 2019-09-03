# OTP

OTP is a small suite of C programs that handles one-time pad key generation, encryption, and decryption of text.
This was an assignment completed for CS344 Operating Systems at OSU.

## Usage


### Compiling and Running
To compile, type the follow command at a Linux terminal:

	./compileall

First, generate a key:

    keygen keylength > mykey
    
keylength is the length of the key file in characters.

Next, initiate the daemon-like utilities:

    otp_dec_d listening_port
    otp_enc_d listening_port

listening_port is any port above 1023. For best results, use five-digit ports.

Make requests to the encryption daemon using any of the below invocations:

    otp_enc myplaintext mykey port
    otp_enc myplaintext mykey port > myciphertext
    otp_enc myplaintext mykey port > myciphertext &

Similarly, request decryption using any of the below:

    otp_dec myciphertext mykey port
    otp_dec myciphertext mykey port > myplaintext
    otp_dec myciphertext mykey port > myplaintext &

mykey is the valid OTP key provided by keygen.

port is the port number that the corresponding daemon is currently listening on.

To compare the implementation of OTP against the provided grading script:

    ./p4gradingscript PORT1 PORT2 > otp_testresults 2>&1

PORT1 and PORT2 can be any valid port for use by the otp\_enc and otp\_dec daemons.

## Notes

OTP does **not** generate cryptographically safe keys; it is not meant for sensitive data.

OTP is designed to use only uppercase ASCII letters from A-Z.

Each OTP daemon can handle up to five concurrent connections.




