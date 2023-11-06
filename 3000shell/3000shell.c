	/* 3000shell.c */
/* v2 Sept. 15, 2019 */
/* v1 Sept. 24, 2017 */
/* based off of csimpleshell.c, Enrico Franchi © 2005
      https://web.archive.org/web/20170223203852/
      http://rik0.altervista.org/snippets/csimpleshell.html */
/* Original under "BSD" license */
/* This version is under GPLv3, copyright 2017, 2019 Anil Somayaji */
/* You really shouldn't be incorporating parts of this in any other code,
   it is meant for teaching, not production */

/* This work is additionally annotated & edited under GPLv3, 2023 Sibte Kazmi. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 1<<16
#define ARR_SIZE 1<<16
#define COMM_SIZE 32

const char *proc_prefix = "/proc";

// parses the args for a program that is about to be executed in a child process
// places the parsed args in the args string array
// buffer is just the first arg already provided for ease of initializing the args list/ptr
// kinda weird since wbuf literally is just a pointer to this pointer intended to then be iterated....
void parse_args(char* buffer, char** args, size_t args_size, size_t *nargs)
{
	char* buf_args[args_size]; /* You need C99 */
	char** cp; // ptr for current position
	size_t i, j;
	
	char* wbuf = buffer;
	
	buf_args[0] = buffer;
	args[0] = buffer;
	
	// basically cp iterates over every arg position and we just copy the buffer split based off of " \n\t" characters
	// until we get a null, which means we have run out of args to copy over
	for(cp = buf_args; (*cp=strsep(&wbuf, " \n\t")) != NULL;)
		if ((*cp != NULL) && (++cp >= &buf_args[args_size]))
			break;
	
	// put it all in args the output variable while counting size
	for (j = i = 0; buf_args[i] != NULL; i++)
		if (strlen(buf_args[i]) > 0)
			args[j++] = buf_args[i];
	
	// put the size in the size output
	*nargs = j;
	args[j] = NULL; // remember to null terminate ;)
}

/* this is kind of like getenv() */ 
char* find_env(char* envvar, char* notfound, char* envp[])
{
	const int MAXPATTERN = 128;
	int i, p;
	char c;
	char pattern[MAXPATTERN];
	char *value = NULL;

	p = 0;
	while (c = envvar[p]) 
	{
		pattern[p] = c;
		p++;
		if (p == (MAXPATTERN - 2))
			break;
	}
	pattern[p] = '=';
	p++;
	pattern[p] = '\0';
	
	i = 0;
	while (envp[i] != NULL) 
	{
		if (strncmp(pattern, envp[i], p) == 0)                      
			value = envp[i] + p;
		i++;
	}

	if (value == NULL)
		return notfound;
	else 
		return value;
	
}

// aka the entry point into a called program (first function called in a child process)
// turns fn into the file name (which is args[0] for ./ or /) or path 
void find_binary(char* name, char* path, char* fn, int fn_size) 
{
	char* n;
	char* p;
	int r, stat_return;

	struct stat file_status; // file status (stat) structure, see <sys/stat.h>.
	
	// if the name is one of these its easy lmao
	if (name[0] == '.' || name[0] == '/') 
	{
		strncpy(fn, name, fn_size);
		return;
	}
	
	// now its not so simple, this thing was called and we have to search the path variable to see if its in any of them
	// we get the path, then we seperate it into the different directories (based off the : seperator) then we try each one
	p = path;
	while (*p != '\0') 
	{       
		r = 0;
		while (*p != '\0' && *p != ':' && r < fn_size - 1) 
		{
			fn[r] = *p;
			r++;
			p++;
		}

		fn[r] = '/';
		r++;
		
		n = name;
		while (*n != '\0' && r < fn_size) 
		{
			fn[r] = *n;
			n++;
			r++;
		}
		fn[r] = '\0';

		
		stat_return = stat(fn, &file_status); // stat func returns weather or not the file exists

		if (stat_return == 0)  // we found it
			return;
		
		if (*p != '\0') 
			p++;
	}
}

// Small helper function that builds a proc/{procid}/comm file path
void setup_comm_fn(char* pidstr, char* comm_fn)
{
	char* c;

	strcpy(comm_fn, proc_prefix);
	c = comm_fn + strlen(comm_fn);
	*c = '/';
	c++;
	strcpy(c, pidstr);
	c = c + strlen(pidstr);
	strcpy(c, "/comm");
}

void plist()
{
	DIR *proc;
	struct dirent *e;
	int result;
	char comm[COMM_SIZE];  /* seems to just need 16 */        
	char comm_fn[512];
	int fd, i, n;
	
	// we open the /proc directory....
	proc = opendir(proc_prefix);

	if (proc == NULL) 
		fprintf(stderr, "ERROR: Couldn't open /proc.\n");
	
	// you can call readdir() unix syscall in a loop to get each directory entry.
	// heres the function prototype:
	// struct dirent* readdir(DIR* dirp);
	// dirent is a struct in the dirent.h header and represents a single directory entry (file/folder)
	// the DIR pointer is a pointer to a directory, like the proc directory we opendir()'d before.
	// The function readdir() will return a directory entry until it has reached the end of the list; then it will return NULL.
	for (e = readdir(proc); e != NULL; e = readdir(proc)) 
	{
		if (isdigit(e->d_name[0])) 
		{
			setup_comm_fn(e->d_name, comm_fn); // here we take the file name out of the proc list and turn it into a file path (see the setup_comm_fn func above) 
			fd = open(comm_fn, O_RDONLY); // open it as read only...
			if (fd > -1) 
			{                                
				n = read(fd, comm, COMM_SIZE);
				close(fd);
				for (i=0; i < n; i++) 
				{
					if (comm[i] == '\n') 
					{
						comm[i] = '\0';
						break;
					}
				}
				printf("%s: %s\n", e->d_name, comm); // output each comm
			} 
			else 
			{
				printf("%s\n", e->d_name);
			}
		}
	}
	
	result = closedir(proc);
	if (result) 
		fprintf(stderr, "ERROR: Couldn't close /proc.\n");
}

