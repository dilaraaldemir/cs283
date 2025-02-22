Can you think of why we use fork/execvp instead of just calling execvp directly? What value do you think the fork provides?

Answer: fork creates a child process to run the command, so the parent shell can keep going. Without it, the shell would be replaced by the command, and you wouldn’t be able to interact with the shell anymore.

What happens if the fork() system call fails? How does your implementation handle this scenario?

Answer: If fork() fails, it returns -1. The program should check for this and print an error message or return an error code to handle the issue properly.

How does execvp() find the command to execute? What system environment variable plays a role in this process?

Answer: execvp looks through the directories listed in the PATH environment variable to find the command. If it can't find it, it returns -1 and sets errno.

What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

Answer: wait() makes sure the parent process collects the exit status of the child process, preventing it from becoming a zombie process. Without it, the child might be left hanging.

In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

Answer: WEXITSTATUS() retrieves the exit status of the child process, which tells you if the command ran successfully. It’s useful for error handling and understanding whether things went as expected.

Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

Answer: It keeps arguments within quotes as a single entity, which is key for handling spaces inside quotes correctly when passing them to a command.

What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

Answer: I made adjustments for handling pipes, background processes, and quoted arguments. Refactoring was tricky, especially when dealing with edge cases or special characters.

For this quesiton, you need to do some research on Linux signals. You can use this google search to get started.

What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

Answer: Signals notify processes about events, while IPC lets processes exchange data. Signals are lightweight and asynchronous, whereas IPC is for transferring data between processes.

Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

Answer: 
-SIGQUIT: Similar to SIGINT, but also causes the process to generate a core dump. Typically used when the user wants to terminate a process and inspect it later.
-SIGUSR1: A user-defined signal. It is used for custom purposes by programs and can be caught or ignored.
-SIGSEGV: Sent when a process tries to access memory that it is not allowed to. It’s a segmentation fault signal and typically indicates a bug in the program.

What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

Answer: SIGSTOP pauses the process, and it can’t be caught or ignored. It’s used to temporarily stop a process, often for debugging.
