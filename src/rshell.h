#ifndef _RSHELL_H_
#define _RSHELL_H_

#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h> //waitpid
#include <sys/wait.h> //waitpid
#include <unistd.h> //execve, pid_t, getlogin, gethostname
#include <stdlib.h>  
#include <stdio.h> //perror

/* Base Class */
class CommandLine 
{
	public:
		CommandLine() { };
		virtual int run() = 0;
};

/* Leaf Class */
class Command : public CommandLine
{
	private:
		char *arg[64]; // Command and Argument or Flag list
		std::vector<std::string> vec;
		
	public:
		Command () { }
		
		Command(std::vector<std::string> vec) : vec(vec)
		{ }
		
		void display() // Extra credit: gethostname, getlogin 
		{ 
			char hostname[1024];
			char username[1024];
			
			gethostname(hostname, 1024);
			getlogin_r(username, 1024);
			
//			char *p = strtok (hostname, " ,.-");	//ERROR: Unused variable (*p)
			
			std::cout << "[" << username << "@" << hostname << "]" << "$ ";
		}; 
		
		int run() // Executes the command
		{
			pid_t pid = fork(); // Fork and get child pid
			
			if (pid == -1) // If pid is -1, error and fork() has failed
			{
			    exit(-1);
			} 
			else if (pid > 0) // If pid is positive and greater than 0, wait 
			//for child process
			{
			    int status;
			    waitpid(pid, &status, 0);
			}
			else // If pid is 0, execute command in child process
			{
				unsigned int i;
				int err;
				
				for (i = 0; i < vec.size(); ++i) //Create argv array
				{
					this->arg[i] = const_cast<char*>(vec.at(i).c_str());
				}
				this->arg[i] = NULL;
				
			    err = execvp(arg[0], arg); // Execute command
			    
			    //Check error, output perror if execvp does not execute
			    //output error message with cmd
			    std::string c_message(arg[0]);
			    std::string e_message = "Error: " + c_message;
			    if (err == -1) perror(e_message.c_str()); 
			    
			    return err;
			    
			    // Exec never returns
			    _exit(EXIT_FAILURE);   
			}
			
			return 0;
		};
};

/* Composite Class */
class Connector : public CommandLine 
{
	protected:
		std::string line;
		std::vector<CommandLine*> commands; //List of strings and connectors
		bool runCondition;	//Command checks parent composite class	
		
	public:
		Connector() { }
		
		Connector(std::string input) : line(input) { }
		
		//Run all commands in the vector
		virtual int run() 
		{ 
		    for (unsigned int i = 0; i < commands.size(); ++i) 
		    { 
		        commands.at(i)->run(); 
		    } 
		    
		    return 0;
		};
		
        virtual void parse()
		{
			std::vector<std::string> vl;
			char *cstr, *p;
			
			cstr = new char [line.size() + 1];
			strcpy (cstr, line.c_str());
		
		    //Removes any comments from being run or gets everything before 
		    //comment
			p = strtok(cstr, "#"); 
			//Separates all commands
			p = strtok(cstr, ";"); 
			while (p != NULL)
			{
				// std::cout << p << std::endl;
				vl.push_back(p);
				p = strtok(NULL, ";");
			}

            //Creates a command for each one that was separated by ';'
			for (unsigned int i = 0; i < vl.size(); ++i) 
			{
				std::vector<std::string> vc;
				std::string find = " ";
				cstr = new char [vl.at(i).size() + 1];
				strcpy (cstr, vl.at(i).c_str());
				
				// Separate argument list from actual command
				p = strtok (cstr, find.c_str()); 
				while (p != NULL) 
				{
				// 	std::cout << p << std::endl;
					vc.push_back(p);
					p = strtok(NULL, find.c_str());
				}
				
				// Fill vector with all commands
			    commands.push_back(new Command(vc)); 

			}
			
		    delete[] cstr;
		};
};

//AND
class AND : public Connector 
{
    private:
        std::vector<std::string> vec;
        
	public:
	    AND() { }
	    
	    AND(std::vector<std::string> vec) : vec(vec) { this->parse(); }
	    
