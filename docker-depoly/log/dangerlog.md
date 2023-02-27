# Danger log
The danger log will be delivered based on the implementation categories as given below:
## General:
* For the recvied data from the socket recv() functions, the standard send() and recv() will have a char * as a pass-in argument, if we use a string to store the data, and do a str.c_str() to conver to a const char * and send, it will encounter problem. As the c_str() will only generate the contents in stack, and we passed in the the pointer to the function, it will lose the control of the content, resulting in the recv or send fail. Instead, the contents are received and stored as a form of **vector<char>** as this way it can be resized and accessed even convert to string in a convienient way.
## Request:

## Response:
## Proxy: