#include "Splitter.h"

CSVSplitter::CSVSplitter()
{
	mArgc = 0;
	miInputCsvColumns = 0;
	miCSVFileCount = 1;
	miOutputCSVColumnCount = 1;
	miLineCount = 0;
	mbArgument = mbHeader = mbValidation = true;

	memset(mCsvInputFile, '\0', sizeof(mCsvInputFile));
	memset(mCsvOutPath, '\0', sizeof(mCsvOutPath));
	memset(mDrvName, '\0', sizeof(mDrvName));
	memset(mDir, '\0', sizeof(mDir));
	memset(mFileName, '\0', sizeof(mFileName));
	memset(mExt, '\0', sizeof(mExt));
}

CSVSplitter::CSVSplitter(int argc, char* argv[])
{
	mArgc = argc;
	miInputCsvColumns = 0;
	miCSVFileCount = 1;
	miOutputCSVColumnCount = 1;
	miLineCount = 0;
	mbArgument = mbHeader = mbValidation = true;

	memset(mCsvInputFile, '\0', sizeof(mCsvInputFile));
	memset(mCsvOutPath, '\0', sizeof(mCsvOutPath));
	memset(mDrvName, '\0', sizeof(mDrvName));
	memset(mDir, '\0', sizeof(mDir));
	memset(mFileName, '\0', sizeof(mFileName));
	memset(mExt, '\0', sizeof(mExt));

	for (int iCounter = 0; iCounter < mArgc; iCounter++)
		mArgv.push_back(argv[iCounter]);
}

CSVSplitter::~CSVSplitter()
{
	mArgc = 0;
	miInputCsvColumns = 0;
	miCSVFileCount = 0;
	miOutputCSVColumnCount = 0;
	miLineCount = 0;
	mbArgument = mbHeader = mbValidation = false;

	memset(mCsvInputFile, '\0', sizeof(mCsvInputFile));
	memset(mCsvOutPath, '\0', sizeof(mCsvOutPath));
	memset(mDrvName, '\0', sizeof(mDrvName));
	memset(mDir, '\0', sizeof(mDir));
	memset(mFileName, '\0', sizeof(mFileName));
	memset(mExt, '\0', sizeof(mExt));
	mArgv.clear();
}

void CSVSplitter::DisplayToolUsage(void)
{
	printf("\n"
		"=========================================================================================================\n"
		" USAGE:\n"
		" 1 : CSVSplitter.exe \n"
		" 2 : Stream Data generated file path in the csv format as Input \n"
		" 3 : CSV Split Column value \n"
		" 4 : Splitted CSV file Validation Argument. 1 : Validation. 0 : No Validation. Defualt 1 : Do Validation\n"
		"=========================================================================================================\n"
	);
}

int CSVSplitter::ReadArguments()
{
	int iRetCode = ERROR_SPLIT_SUCCESS;
	int iCount = 1;
	char cTempBuffer[1000] = "\0";

	switch (mArgc)
	{
	case 3:
		memset(cTempBuffer, 0, sizeof(cTempBuffer));
		// Extracting Input CSV path
		strncpy(mCsvInputFile, mArgv.at(iCount).c_str(), _MAX_PATH - 1);
		iCount++;

		// Extracting CSV Split Column value
		memset(cTempBuffer, 0, sizeof(cTempBuffer));
		strncpy(cTempBuffer, mArgv.at(iCount).c_str(), sizeof(cTempBuffer) - 1);
		miOutputCSVColumnCount = std::atoi(cTempBuffer);
		break;

	case 4:
		memset(cTempBuffer, 0, sizeof(cTempBuffer));
		// Extracting Input CSV path
		strncpy(mCsvInputFile, mArgv.at(iCount).c_str(), _MAX_PATH - 1);
		++iCount;

		// Extracting CSV Split Column value
		memset(cTempBuffer, 0, sizeof(cTempBuffer));
		strncpy(cTempBuffer, mArgv.at(iCount).c_str(), sizeof(cTempBuffer) - 1);
		miOutputCSVColumnCount = std::atoi(cTempBuffer);

		++iCount;
		// Extracting Validation input. Default True(1)
		memset(cTempBuffer, 0, sizeof(cTempBuffer));
		strncpy(cTempBuffer, mArgv.at(iCount).c_str(), sizeof(cTempBuffer) - 1);
		mbValidation = (std::atoi(cTempBuffer) > 0) ? true : false;
		break;

	case 0:
	case 1:
	case 2:
	default:
		DisplayToolUsage();
		iRetCode = ERROR_INVALID_ARGUMENTS;
		break;
	}

	return iRetCode;
}

