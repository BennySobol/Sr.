#pragma once
#include "parametersHandler.h"
 
#define SHOW_HELP1 "-h"
#define SHOW_HELP2 "-H"
#define SHOW_MATCH1 "-v"
#define SHOW_MATCH2 "-V"
#define SORT_IMAGES1 "-s"
#define SORT_IMAGES2 "-S"
#define DOWN_SCALE1 "-d"
#define DOWN_SCALE2 "-D"
#define FOCAL_LEN1 "-f"
#define FOCAL_LEN2 "-F"
#define CALIB_DIR1 "-c"
#define CALIB_DIR2 "-C"

void ParametersHandler::printHelp()
{
	std::cout << "Usage: project.exe [-h] [-v] [-s] [-d down_scale] [-f focal_length] [-c calib_directory] input_directory" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "		-h                            (optional) Print help message" << std::endl;
	std::cout << "		-v                            (optional) Show match - default is unactive" << std::endl;
	std::cout << "		-s                            (optional) Sort input images - default is no sort" << std::endl;
	std::cout << "		-d down_scale				  (optional) Downscale factor for input images - default is 1 (decimal)" << std::endl;
	std::cout << "		-f focal_length               (option 1) Camera focal length (decimal)" << std::endl;
	std::cout << "		-c calib_directory			  (option 2) Directory to find chess board images for camera calibration" << std::endl;
	std::cout << "		input_directory               (option 3) Directory to find input images and calib.xml(optinal)" << std::endl;
	std::cout << "PLEASE NOTE THAT YOU MUST ENTER OR FOCAL LENGTH, OR calib_directory, OR HAVE calib.xml IN THE input_directory NATIVE!" << std::endl;
	
}

//checks all params
ParametersHandler::ParametersHandler(int argc, char* argv[]) : _showMatch(false), _isSorted(true), _focalLength(0), _downScaleFactor(1), _objectImagesPath(""), _chessboardImagesPath("")
{
	if (argc < 2)
	{
		printHelp();
		throw std::exception("Main param are invalid");
	}
	std::vector<std::string> paramList;
	for (int i = 0; i < argc; i++)
	{
		paramList.push_back(argv[i]);
	}

	std::vector<std::string>::iterator it;
	for (it = paramList.begin(); it != paramList.end(); it++)
	{

		if (*it == SHOW_HELP1 || *it == SHOW_HELP2)
		{
			printHelp();
		}
		else if( *it == SHOW_MATCH1 || *it == SHOW_MATCH2)
		{
			_showMatch = true;
		}
		else if(*it == SORT_IMAGES1 || *it == SORT_IMAGES2)
		{
			_isSorted = false;
		}
		else if(*it == DOWN_SCALE1|| *it == DOWN_SCALE2)
		{
			it++;
			if (it == paramList.end())
			{
				printHelp();
				throw std::exception("Main param are invalid");
			}
			_downScaleFactor = std::stod((*it), nullptr); // stod throw error if it fails to convert
			if(_downScaleFactor <= 1)
			{
				_downScaleFactor = 1;
			}
		}
		else if (*it == FOCAL_LEN1 || *it == FOCAL_LEN2)
		{
			it++;
			if (it == paramList.end())
			{
				printHelp();
				throw std::exception("Main param are invalid");
			}
			_focalLength = std::stod((*it), nullptr);
		}
		else if (*it == CALIB_DIR1 || *it == CALIB_DIR2)
		{
			it++;
			if (it == paramList.end())
			{
				printHelp();
				throw std::exception("Main param are invalid");
			}
            _chessboardImagesPath = (*it);
		}
		else
		{
			_objectImagesPath = (*it);
		}
	}
}
