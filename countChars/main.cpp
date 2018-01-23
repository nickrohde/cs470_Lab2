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
const int  i_LENGTH   = 26;
const string tempFile = "temp";

// Prototypes:
void countCharInFile(char, int*, fstream*);
void printStats(int*, string&);
void makeTempFile(void);
void storeResult(char, int);
void writeResultToArray(int*);

bool isFileValid(string);

string askUserForFileName(void);

int main(int argc, char **argv)
{
	// Variables: 
	string s_fileName;
	int* ia_results = new int[i_LENGTH]; 
	fstream file;
	
	
	if(argc > 1)
	{
		s_fileName = string(argv[1]);
		while (!isFileValid(s_fileName))
		{
			s_fileName = askUserForFileName();
		} // end while 
	} // end if
	else
	{
		do
		{
			s_fileName = askUserForFileName();
		} while (!isFileValid(s_fileName));
	} // end else
	
	makeTempFile(); // improvised shared memory
		
	for(int i = 0; i < i_LENGTH; i++)
	{
		ia_results[i] = 0;
		
		pid_t pid = fork();
		
		if(pid == 0)
		{
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

	for(;;) // wait for all children to die
	{
		int status;
		pid_t done = wait(&status);
		
		if (done == -1) 
		{
			if (errno == ECHILD)
				break; // no more children
		} // end if
	} // end for
	
	writeResultToArray(&ia_results[0]); // get stored data from file
	
	printStats(ia_results, s_fileName);
	
	// clean up
	delete[] ia_results;
	remove(tempFile.c_str());
	
	return EXIT_SUCCESS;
} // end method Main


void countCharInFile(char c_target, int *i_result, fstream* file)
{
	// Variables:
	int count = 0;
	
	
	if(file->is_open() && !file->bad())
	{
		char c;
		while(file->get(c))
		{
			c = tolower(c); // ignore upper case
			
			if(c == c_target)
			{
				count++;
			} // end if
		} // end while
	} // end if
	
	*i_result = count;
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
	remove(tempFile.c_str()); // delete any previous temp file
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
		char a;
		int  b;
		
		
		while(file >> a >> b)
		{
			result[(int)a - 97] = b;
		} // end while
	} // end if
} // end method writeResultToArray

string askUserForFileName(void)
{
	// Variables:
	string out;
	
	
	cout << "Please enter a file name: ";
	getline(cin, out);
	
	return out;
} // end method askUserForFileName

bool isFileValid(string s_fileName)
{
	// Variables:
	fstream file;
	bool out = false;
	
	
	file.open(s_fileName.c_str());
	
	if(!file.is_open() || file.bad())
	{
		cout << "\nFile could not be opened! Please check the filename and try again.\n\n";
	} // end if
	else
	{
		file.close();
		out = true;
	} // end else

	return out;
} // end method isFileValid