void CSVSplitter::ReadCSVHeaderCount(void)
{
	std::string strWord;
	std::ifstream inputCsvFile(mCsvInputFile);
	std::string strLine;

	if (inputCsvFile.is_open())
	{
		std::getline(inputCsvFile, strLine);
		std::stringstream str(strLine);

		while (std::getline(str, strWord, COLUMN_SEPERATOR))
			miInputCsvColumns++;

		inputCsvFile.close();
	}
}

int CSVSplitter::WriteCSVFile(void)
{
	int iRetCode = ERROR_SPLIT_SUCCESS;
	bool bDirCreated = false;
	char csvDirName[_MAX_PATH] = "\0";
	std::string strLine;
	std::string strWord;
	std::vector<std::ofstream> outCSVFile;
	std::ifstream inputCsvFile(mCsvInputFile);

	// Creating multiple CSV files for writting

	if (miInputCsvColumns < 9)
		miCSVFileCount = 1;
	else if (0 == miInputCsvColumns % miOutputCSVColumnCount)
		miCSVFileCount = miInputCsvColumns / miOutputCSVColumnCount;
	else
		miCSVFileCount += (miInputCsvColumns / miOutputCSVColumnCount);

	printf("Input CSV File : %s : Split Count : %d : Splitting : Started\r", mFileName, miCSVFileCount);

	for (int iCounter = 1; iCounter <= miCSVFileCount; iCounter++)
	{
		char csvFileName[_MAX_PATH] = "\0";
		char csTemp[10] = "\0";
		snprintf(csTemp, sizeof(csTemp), "%d", iCounter);
		strncat(csvFileName, mDrvName, sizeof(csvFileName) - strlen(mDrvName) - 1);
		strncat(csvFileName, mDir, sizeof(csvFileName) - strlen(mDir) - 1);
		strncat(csvFileName, mFileName, sizeof(csvFileName) - strlen(mFileName) - 1);
		strncat(csvFileName, OUT_DIRECTORY, sizeof(csvFileName) - strlen(OUT_DIRECTORY) - 1);
		strncpy(csvDirName, csvFileName, _MAX_PATH - 1); csvDirName[_MAX_PATH - 1] = '\0';
		strncat(csvFileName, mFileName, sizeof(csvFileName) - strlen(mFileName) - 1);
		strncat(csvFileName, "_", sizeof(csvFileName) - strlen("_") - 1);
		strncat(csvFileName, csTemp, sizeof(csvFileName) - strlen(csTemp) - 1);
		strncat(csvFileName, INPUT_FILE_EXTENSION, sizeof(csvFileName) - strlen(INPUT_FILE_EXTENSION) - 1);

		if (!bDirCreated)
		{
#if defined(_WIN_PLATFORM)
			if ((-1 == _mkdir(csvDirName)) && (EEXIST != errno))
				printf(" Error in Output directory creation.Exiting CSV Creation\n");
#elif defined(__GNUC__)
			if (-1 == mkdir(csvDirName, 0777))
				printf(" Error in Output directory creation.Exiting CSV Creation\n");
#endif
			bDirCreated = true;
		}

		std::ofstream outCSV(csvFileName);

		if (outCSV.is_open())
			outCSVFile.push_back(std::ofstream(csvFileName, std::ios::out));
		else
		{
			printf("Input CSV File : %s : Split Count : %d : Splitting : Error in %s Out CSV file creating\r", mFileName, miCSVFileCount, csvFileName);
			iRetCode = ERROR_OUTPUT_CSV_CREATION;

			// Closing the created the output CSV files
			for (unsigned int iTempCounter = 0; iTempCounter < outCSVFile.size(); iTempCounter++)
				outCSVFile.at(iTempCounter).close();

			inputCsvFile.close();

			return iRetCode;
		}
	}

	if (inputCsvFile.is_open())
	{
		printf("Input CSV File : %s : Split Count : %d : Splitting : Processing\r", mFileName, miCSVFileCount);

		while (std::getline(inputCsvFile, strLine))
		{
			int iFileIndex = 0;
			int iCounter = 0;
			std::stringstream str(strLine);
			while (std::getline(str, strWord, COLUMN_SEPERATOR))
			{
				iFileIndex = iCounter / miOutputCSVColumnCount;
				// Write the word into the respective CSV file 
				outCSVFile.at(iFileIndex) << strWord << ",";

				iCounter++;
			}

			iFileIndex = 0;
			for (unsigned int iTempCounter = 0; iTempCounter < outCSVFile.size(); iTempCounter++)
			{
				outCSVFile.at(iTempCounter) << std::endl;
				outCSVFile.at(iTempCounter).flush();
			}

			miLineCount++;
		}

		inputCsvFile.close();

		// Closing the created the output CSV files
		for (unsigned int iTempCounter = 0; iTempCounter < outCSVFile.size(); iTempCounter++)
		{
			outCSVFile.at(iTempCounter).flush();
			outCSVFile.at(iTempCounter).close();
		}
		printf("Input CSV File : %s : Split Count : %d : Splitting : Completed     \n", mFileName, miCSVFileCount);
	}
	else
	{
		iRetCode = ERROR_INPUT_CSV_OPEN;
		printf("Input CSV File : %s : Split Count : %d : Splitting : Error in opening Input CSV file\n", mFileName, miCSVFileCount);
	}

	return iRetCode;
}

