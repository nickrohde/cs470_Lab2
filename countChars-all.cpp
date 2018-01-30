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
#include <limits>

// Namespace:
using namespace std;

// Globals:
const int  i_LENGTH   = 26;					// number of characters to check for
const string tempFile = "temp"; 			// temp file for storing data
const string tempFilePerms = "ptemp";		// temp file for storing permutation data

// Prototypes:
void countCharInFile(char, int*, fstream*); // counts the number of occurrences of character <arg1> in the given file in <arg3> and stores the number in the location that <arg2> points to
void countCharPairsInFile(char, char, int*, fstream*); // counts the number of occurrences of pairs of <arg1> and <arg2> in the given file in <arg3> and stores the number in the location that <arg2> points to
void printStats(int*, string&);             // prints the statistics for single letters in the given file
void printStats(int**, string&);			// prints the statistics for pairs in the given file
void makeTempFile(void);					// makes a temporary file for storage
void storeResult(char, int);				// stores the result in the temporary file
void storeResult(char, char, int);
void writeResultToArray(int*);				// writes the results from the temp file to an array pointed to by <arg1>
void writeResultToArray(int**);				// writes the results from the temp file to an array pointed to by <arg1>

bool isFileValid(string);					// checks if a given file name is valid

string askUserForFileName(void);			// asks the user to input a filename

// driver method
int main(int argc, char **argv)
{
	// Variables: 
	string s_fileName;
	int* ia_results_single = new int[i_LENGTH];
	int** ia_results_permutations = new int*[i_LENGTH]; 
	
	for(int i = 0; i < i_LENGTH; i++)
	{
		ia_results_permutations[i] = new int[i_LENGTH];
	} // end for
	
	
	fstream file;
	
	try
	{
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
	} // end try
	catch(exception e)
	{
		delete[] ia_results_single;
		
		for(int i = 0; i < i_LENGTH; i++)
		{
			delete[] ia_results_permutations[i];
		} // end for
		
		delete[] ia_results_permutations;
		return EXIT_FAILURE;
	} // end catch 
	
	makeTempFile(); // improvised shared memory
		
	for(int i = 0; i < i_LENGTH; i++) // spawn a child for each character of the alphabet and go to the next
	{
		ia_results_single[i] = 0;
		
		pid_t pid = fork();
		
		if(pid == 0)
		{
			// Variables:
			int  i_single = 0;
			char c_current = (char)(i + 97); // a = 97
			
			
			try
			{
				file.open(s_fileName.c_str(), ios::in);
				
				countCharInFile(c_current, &i_single, &file);
				
				storeResult(c_current, i_single);
				
				file.close();
				
				for(int j = 0; j < i_LENGTH; j++)
				{
					char c_second = (char)(j + 97);
					
					ia_results_permutations[i][j] = 0;
					
					pid_t pid_child = fork();
					
					if(pid_child == 0)
					{
						try
						{
							int temp = 0;
							fstream file2;
							file2.open(s_fileName.c_str(), ios::in);
							countCharPairsInFile(c_current, c_second, &temp, &file2);
							file2.close();
							
							storeResult(c_current, c_second, temp);
							
							exit(EXIT_SUCCESS);
						} // end try
						catch(exception e)
						{
							exit(EXIT_FAILURE);
						} // end catch
					} // end if
				} // end for j
				
				for(;;) // wait for all grandchildren to finish
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
				
				exit(EXIT_SUCCESS);
			} // end try
			catch(exception e)
			{
				exit(EXIT_FAILURE);
			} // end catch
		} // end if
	} // end for i

	
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
	
	writeResultToArray(ia_results_single); // get stored data back from temp file
	writeResultToArray(ia_results_permutations);
	
	printStats(ia_results_single, s_fileName);
	printStats(ia_results_permutations, s_fileName);
	
	// clean up
	delete[] ia_results_single;
	
	for(int i = 0; i < i_LENGTH; i++)
	{
		delete[] ia_results_permutations[i];
	} // end for
	
	delete[] ia_results_permutations;
	remove(tempFile.c_str());
	remove(tempFilePerms.c_str());
	
	return EXIT_SUCCESS;
} // end method Main

