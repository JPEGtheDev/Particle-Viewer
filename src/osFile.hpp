//winFile.hpp
#ifndef osFile_HPP
	#define osFile_HPP
	
	#include <string>
	#include <iostream>

	#ifdef _WIN32
		#include <windows.h>
	#elif __APPLE__
		#include <limits.h>
		#include <unistd.h>	
	#elif __linux__
		#include <limits.h>
		#include <unistd.h>
	#endif
	static std::string ExePath() 
	{
		#ifdef _WIN32
	    	char buffer[MAX_PATH];
	    	GetModuleFileName( NULL, buffer, MAX_PATH );
	    	std::string::size_type pos = string( buffer ).find_last_of( "\\/" );
	    	return std::string( buffer ).substr( 0, pos);
	    #elif __APPLE__
			char result[ PATH_MAX ];
			ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
			return std::string( result, (count > 0) ? count : 0 );
	    #elif __linux__
			char result[ PATH_MAX ];
		 	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
		 	return std::string( result, (count > 0) ? count : 0 );
	    #endif
	}
	
#endif