int CSVSplitter::ValidateSplitCSVData()
{
	int iRetCode = ERROR_SPLIT_SUCCESS;
	std::vector<std::string> vCSVFileNames;		// Vector to store the Split CSV file Names
	std::vector<std::ifstream> iSplitCSVFile;	// Vector to store the Split CSV file open handle

	for (int iCounter = 1; iCounter <= miCSVFileCount; iCounter++)
	{
		char csvFileName[_MAX_PATH] = "\0";
		char csTemp[10] = "\0";
		char csData[10] = "Data";
		snprintf(csTemp, sizeof(csTemp), "%d", iCounter);
		strncat(csvFileName, mDrvName, sizeof(csvFileName) - strlen(mDrvName) - 1);
		strncat(csvFileName, mDir, sizeof(csvFileName) - strlen(mDir) - 1);
		strncat(csvFileName, mFileName, sizeof(csvFileName) - strlen(mFileName) - 1);
		strncat(csvFileName, OUT_DIRECTORY, sizeof(csvFileName) - strlen(OUT_DIRECTORY) - 1);
		strncat(csvFileName, mFileName, sizeof(csvFileName) - strlen(mFileName) - 1);
		strncat(csvFileName, "_", sizeof(csvFileName) - strlen("_") - 1);
		strncat(csvFileName, csTemp, sizeof(csvFileName) - strlen(csTemp) - 1);
		strncat(csvFileName, INPUT_FILE_EXTENSION, sizeof(csvFileName) - strlen(INPUT_FILE_EXTENSION) - 1);

		std::ifstream outCSV(csvFileName);
		vCSVFileNames.push_back(csvFileName);

		if (outCSV.is_open())
			iSplitCSVFile.push_back(std::ifstream(csvFileName, std::ios::in));
		else
		{
			printf("Input CSV File : %s : Split Count : %d : CSV Validation : Error in %s Out CSV file is reading\r", mFileName, miCSVFileCount, csvFileName);
			iRetCode = ERROR_CSV_READ;

			// Closing the created the output CSV files
			for (unsigned int iTempCounter = 0; iTempCounter < iSplitCSVFile.size(); iTempCounter++)
				iSplitCSVFile.at(iTempCounter).close();

			return iRetCode;
		}
	}

	char cErrorFile[_MAX_PATH] = "\0";
	strncat(cErrorFile, mDrvName, sizeof(cErrorFile) - strlen(mDrvName) - 1);
	strncat(cErrorFile, mDir, sizeof(cErrorFile) - strlen(mDir) - 1);
	strncat(cErrorFile, mFileName, sizeof(cErrorFile) - strlen(mFileName) - 1);
	strncat(cErrorFile, OUT_DIRECTORY, sizeof(cErrorFile) - strlen(OUT_DIRECTORY) - 1);
	strncat(cErrorFile, mFileName, sizeof(cErrorFile) - strlen(mFileName) - 1);
	strncat(cErrorFile, "_", sizeof(cErrorFile) - strlen("_") - 1);
	strncat(cErrorFile, ERROR_FILE, sizeof(cErrorFile) - strlen(ERROR_FILE) - 1);
	std::ofstream errorFile(cErrorFile, std::ofstream::out | std::ofstream::trunc);

	// Reading the Input CSV file
	long unsigned int iLineCount = 0;
	std::ifstream inputCsvFile(mCsvInputFile);
	std::vector<std::vector<std::string>> vErrorLog(miCSVFileCount);

	if (inputCsvFile.is_open())
	{
		std::string strInputCSVLine;
		std::string strInputCSVWord;
		long unsigned int iWordCount = 0;

		while (std::getline(inputCsvFile, strInputCSVLine))
		{
			std::stringstream sInputCSVStream(strInputCSVLine);
			std::vector<std::string> vWord;

			while (std::getline(sInputCSVStream, strInputCSVWord, COLUMN_SEPERATOR))
				vWord.push_back(strInputCSVWord);

			iWordCount = 0;

			for (unsigned long int iCounter = 0; iCounter < (unsigned long int)miCSVFileCount; iCounter++)
			{
				std::string strCSVLine;
				std::string strCSVWord;
				bool bValidationStatus = true;

				while (std::getline(iSplitCSVFile[iCounter], strCSVLine))
				{
					std::stringstream sCSVStream(strCSVLine);
					std::vector<std::string> vTemp;

					while (std::getline(sCSVStream, strCSVWord, COLUMN_SEPERATOR))
						vTemp.push_back(strCSVWord);

					for (unsigned long int iTemp = 0; iTemp < (unsigned long int)vTemp.size(); iTemp++)
					{
						if (0 != vWord[iWordCount].compare(vTemp[iTemp]))
						{
							char cTempBuffer[300] = "\0";
							memset(cTempBuffer, 0, sizeof(cTempBuffer));
							snprintf(cTempBuffer, sizeof(cTempBuffer), "Line Number, %d, Cell Number :, %d, :  Original Value :,%s, Split Value:,%s\n", iLineCount, iWordCount, vWord[iWordCount].c_str(), vTemp[iTemp].c_str());
							vErrorLog.at(iCounter).push_back(cTempBuffer);
							bValidationStatus = false;
							iRetCode = ERROR_VALIDATION_FAILED;
						}
						iWordCount++;
					}

					vTemp.clear();

					if ((iWordCount == vWord.size()) || (iWordCount == miInputCsvColumns) || (0 == (iWordCount % miOutputCSVColumnCount)))
						break;
				}
			}

			vWord.clear();
			iWordCount = 0;
			iLineCount++;
		}
	}
	else
	{
		printf("Input CSV File : %s : Split Count : %d : CSV Validation : Error in opeing the Input CSV file\r", mFileName, miCSVFileCount);
		iRetCode = ERROR_WRITE_CSV_FAILED;
	}

	// Closing all open files
	if (inputCsvFile.is_open())
		inputCsvFile.close();

	for (int iCounter = 0; iCounter < miCSVFileCount; iCounter++)
		if (iSplitCSVFile.at(iCounter).is_open())
			iSplitCSVFile.at(iCounter).close();

	iSplitCSVFile.clear();

	// Writting in Error Log file
	if (errorFile.is_open())
	{
		for (unsigned long int iCounter = 0; iCounter < (unsigned long int)vErrorLog.size(); iCounter++)
		{
			if (vErrorLog[iCounter].size() > 0)
			{
				errorFile << vCSVFileNames[iCounter] << " : Validation Failed. " << std::endl;
				for (auto iterator = vErrorLog[iCounter].begin(); iterator != vErrorLog[iCounter].end(); iterator++)
					errorFile << *iterator << std::endl;
			}
			else
			{
				errorFile << vCSVFileNames[iCounter] << " : Validation Passed. " << std::endl;
			}
		}

		errorFile.flush();
		errorFile.close();
	}

	return iRetCode;
}

