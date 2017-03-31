# Rubix OS

## Features

* SYS Calls
	* `yield` - ends the current time slice.
	* `write` - writes to an open file descriptor.
	* `read`  - reads from an file descriptor. Returns length of read, 0 on EOF.
	            May blocking - see `set_nonblocking`.
	* `close` - closes a file descriptor.
	* `dup2`  - duplicates fd from `old` to `new`. `new` is closed if it already exists.
	* `pipe`  - creates a pipe. fd[0] is read, fd[1] is write.
	* `fopen` - open file. Not quite POSIX, as it's non-blocking
	* `fork`  - clones process.
	            Return value is 0 if child, PID of child if parent, -1 if error.
	* `exec`  - replaces the process with another program. PID is kept.
	            Stack and FDs (except in/out/err) are destroyed.
	* `exit`  - exits with exit code.
	* `wait`  - waits for a child program to exit, and gives exit code.
	* `kill`  - sends a kill signal to a process. Killed processes will not return an exit code.
	            `signal` is not yet implemented.
	* `setpriority` - set priority of child process.
	* `set_nonblocking` - is not POSIX, unfortunately. Set pipe non-blocking.
* LibC help functions
	* `popen` - opens a process and returns a FD. Uses `fork`, `pipe`, `exec`, and `dup2`.
	* `wait`/`waitpid` - both use the `wait` syscall.
* Processe
	* **time slicing**      - timer based timer slices.
	* **priority-based**    - priority(P) = priority_base(P) + slices_since_last_ran(P)
	* **blocked queue**     - for processes waiting for a process or file resource.
	* **process ownership** - processes have a parent, which can kill/wait them.
	* **process groups**    - a limited type of process group, where all processes
	                          that share a parent and the parent itself are in a group.
* Files
	* `FiDes` - File descriptor. Interacted with using function pointers. Can be blocking or not.
	* `pipe`  - Pointed to by a FD.
	* `in/out/err` - these are "files" too!
	* `filesystem` - Files are limited to 256 bytes, maximum of 10 files.

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
