Printer spooler application with multiple processes, using shared-memory for inter-process communication

This program implements a printer spooler (server) application. The server spooler runs as a daemon process mimicking the printer server. It has a bounded circular buffer. The clientspooler process inserts job to print into this buffer. The printer server takes each job from the buffer (the buffer is treated as a FIFO queue) and prints it. Each job has a duration (time taken to print) as a defining parameter.
