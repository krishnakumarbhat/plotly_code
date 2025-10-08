#ifndef _SPLITTER_H__

#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include "genutil.h"

#if defined(_WIN_PLATFORM)
#include "Windows.h"
#include <direct.h>
#include <filesystem>
#include "corecrt_io.h"
#elif defined(__GNUC__)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#endif

#define INPUT_FILE_EXTENSION ".csv"
#define ERROR_FILE "Error.txt"
#define COLUMN_SEPERATOR ','
#define MAX_EXCEL_CSV_COLUMN_COUNT 16383
#if defined(_WIN_PLATFORM)
#define OUT_DIRECTORY "_output\\"
#elif defined(__GNUC__)
#define OUT_DIRECTORY "_output/"
#endif

enum ERROR_CODES
{
	ERROR_SPLIT_SUCCESS = 0,
	ERROR_INVALID_ARGUMENTS = -1,
	ERROR_INPUT_CSV_PATH = -2,
	ERROR_INVALID_INPUR_FILE_EXTENSION = -3,
	ERROR_INPUT_CSV_COLUMN_VALUE = -4,
	ERROR_INPUT_CSV_COLUMN_COUNT_LESS = -5,
	ERROR_WRITE_CSV_FAILED = -6,
	ERROR_INPUT_CSV_OPEN = -7,
	ERROR_OUTPUT_CSV_CREATION = -8,
	ERROR_CSV_READ = -9,
	ERROR_VALIDATION_FAILED = -10,
	ERROR_MAX_CSV_COLUMN_COUNT = -11
};

struct csvDetails
{
	Customer_T iCustomerID;
	unsigned int iStreamNum;
	std::string streamName;

public:
	csvDetails()
	{
		iCustomerID = Customer_T::INVALID_CUSTOMER;
		iStreamNum = -1;
		streamName = "";
	}
	~csvDetails()
	{
		iCustomerID = Customer_T::INVALID_CUSTOMER;
		iStreamNum = -1;
		//iStreamVersion = -1;
		streamName = "";
	}
};

class CSVSplitter
{
private:
	int mArgc;						 // Command Line Arument Count
	int miInputCsvColumns;			 // No of columns present in the input CSV
	int miCSVFileCount;				 // No of out CSV file required 
	int miOutputCSVColumnCount;		// This is given as input paramater
	int miLineCount;				// stores the line in the CSV file
	bool mbValidation;				// Do validation on output need to pbe carried out. I/P from command line argument. By default True(1)
	bool mbHeader;				// Whether to read CSV Header or not
	bool mbArgument;			// Whether to read Argument or not

	char mCsvInputFile[_MAX_PATH];	// Input CSV file Path
	char mCsvOutPath[_MAX_PATH];	// Output CSV Path
	char mDrvName[_MAX_DRIVE];		// Drive Name
	char mDir[_MAX_DIR];			// Directory Name
	char mFileName[_MAX_FNAME];		// File Name
	char mExt[_MAX_EXT];			// File Extension
	std::vector<std::string> mArgv;				// Command line Argument List

	void DisplayToolUsage(void);
	int ValidateSplitCSVData();
	int WriteCSVFile(void);
	int ReadArguments();

public:
	CSVSplitter();
	CSVSplitter(int argc, char* argv[]);
	~CSVSplitter();
	int SplitCSV();
	void ReadCSVHeaderCount(void);
	int getCSVColumnCount(void);
	void SetCSVData(bool Header, bool Argument, bool bValidation, std::string csvPath);
};

int CSV_file_exist(const char* _Filename, int _AccessMode);

#define _SPLITTER_H__
#endif //_SPLITTER_H__