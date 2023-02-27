# Danger log
The danger log will be delivered based on the implementation categories as given below:
## General:
* For the recvied data from the socket recv() functions, the standard send() and recv() will have a char * as a pass-in argument, if we use a string to store the data, and do a str.c_str() to conver to a const char * and send, it will encounter problem. As the c_str() will only generate the contents in stack, and we passed in the the pointer to the function, it will lose the control of the content, resulting in the recv or send fail. Instead, the contents are received and stored as a form of **vector<char>** as this way it can be resized and accessed even convert to string in a convienient way.
## Request:
* For the request received, the Host Line will provide the target server's host name and port number, however, some request will not explicitly specify the port number, and they will treat the port number 80 as default for HTTP proxy. Thus the judgement should be given on wheter the port number is specified or not.
* The 
## Response:
## Proxy: