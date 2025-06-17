## There are three files in this project:
1. main.cpp : This is the main file that contains the running/testing of the program.
2. logparser.h: This file contains the class definition for LogParser.
3. logparser.cpp: This file contains the implementation of the LogParser class.

## There are also log files in the logs directory.

## Algorithm
The program reads the log files and parse the messages. <br /> 

It puts the parsed messages into an unordered_map of messages and inserts the message id to a **"previous message id"** unordered_map.<br />

This helps to find a message which is not in the previous message id map that is the first message. This kind of messages can be more than once
so algorithm start sequencing for each of them.<br />

After that it creates a list and inserts the messages following the next_ids starting form the first message. 
Broken links are inserted to final list according the their place in the unordered_map of messages. <br />

Finally, it prints the messages in the vector:

![Capture](docs/Capture.PNG)

or the printable string can be received and used.

## Unit tests

The unit tests are implemented using Google Test framework. 
They check for malformed lines, correct parsing of messages, and the correct order of messages:
![Capture2](docs/Capture2.PNG)
