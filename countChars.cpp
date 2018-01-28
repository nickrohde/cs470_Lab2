#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <errno.h>

// Namespace:
using namespace std;

// Globals:
const int  i_LENGTH   = 26;					// number of characters to check for
const string tempFile = "temp"; 			// temp file for storing data

// Prototypes:
void countCharInFile(char, int*, fstream*); // counts the number of occurrences of character <arg1> in the given file in <arg3> and stores the number in the location that <arg2> points to
void printStats(int*, string&);             // prints the statistics for the given file
void makeTempFile(void);					// makes a temporary file for storage
void storeResult(char, int);				// stores the result in the temporary file
void writeResultToArray(int*);				// writes the results from the temp file to an array pointed to by <arg1>

bool isFileValid(string);					// checks if a given file name is valid

string askUserForFileName(void);			// asks the user to input a filename

int main(int argc, char **argv)
{
	// Variables: 
	string s_fileName;
	int* ia_results = new int[i_LENGTH]; 
	fstream file;
	
	
	
	if(argc > 1)
	{
		s_fileName = string(argv[1]);
		while (!isFileValid(s_fileName)) // ensure file exists
		{
			s_fileName = askUserForFileName();
		} // end while 
	} // end if
	else // if no file was passed as argument, ask the user for a filename
	{
		do
		{
			s_fileName = askUserForFileName();
		} while (!isFileValid(s_fileName)); // ensure file exists
	} // end else
	
	makeTempFile(); // improvised shared memory
		
	for(int i = 0; i < i_LENGTH; i++) // spawn a child for each character of the alphabet and go to the next
	{
		ia_results[i] = 0;
		
		pid_t pid = fork();
		
		if(pid == 0)
		{
			// Variables:
			int temp = 0;
			char current = (char)(i + 97); // a = 97
			
			
			try
			{
				file.open(s_fileName.c_str(), ios::in);
				
				countCharInFile(current, &temp, &file);
				
				storeResult(current, temp);
				
				file.close();
				exit(EXIT_SUCCESS);
			} // end try
			catch(exception e)
			{
				exit(EXIT_FAILURE);
			} // end catch
		} // end if
	} // end for

	
	for(;;) // wait for all children to finish
	{
		// Variables:
		int status;
		pid_t child = wait(&status);
		
		
		if (child == -1)
		{
			if (errno == ECHILD)
				break; // no more children
		} // end if
	} // end for
	
	writeResultToArray(&ia_results[0]); // get stored data back from temp file
	
	printStats(ia_results, s_fileName);
	
	// clean up
	delete[] ia_results;
	remove(tempFile.c_str());
	
	return EXIT_SUCCESS;
} // end method Main


void countCharInFile(char c_target, int *i_result, fstream* file)
{
	// Variables:
	int i_count = 0;
	
	
	if(file->is_open() && !file->bad())
	{
		char c;
		while(file->get(c))
		{
			c = tolower(c); // ignore upper case
			
			if(c == c_target)
			{
				i_count++;
			} // end if
		} // end while
	} // end if
	
	*i_result = i_count;
} // end method countCharInFile

void printStats(int* ia_results, string& s_fileName)
{
	cout << "Statistics for " << s_fileName << ":" << endl;
	
	for(int i = 0; i < i_LENGTH; i++)
	{
		if(ia_results[i] > 0)
		{
			cout << "The character " << (char)(i+97) << " appears " << ia_results[i] << " times in the given file." << endl;
		} // end if
		else
		{
			cout << "The character " << (char)(i+97) << " did not appear in the given file." << endl;
		} // end else
	} // end for
} // end method printStats


void makeTempFile(void)
{
	remove(tempFile.c_str()); // delete any previous temp file to avoid issues
	fstream file(tempFile.c_str(), ios::in | ios::out | ios::trunc);
} // end method makeSortedFile


void storeResult(char val, int result)
{
	// Variables:
	fstream file;
	stringstream ss;
	

	file.open(tempFile.c_str(), ios::in | ios::out | ios::app);
	
	ss << val;
	ss << " ";
	ss << result;
	ss << "\n";
	
	if (file.is_open())
	{
		file << ss.str();
		file.close();
	} // end if
} // end method storeResult


void writeResultToArray(int* result)
{ // a = 97
	// Variables:
	ifstream file;
	
	
	file.open(tempFile.c_str());
	
	if(file.is_open() && !file.bad())
	{
		// Variables:
		char c;
		int  i;
		
		
		while(file >> c >> i)
		{
			result[(int)c - 97] = i;
		} // end while
	} // end if
	
	file.close();
} // end method writeResultToArray

string askUserForFileName(void)
{
	// Variables:
	string s_name;
	
	
	cout << "Please enter a file name: ";
	getline(cin, s_name);
	
	return s_name;
} // end method askUserForFileName

bool isFileValid(string s_fileName)
{
	// Variables:
	fstream file;
	bool b_out = false;
	
	
	file.open(s_fileName.c_str());
	
	if(!file.is_open() || file.bad())
	{
		cout << "\nFile could not be opened! Please check the filename and try again.\n\n";
	} // end if
	else
	{
		file.close();
		b_out = true;
	} // end else

	return b_out;
} // end method isFileValid