// counts the number of times <arg1> and <arg2> appear together in the given file
void countCharPairsInFile(char c_first, char c_second, int *i_result, fstream* file)
{
	// Variables:
	int i_count = 0;
	
	
	if(file->is_open() && !file->bad())
	{
		string s;
		
		while(getline(*file,s,'\n'))
		{
			int i_length = s.length();
			
			for(int i = 0; i < i_length-1; i++)
			{
				char c1 = tolower(s[i]);
				char c2 = tolower(s[i+1]);
				if(c1 == c_first && c2 == c_second)
				{
					i_count++;
				} // end if
			} // end for i
		} // end while
	} // end if
	
	*i_result = i_count;
} // end method countCharPairsInFile

// counts the number of times <arg1> appears in the given file
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

// Prints stats for the single letters
void printStats(int* ia_results, string& s_fileName)
{
	cout << "Single letter statistics for " << s_fileName << ":" << endl;
	
	for(int i = 0; i < i_LENGTH; i++)
	{
		if(ia_results[i] > 0)
		{
			cout << "The character " << (char)(i+97) << " appears " << ia_results[i] << " time" << (ia_results[i] > 1 ? "s":"") << " in the given file." << endl;
		} // end if
		else
		{
			cout << "The character " << (char)(i+97) << " did not appear in the given file." << endl;
		} // end else
	} // end for
} // end method printStats

// Prints stats for the double letters
void printStats(int** ia_results, string& s_fileName)
{
	cout << "\n----------------------------------------------------------\n" << endl;
	cout << "Double letter statistics for " << s_fileName << ":" << endl;
	
	for(int i = 0; i < i_LENGTH; i++)
	{
		for(int j = 0; j < i_LENGTH; j++)
		{
			if(ia_results[i] > 0)
			{
				cout << "The character pair " << (char)(i+97) << (char)(j+97) << " appears " << ia_results[i][j] << " time" << (ia_results[i][j] > 1 ? "s":"") << " in the given file." << endl;
			} // end if
			else
			{
				cout << "The character pair " << (char)(i+97) << (char)(j+97) << " did not appear in the given file." << endl;
			} // end else
		} // end for j
	} // end for i
} // end method printStats

// creates temporary files
void makeTempFile(void)
{
	remove(tempFile.c_str()); // delete any previous temp file to avoid issues
	remove(tempFilePerms.c_str());
	fstream file1(tempFile.c_str(), ios::in | ios::out | ios::trunc);
	fstream file2(tempFilePerms.c_str(), ios::in | ios::out | ios::trunc);
} // end method makeSortedFile

// store for single letters
void storeResult(char c_val, int i_single)
{
	// Variables:
	fstream file;
	stringstream ss;
	

	file.open(tempFile.c_str(), ios::out | ios::app);
	
	ss << c_val;
	ss << " ";
	ss << i_single;
	ss << "\n";
	
	if (file.is_open())
	{
		file << ss.str();
		file.close();
	} // end if
} // end method storeResult

// store for letter pairs
void storeResult(char c_first, char c_second, int i_pairs)
{
	// Variables:
	fstream file;
	stringstream ss;
	

	file.open(tempFilePerms.c_str(), ios::out | ios::app);
	
	ss << c_first;
	ss << c_second;
	ss << " ";
	ss << i_pairs;
	ss << "\n";
	
	if (file.is_open())
	{
		file << ss.str();
		file.close();
	} // end if
} // end method storeResult

// retrieves results from temp for single letters
void writeResultToArray(int* result)
{ // a = 97
	// Variables:
	ifstream file;
	
	
	file.open(tempFile.c_str());
	
	if(file.is_open() && !file.bad())
	{
		// Variables:
		char c;
		int  i1;
		
		while(file >> c >> i1)
		{
			result[(int)c - 97] = i1;
		} // end while
	} // end if
	
	file.close();
} // end method writeResultToArray

// retrieves results from temp for letter pairs
void writeResultToArray(int** result)
{ // a = 97
	// Variables:
	ifstream file;
	
	
	file.open(tempFilePerms.c_str());
	
	if(file.is_open() && !file.bad())
	{
		// Variables:
		char c1;
		char c2;
		int  i1;
		
		while(file >> c1 >> c2 >> i1)
		{
			result[(int)c1 - 97][(int)c2 - 97] = i1;
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