int CSVSplitter::SplitCSV()
{
	int iRetCode = ERROR_SPLIT_SUCCESS;

	if (mbArgument)
	{
		if (ERROR_SPLIT_SUCCESS != ReadArguments())
		{
			iRetCode = ERROR_INVALID_ARGUMENTS;
			printf("Input CSV File : %s : Split Count : %d : Splitting : Reading Input Arguments failed\r", mCsvInputFile, miCSVFileCount);
		}
	}
#if defined(_WIN_PLATFORM)
	if (ERROR_SPLIT_SUCCESS == split_path(mCsvInputFile, mDrvName, mDir, mFileName, mExt))
	{
		if (ERROR_SPLIT_SUCCESS == strcmp(mExt, INPUT_FILE_EXTENSION))
		{
			if (ERROR_SPLIT_SUCCESS == WriteCSVFile())
			{
				if (mbValidation)
				{
					printf("Input CSV File : %s : Split Count : %d : CSV Validation : Started\r", mFileName, miCSVFileCount);

					if (ERROR_SPLIT_SUCCESS == ValidateSplitCSVData())
						printf("Input CSV File : %s : Split Count : %d : CSV Validation : Pass\r", mFileName, miCSVFileCount);
					else
						printf("Input CSV File : %s : Split Count : %d : CSV Validation : Fail.check the Output Error file\r", mFileName, miCSVFileCount);
				}
			}
			else
			{
				iRetCode = ERROR_WRITE_CSV_FAILED;
				printf("Input CSV File : %s : Split Count : %d : Splitting : Output CSV creation failed\r", mFileName, miCSVFileCount);
			}
		}
	}
	else
	{
		iRetCode = ERROR_INPUT_CSV_PATH;
		printf(" Input CSV File : %s : Split Count : %d : Splitting : Extracting Input CSV Path failed\r", mFileName, miCSVFileCount);
	}
#elif defined(__GNUC__)
    char** Ptr_mDrvName = (char**)malloc(sizeof(char*));
	*Ptr_mDrvName = mDrvName;
	char** Ptr_mDir = (char**)malloc(sizeof(char*));
	*Ptr_mDir = mDir;
	char** Ptr_mFileName = (char**)malloc(sizeof(char*));
	*Ptr_mFileName = mFileName;
	char** Ptr_mExt = (char**)malloc(sizeof(char*));
	*Ptr_mExt = mExt;	
	if (ERROR_SPLIT_SUCCESS == split_pathLinux(mCsvInputFile, Ptr_mDrvName, Ptr_mDir, Ptr_mFileName, Ptr_mExt))
	{
		if (ERROR_SPLIT_SUCCESS == strcmp(mExt, INPUT_FILE_EXTENSION))
		{
			if (ERROR_SPLIT_SUCCESS == WriteCSVFile())
			{
				if (mbValidation)
				{
					printf("Input CSV File : %s : Split Count : %d : CSV Validation : Started\r", mFileName, miCSVFileCount);

					if (ERROR_SPLIT_SUCCESS == ValidateSplitCSVData())
						printf("Input CSV File : %s : Split Count : %d : CSV Validation : Pass\r", mFileName, miCSVFileCount);
					else
						printf("Input CSV File : %s : Split Count : %d : CSV Validation : Fail.check the Output Error file\r", mFileName, miCSVFileCount);
				}
			}
			else
			{
				iRetCode = ERROR_WRITE_CSV_FAILED;
				printf("Input CSV File : %s : Split Count : %d : Splitting : Output CSV creation failed\r", mFileName, miCSVFileCount);
			}
		}
	}
	else
	{
		iRetCode = ERROR_INPUT_CSV_PATH;
		printf(" Input CSV File : %s : Split Count : %d : Splitting : Extracting Input CSV Path failed\r", mFileName, miCSVFileCount);
	}	
 #endif
	return iRetCode;
}

int CSVSplitter::getCSVColumnCount(void)
{
	return miInputCsvColumns;
}

void CSVSplitter::SetCSVData(bool bHeader, bool bArgument, bool bValidation, std::string csvPath)
{
	mbValidation = bValidation;
	mbArgument = bArgument;
	mbHeader = bHeader;
	miOutputCSVColumnCount = MAX_EXCEL_CSV_COLUMN_COUNT;
	memset(mCsvInputFile, '\0', MAX_PATH);
	strncpy(mCsvInputFile, csvPath.c_str(), sizeof(mCsvInputFile) - 1); mCsvInputFile[sizeof(mCsvInputFile) - 1] = '\0';
}

int CSV_file_exist(const char* _Filename, int _AccessMode)
{
	if (strlen(_Filename) != 0)
	{
#if defined(_WIN_PLATFORM)
		if (access(_Filename, 0) != 0)
			return -1;
#elif defined (__GNUC__)
		if (access(_Filename, 0) != 0)
			return -1;
#endif
		FILE* fp = NULL;
		if ((fp = fopen(_Filename, "r")) == 0)
			return -1;
		else
			fclose(fp); /* Make sure to close it */
	}

	return 0;
}