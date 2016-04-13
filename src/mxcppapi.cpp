/*
 * mxCPPAPI.cpp
 *
 *  Created on: Aug 12, 2015
 *      Author: zwh
 */
#define _MXCPPAPI_CPP_
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <cctype>
#include <sstream> 



extern "C" void mxClearHashMap(void);
extern "C" void AddCStrToHashMap(const char * cstr, int id);
extern "C" int  LookUpCStr(const char * cstr);


struct eqstr {
    inline size_t operator()(const char *s) const {
        size_t hash = 1;
        for (; *s; ++s) hash = hash * 5 + *s;
        return hash;
    }
    inline bool operator()(const char *s1, const char *s2) const {
        return strcmp(s1, s2) == 0;
    }
};

typedef std::unordered_map<const char*, int, eqstr, eqstr> strhash;

strhash mxHashMap;

// Clear Hash Map
extern "C" void mxClearHashMap(void)
{
	mxHashMap.clear();
}

// Add a string to table
extern "C" void AddCStrToHashMap(const char * cstr, int id)
{
	mxHashMap[cstr] = id;
}

// Look up if a string exist
extern "C" int LookUpCStr(const char * cstr)
{
	if(mxHashMap.find(cstr)!=mxHashMap.end())
		return  0; // exist
	else
		return -1; // not found
}



extern "C" int ReadValueFromConf(const char* fileName, const char *pConfName, char *pConfValue)
{
	std::ifstream infile(fileName);
	std::string line;
	if(NULL == fileName || NULL == pConfName)
	{	
		return -1;
	}
	std::string cName;
	cName = std::string(pConfName);
	while (std::getline(infile, line))
	{
		if(line.empty()) 
		{
			continue; // jump empty
		}
		line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // delete all space
		if('#' == line.at(0)) 
		{
			continue; // jump comments
		}
		std::string name;
		std::string value;

		std::size_t pos  = line.find('=');
		if(std::string::npos == pos)
		{
			//wrong format
			continue;
		}
		name = line.substr(0, pos);
		value = line.substr(pos+1);

		if(cName ==  name)
		{
			size_t cIndex = 0;
			for(cIndex= 0; cIndex<value.length(); cIndex++)
			{
				pConfValue[cIndex] = value.at(cIndex);
			}
			pConfValue[cIndex] = '\0';
		}
	}
	return 0;
}


extern "C" int FindSubStrInString(const char* str, const char *subStr)
{
	std::string longStr;
	std::string shortStr;
	longStr = std::string(str);
	shortStr = std::string(subStr);
	
	std::size_t found = longStr.find(shortStr);
	if(found!=std::string::npos)
	{
		return 0; //found
	}
	{
		return -1;
	}
}












#undef _MXCPPAPI_CPP_
