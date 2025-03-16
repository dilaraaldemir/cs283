1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?
To make sure each child process completes before taking in new input, the shell calls waitpid() on each one. In the absence of waitpid(), zombie processes expand, consuming system resources and producing unpredictable outcomes.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?
After dup2(), unused pipe ends need to be closed to avoid blocking, resource leaks, and unwanted data flow. Processes may hang indefinitely or exhaust file descriptors if pipes remain open.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?
An external process can't change the shell's working directory like cd can. Only the child process, not the shell itself, would be impacted if it were applied externally.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?
Use malloc() and realloc() to swap out the fixed CMD_MAX for a newly allocated list. Increased memory usage, resizing performance overhead, and more complex code are all trade-offs.

