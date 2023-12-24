# Shell-simulation
Developed a custom command shell in C, utilizing OS system calls to execute built-in Linux commands. The shell runs an infinite loop, only exiting with the 'exit' command. It displays a prompt indicating the current working directory followed by '$'. The shell supports the 'cd' command for changing the working directory using cd <directoryPath> or cd ...

It handles error scenarios, printing 'Shell: Incorrect command' for incorrect formats and displaying error messages generated during command execution. The shell responds to signals generated from the keyboard using 'Ctrl + C' and 'Ctrl + Z', continuing to work and only stopping with the 'exit' command.

The shell supports both sequential execution (using '##') and parallel execution (using '&&'), waiting for all commands to terminate before accepting further inputs. Output redirection is implemented for commands using the '>' symbol (e.g., ls > info.out writes output to 'info.out').

The code is designed to be modular and readable, following proper indentation and including comments for clarity. It avoids unnecessary complexity, adheres to grading guidelines, and does not use external library functions requiring additional installations or command line arguments during compilation. It is compatible with Linux systems and focuses on simplicity for ease of understanding.
