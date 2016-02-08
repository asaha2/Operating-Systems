#printer-spooler

This program implements a printer spooler (server) application with multiple processes, using shared-memory for inter-process communication. The server spooler runs as a daemon process mimicking the printer server. It has a bounded circular buffer. The client spooler process inserts job to print into this buffer. The printer server takes each job from the buffer (the buffer is treated as a FIFO queue) and prints it. Each job has a duration (time taken to print) as a defining parameter.