void signal_handler(int the_signal)
{
	int pid, status;

	if (the_signal == SIGHUP) 
	{
		fprintf(stderr, "Received SIGHUP.\n");
		return;
	}
	
	if (the_signal == SIGUSR1) 
	{
		fprintf(stderr, "Ouch!\n");
		return;
	}
	
	if (the_signal != SIGCHLD) 
	{
		fprintf(stderr, "Child handler called for signal %d?!\n", the_signal);
		return;
	}
	pid = wait(&status);

	if (pid == -1) /* nothing to wait for */
		return;
	
	
	if (WIFEXITED(status)) 
		fprintf(stderr, "\nProcess %d exited with status %d.\n", pid, WEXITSTATUS(status));
	else
		fprintf(stderr, "\nProcess %d aborted.\n", pid);
}

void run_program(char* args[], int background, char* stdout_fn, char* path, char* envp[])
{
	pid_t pid;
	int fd, *ret_status = NULL; // amazing... int fd and int* ret_status huh.
	char bin_fn[BUFFER_SIZE];

	pid = fork(); // Okkkkk we forked the program. 
	// From now on we have 2 lines of execution.
	// if we have a value for pid that means we are the parent that forked
	// if we don't have a pid that means we are the child (obviously, since the child didn't call the fork() function - its the result of it)
	
	if (pid) // PARENT
	{
		if (background) // depending on the background flag we decide if we wait() (its a syscall, <sys/wait.h>) for the child or go back into console mode
			fprintf(stderr, "Process %d running in the background.\n", pid);
		else 
			wait(ret_status); // for some reason this was "pid = wait(ret_status);" not sure why.
	} 
	else // CHILD
	{
		find_binary(args[0], path, bin_fn, BUFFER_SIZE); // gets us the full path & name of the executable binary, stores in bin_fn
		
		if (stdout_fn != NULL) // if there is an output file specified
		{
			fd = creat(stdout_fn, 0666); // make one
			dup2(fd, 1);  // fd is an integer that points/reffers to the file, "file discriptor"
			// dup2() is a syscall used to duplicate a file discriptor, in this case we are duplicating output stream 1 fd to our new files fd
			// output stream 1 fd is the stdout stream of the process, so anything written to stdout is now copied to our file
			close(fd);
		}
		
		if (execve(bin_fn, args, envp)) // execve() execution syscall - replaces the current child process with a new one, specified by bin_fn
		{
			puts(strerror(errno)); 
			// errno is a global variable in C used by system calls to store returned error numbers. 
			// strerror() makes it human readable, puts places it on your screen.
			exit(127); // 127 error code = failed to execute as expected
		}
	}
}

void prompt_loop(char* username, char* path, char* envp[])
{
	char buffer[BUFFER_SIZE];
	char* args[ARR_SIZE];
	
	int background;
	size_t nargs;
	char* s;
	int i, j;
	char* stdout_fn;
	
	while(1)
	{
		printf("%s $ ", username);
		s = fgets(buffer, BUFFER_SIZE, stdin); // grab first line, username
		
		if (s == NULL) 
		{
			/* we reached EOF */
			printf("\n");
			exit(0);
		}
		
		parse_args(buffer, args, ARR_SIZE, &nargs); 
		
		if (nargs==0) continue;
		
		if (!strcmp(args[0], "exit"))
			exit(0);
		
		if (!strcmp(args[0], "plist")) 
		{
			plist();
			continue;
		}

		background = 0;            
		if (strcmp(args[nargs-1], "&") == 0) 
		{
			background = 1;
			nargs--;
			args[nargs] = NULL;
		}

		stdout_fn = NULL; // by default there is no output
		for (i = 1; i < nargs; i++) 
		{
			if (args[i][0] == '>')  // however if there is a > in the args...
			{
				stdout_fn = args[i]; // ptr to the '>' arg
				stdout_fn++; // increment ptr to the next arg (the output file name!)
				printf("Successfully set stdout to %s\n", stdout_fn);
				
				// now lets quietly delete this > from ever existing in the command...
				for (j = i; j < nargs - 1; j++)
					args[j] = args[j+1];
				
				nargs--;
				args[nargs] = NULL;
				break;
			}
		}
		
		// runs the program! see the run_program() function
		run_program(args, background, stdout_fn, path, envp);
	}    
}

int main(int argc, char *argv[], char *envp[])
{
        struct sigaction signal_handler_struct;
        
        char* username;
        char* default_username = "UNKNOWN";
        
        char* path;
        char* default_path = "/usr/bin:/bin";
        
        memset (&signal_handler_struct, 0, sizeof(signal_handler_struct));
        signal_handler_struct.sa_handler = signal_handler;
        signal_handler_struct.sa_flags = SA_RESTART;
        
        if (sigaction(SIGCHLD, &signal_handler_struct, NULL)) {
			fprintf(stderr, "Couldn't register SIGCHLD handler.\n");
        }
        
        if (sigaction(SIGHUP, &signal_handler_struct, NULL)) {
			fprintf(stderr, "Couldn't register SIGHUP handler.\n");
        }
        
		if (sigaction(SIGUSR1, &signal_handler_struct, NULL)) {
			fprintf(stderr, "Couldn't register SIGHUP handler.\n");
        }
		
        username = find_env("USER", default_username, envp);
        path = find_env("PATH", default_path, envp);

        prompt_loop(username, path, envp);
        
        return 0;
}