	    int run() 
	    { 
	        for (unsigned int i = 0; i < commands.size(); ++i) 
		    { 
		        if (commands.at(i)->run() == -1) return -1;
		    }
		    
		    return 0;
	    };
	    
	    void parse();
};

//OR
class OR : public Connector 
{
    private:
        std::vector<std::string> vec;
        
	public:
	    OR() { }
	    
	    OR(std::vector<std::string> vec) : vec(vec) { this->parse(); }
	    
        int run() 
        { 
		    for (unsigned int i = 0; i < commands.size(); ++i) 
		    { 
		        if (commands.at(i)->run() == 0) return 0;
		    }
		    
		    return -1;
        };
	    
        void parse()
        {
        	char *cstr, *p;

			for (unsigned int i = 0; i < vec.size(); ++i) 
			{
				std::vector<std::string> vc;
				std::string find = " ";
				cstr = new char [vec.at(i).size() + 1];
				strcpy (cstr, vec.at(i).c_str());
				
			    if (vec.at(i).find("&&") != std::string::npos)
			    {
			        find = "&&";
			    }
				
				// Separate argument list from actual command
				p = strtok (cstr, find.c_str()); 
				while (p != NULL) 
				{
					vc.push_back(p);
					p = strtok(NULL, find.c_str());
				}
				
				if (find == "&&")
				{
				    commands.push_back(new AND(vc));
				}
				else 
				{
				    commands.push_back(new Command(vc)); 
				}
			}
        };
};

//Terminating
class Terminate : public Connector 
{
	public:
	    Terminate() : Connector() { }
	    
	    Terminate(std::string input) : Connector(input) { }
	    
	    int run() { Connector::run(); return 0;};
	    
	    void parse()
		{
			std::vector<std::string> vl;
			char *cstr, *p;
			
			cstr = new char [line.size() + 1];
			strcpy (cstr, line.c_str());
		
		    //Removes any comments from being run or gets everything before 
		    //comment
			p = strtok(cstr, "#"); 
			//Separates all commands
			p = strtok(cstr, ";"); 
			while (p != NULL)
			{
				// std::cout << p << std::endl;
				vl.push_back(p);
				p = strtok(NULL, ";");
			}

            //Creates a command for each one that was separated by ';'
			for (unsigned int i = 0; i < vl.size(); ++i) 
			{
				std::vector<std::string> vc;
				std::string find = " ";
				cstr = new char [vl.at(i).size() + 1];
				strcpy (cstr, vl.at(i).c_str());
				
			    if (vl.at(i).find("||") != std::string::npos)
			    {
			        find = "||";
			    }
			    else if (vl.at(i).find("&&") != std::string::npos)
			    {
			        find = "&&";
			    }
				
				// Separate argument list from actual command
				p = strtok (cstr, find.c_str()); 
				while (p != NULL) 
				{
					vc.push_back(p);
					p = strtok(NULL, find.c_str());
				}
				
				// Fill vector with all commands
				if (find == "||")
				{
				    commands.push_back(new OR(vc));
				}
				else if (find == "&&")
				{
				    commands.push_back(new AND(vc));
				}
				else 
				{
				    commands.push_back(new Command(vc)); 
				}
			}
			
		    delete[] cstr;
		};
};

void AND::parse()
{
	char *cstr, *p;

	for (unsigned int i = 0; i < vec.size(); ++i) 
	{
		std::vector<std::string> vc;
		std::string find = " ";
		cstr = new char [vec.at(i).size() + 1];
		strcpy (cstr, vec.at(i).c_str());
		
	    if (vec.at(i).find("||") != std::string::npos)
	    {
	        find = "||";
	    }
		
		// Separate argument list from actual command
		p = strtok (cstr, find.c_str()); 
		while (p != NULL) 
		{
			vc.push_back(p);
			p = strtok(NULL, find.c_str());
		}
		
		if (find == "||")
		{
		    commands.push_back(new OR(vc));
		}
		else 
		{
		    commands.push_back(new Command(vc)); 
		}
	}
}

#endif
