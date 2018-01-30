#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <algorithm>

// Namespace:
using namespace std;

// Globals:
const int  i_LENGTH   = 26;
const char ca_CHARS[i_LENGTH] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
const string tempFile = "temp";

// Prototypes:
void countCharInFile(char, int*, fstream*);
void printStats(int*, string&);
void makeTempFile(void);
void storeResult(int);
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
		isFileValid(s_fileName);
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
	
	for(int i = 0; i < i_LENGTH; i++)
	{
		ia_results[i] = 0;
		
		pid_t pid = fork();
		
		if(pid == 0)
		{
			int temp = 0;
			file.open(s_fileName.c_str(), ios::in);
			makeTempFile();
			countCharInFile(ca_CHARS[i], &temp, &file);
			storeResult(temp);
			file.close();
			exit(EXIT_SUCCESS);
		} // end if
		else
		{
			wait(WAIT_MYPGRP);
			writeResultToArray(&ia_results[i]);
		} // end else
	} // end for
	
	printStats(ia_results, s_fileName);
	
	delete[] ia_results;
	remove(tempFile.c_str()); // delete temp file
	
	//cout << "Press enter to exit ..." << endl;
	//getline(cin, s_fileName);
	
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
			c = tolower(c); // make all chars lower case
			
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
			cout << "The character " << ca_CHARS[i] << " appears " << ia_results[i] << " times in the given file." << endl;
		} // end if
		else
		{
			cout << "The character " << ca_CHARS[i] << " did not appear in the given file." << endl;
		} // end else
	} // end for
} // end method printStats


void makeTempFile(void)
{
	remove(tempFile.c_str()); // delete any previous temp file
	fstream file(tempFile.c_str(), ios::in | ios::out | ios::trunc);
} // end method makeSortedFile


void storeResult(int result)
{
	// Variables:
	fstream file;
	stringstream ss;
	string s_out;
	
	
	ss << result;
	s_out = ss.str() + "\n";
	
	file.open(tempFile.c_str(), ios::out | ios::app);
	
	if (file.is_open())
	{
		file << s_out;
		file.close();
	} // end if
} // end method storeResult


void writeResultToArray(int* result)
{
	// Variables:
	ifstream file;
	
	
	file.open(tempFile.c_str());
	
	if(file.is_open())
	{
		file >> *result;
		
		file.close();
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
	
	
	file.open(s_fileName.c_str());
	
	if(!file.is_open() || file.bad())
	{
		cout << "\nFile could not be opened! Please check the filename and try again.\n\n";
		return false;
	} // end if
	else
	{
		file.close();
		return true;
	} // end else
} // end method isFileValid