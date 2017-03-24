# Rubix OS

Hedgehog logo © Laymik, licensed under CC-BY 3.0 US.
https://thenounproject.com/term/hedgehog/582725/

## Features

* SYS Calls
	* `yield` - ends the current time slice.
	* `write` - writes to an open file descriptor.
	* `read`  - reads from an file descriptor. Returns length of read, 0 on EOF. Blocking.
	* `close` - closes a file descriptor.
	* `dup2`  - duplicates fd from `old` to `new`. `new` is closed if it already exists.
	* `pipe`  - creates a pipe. fd[0] is read, fd[1] is write.
	* `fork`  - clones process. Return value is 0 if child, PID of child if parent, -1 if error.
	* `exec`  - replaces the process with another program. PID is kept. Stack and FDs are destroyed.
	* `exit`  - exits with exit code.
	* `wait`  - waits for a child program to exit, and gives exit code.
* Multi-processing
	* **time slicing** - timer based timer slices.
	* **priority-based** - priority(P) = priority_base(P) + slices_since_last_ran(P)
	* **blocked queue** - for processes waiting for a process or file resource.
* Objects
	* `FiDes` - File descriptor. Interacted with using function pointers.
	* `pipe`  - Pointed to by a FD.

## References

All references access March 2017.

### Online

* [Linux Man-pages](http://man7.org/linux/man-pages/). Various. man7.org
* [Wait sys call](http://pubs.opengroup.org/onlinepubs/000095399/functions/waitpid.html). The Open Group. pubs.opengroup.org
* [Exit sys call](http://pubs.opengroup.org/onlinepubs/000095399/functions/exit.html). The Open Group. pubs.opengroup.org
* [POSIX IPC](https://docs.oracle.com/cd/E19455-01/806-4750/6jdqdfltf/index.html). Oracle. docs.oracle.com
* [Linux Interprocess Communications](http://tldp.org/LDP/lpg/node7.html). Goldt. S, Van der Meer. S, Welsh. M. tldp.org
* [Interprocess Communication (IPC), Pipes](https://users.cs.cf.ac.uk/Dave.Marshall/C/node23.html). Marshall, D. Users.cs.cf.ac.uk
* [General overview of the Linux file system](http://www.tldp.org/LDP/intro-linux/html/sect_03_01.html). Machtelt. G. tldp.org

### Print

* Cormen. T, Leiserson. C, Rivest. R, Stein. C. *Introduction to Algorithms 3rd Edition*,  (Massachusetts Institute of Technology, 2009) Heapsort and Priority Queues. pp151-166
