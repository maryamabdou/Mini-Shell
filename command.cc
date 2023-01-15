
/*
 * CS354: Shell project
 *=
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#include "command.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include<time.h>
#include <glob.h>
using namespace std;

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
	_err = 0;
	_exit = 0;
	_astr = 0;
	_ques = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile && !_errFile) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile && !_outFile) {
		free( _errFile );
	}

	if ( _errFile && _outFile) {
		free( _outFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
	_err = 0;
	_exit = 0;
	_astr = 0;
	_ques = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i+1 );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );

	
	if (_outFile != 0 && _err == 0)  // >
	{
		if (_append == 0) // >
		{
			mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
			char *file = _outFile;
			int file_desc = creat(file, mode);
			dup2(file_desc, 1);
			close(file_desc);
		}
		else{  // >>
			char *file = _outFile;
			FILE* file_desc = fopen(file, "a");
			dup2(fileno(file_desc), 1);
			fclose(file_desc);
		}
	}

	if (_err != 0 && _outFile != 0)  // >>&
	{
		char *file = _outFile;
		FILE* file_desc = fopen(file, "a");
		dup2(fileno(file_desc), 1);
		dup2(fileno(file_desc), 2);
		fclose(file_desc);
	}

	if (_inputFile != 0) // <
	{
		char *file = _inputFile;
		FILE* file_desc = fopen(file, "r");
		dup2(fileno(file_desc), 0);
		fclose(file_desc);
	}
	
	if (_numberOfSimpleCommands == 1){  //simple command

		// exit
		if(strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0){
			printf("Good Bye !!\n");
			exit(2);
		}

		// cd
		if (strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0){
			if(_simpleCommands[0]->_arguments[1] != NULL){
				chdir(_simpleCommands[0]->_arguments[1]);
			}
			else{
				chdir(getenv("HOME"));
			}

			clear();
			prompt();
			return;
		}

		if(strcmp(_simpleCommands[0]->_arguments[0], "echo") == 0){
		
			char **found;
			glob_t gstruct;
			int r;
			r = glob(_simpleCommands[0]->_arguments[1], GLOB_ERR , NULL, &gstruct);

			if (gstruct.gl_pathc == 0){
				
			}
			else{
				found = gstruct.gl_pathv;
				while(*found)
				{
					printf("%s\n",*found);
					found++;
				}
				clear();
				prompt();
				return;
			}
					
		}

		int pid = fork();

		if (pid == -1)
		{
			perror( "Command not found\n");
			exit( 2 );
		}

		if (pid == 0)
		{
			int simple_code = execvp(_simpleCommands[0]->_arguments[0], _simpleCommands[0]->_arguments);
			perror( "Command not found\n");
			exit( 2 );
		}
		
		if (_background == 0)
		{
			waitpid( pid, 0, 0 );
		}		
	}

    else  //piping
    {
        
        int status;
        int fdpipe[2*(_numberOfSimpleCommands - 1)];

        pipe(fdpipe);
        int pid = fork();

		if (pid == -1)
		{
			perror( "Command not found\n");
			exit( 2 );
		}

        if (pid == 0){ //first command
            dup2( fdpipe[1], 1);
            for ( int j = 0; j < 2*(_numberOfSimpleCommands - 1); j++ )
            {
                close(fdpipe[j]);
            }
            int simple_code = execvp(_simpleCommands[0]->_arguments[0], _simpleCommands[0]->_arguments);
			perror( "Command not found\n");
			exit( 2 );
        }
        else{
            if (_background == 0)
            {
                waitpid( pid, 0, 0 );
            }
            for ( int i = 0; i < (_numberOfSimpleCommands - 2) ; i++ )  //in between commands
            {
                pipe(fdpipe +(2*(i+1)));
                int pid = fork();

				if (pid == -1)
				{
					perror( "Command not found\n");
					exit( 2 );
				}

                if (pid == 0)
                {
                    dup2( fdpipe[2*i], 0 );
                    dup2( fdpipe[(2*i)+3], 1 );

                    for ( int j = 0; j < 2*(_numberOfSimpleCommands - 1); j++ )
                    {
                        close(fdpipe[j]);
                    }
                    int simple_code = execvp(_simpleCommands[i+1]->_arguments[0], _simpleCommands[i+1]->_arguments);
					perror( "Command not found\n");
					exit( 2 );
                }
            }

            int pid = fork();  //last command

			if (pid == -1)
			{
				perror( "Command not found\n");
				exit( 2 );
			}
			
            if (pid == 0){
                dup2( fdpipe[2*(_numberOfSimpleCommands - 2)], 0 );
                for ( int j = 0; j < 2*(_numberOfSimpleCommands - 1); j++ )
                {
                    close(fdpipe[j]);
                }
                int simple_code = execvp(_simpleCommands[_numberOfSimpleCommands - 1]->_arguments[0], _simpleCommands[_numberOfSimpleCommands - 1]->_arguments);
				perror( "Command not found\n");
				exit( 2 );
            }
            for ( int j = 0; j < 2*(_numberOfSimpleCommands - 1); j++ )
            {
                close(fdpipe[j]);
            }

            if (_background == 0)
            {
				for(int i = 0; i<_numberOfSimpleCommands; i++){
					wait(&status);
				}

				waitpid( pid, 0, 0 );
			}
		
        }

        for ( int j = 0; j < 2*(_numberOfSimpleCommands - 1); j++ )
        {
            close(fdpipe[j]);
        }

        if (_background == 0)
        {
			waitpid( pid, 0, 0 );
        }
        
    }
    
    
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);

	close(defaultin);
	close(defaultout);
	close(defaulterr);
	

	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;


int yyparse(void);

void handler(int sig)
{
	time_t t = time(NULL);
	pid_t chpid = wait(NULL);
	FILE* file = fopen("data.log", "a"); 
  	fprintf(file, "Child pid %d ended. Current date and time is : %s", chpid, ctime(&t));
	fclose(file);
}

int 
main()
{
	signal(SIGINT, SIG_IGN);  //ctrl c

	signal (SIGCHLD, handler); //log file

	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

