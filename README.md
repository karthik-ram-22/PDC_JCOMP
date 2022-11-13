# PDC_JCOMP_ pARLLELZING AES(128 AND 256) USING OPEN MP IN LINUX ENVIRONMENT

TO RUN PARALLELL CODE
1. g++ -fopenmp  aes.cpp  -o -r
2. ./-r
3. enter the message file as 'msg.txt' which is there in the parallel folder
4. Enter keyfile as "Keyfile"
5. Enter the name of output file as o1.txt (or anyother.txt) to make the encrypted text to get stored in that file.


TO RUN SERIAL CODE:
1. g++ encrypt.cpp -o r
2. ./r
3. enter the any message that should be 128 bit

In aes-128 the keyfile is 128 bit and in aes 192 the keyfile is 192 bits
