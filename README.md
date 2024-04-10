# Computer-Networks---Advanced-Meeting-Scheduler-System-
Name: Ashish Gautam
USCID: 6919134678

What have I done in the assignment ? 
    Implemented main, backend servers and client
    Extra credit is not done.

What the code files are and what each one of them does.
    1. serverM.cpp -> Entry to Main server
    2. serverA.cpp -> calls backend.cpp 
    3. serverB.cpp -> calls backend.cpp
    4. backend.cpp -> Generic backend server
    5. client.cpp  -> Entry to Client
    6. common.h    
    7. common.cpp  -> Helper functions, classes

The format of all the messages exchanged:
    Messages like usernames and timelist are stored inside a char buffer (buf)
    
    For usernames, the first 8 bytes (size) tells the number of usernames.
    Then next 25 bytes tells us about the first username and so on.
    buf[0] = number of usernames
    buf[8] = first username
    buf[33] = second username

    For timelist, the first 8 bytes tells us the number of timestamps.
    The next 4 bytes tells about the start of first timestamp and the next 
    4 bytes tells about the end of the first timestamp. And so on ...

Help taken from Beej's guide and stack overflow:
https://stackoverflow.com/questions/780041/confusion-about-udp-ip-and-sendto-recvfrom-return-values
https://stackoverflow.com/questions/24045424/when-acting-on-a-udp-socket-what-can-cause-sendto-to-send-fewer-bytes-than-re
https://stackoverflow.com/a/2662405
https://stackoverflow.com/a/28289678
https://stackoverflow.com/questions/13292125/a-smart-way-to-send-unknown-length-string-in-a-struct-over-tcp-c-linux
https://stackoverflow.com/a/74089089
https://stackoverflow.com/a/2862176
https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
https://stackoverflow.com/a/4047837




Idiosyncrasy: The input files name is hard coded as "a.txt" and "b.txt". However my project should work with any input file. Or renaming the input file by TA may be required. 


