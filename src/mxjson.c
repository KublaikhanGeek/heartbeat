#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <string.h>
#include "cjson.h"
#include "mxlib.h"
#include "mxcppapi.h"
#include "md5.h"


#ifndef MAXNAME
#define MAXNAME 128
#endif

#ifndef MAX_CHAR_ONE_LINE
#define MAX_CHAR_ONE_LINE 1024
#endif

char g_vFilePath[MAXNAME];
char g_idFilePath[MAXNAME];
char g_UrlFilePath[MAXNAME];
char g_DelFilePath[MAXNAME];
char g_KeepFilePath[MAXNAME];
char g_SSDALLFilePath[MAXNAME];
char g_bfTargetPath[MAXNAME];
char g_extDir[MAXNAME];
char g_ExtUrlFilePath[MAXNAME];
char g_ExtMoveFilePath[MAXNAME];




long getFileSize(const char * strFileName);
int JsonPrepare(const char *workDir, const char *bfDir, const char *extDir);
int GenerateAllFilesList(char *bfDir);
int AnalyzeExtAndResPart(cJSON *json);
unsigned long GetFileLineNumber(char * path);
int GenerateDeleteFile(void );
int ParseResPartJson(cJSON *json,  FILE * urlfile,  FILE * keepfile);
int ParseExtPartJson(cJSON *json, FILE * extUrlfile, FILE *extMovefile, FILE * keepfile );
int WriteURLListLine(FILE * urlFile, char * line);
int CheckIfFileExist(char * fullpath);
int WriteKeepListLine(FILE * keepfile, char * line);
int GetFullURL(cJSON *json, char* urlPart, char * fullURL);
int GetResFullPath(char * resString, char * resStringValue, char *fullPath);
int GetExtFullPath(char * input, char *fullPath);
int GetMD5FromJsonPart(const char * input, char * md5);
int WriteVfile(cJSON *json);
int WriteIDfile(cJSON *json);
int WriteVfile(cJSON *json);
int ReadJsonToMem(const char *pathName, char * mem);
int cjsonfile(const char *xmlpath);
int isInExtTempDir(char *md5);
int AddExtToDownloadListFile(FILE *extUrlfile, char *URL, char *md5,char *targetFullPath);
int isInExtTargetDir(char *targetFullPath, char *md5);
int AddExtToMoveListFile(FILE *extMoveFile, char *md5, char *targetFullPath);





long getFileSize(const char * strFileName)    
{   
    if(NULL == strFileName)
    {
        mxLog("File Not Exist");
        return -1;
    }
    FILE * fp = fopen(strFileName, "r");   
    if(NULL == fp)
    {
        mxLog("File Open Failed");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);   
    long size = ftell(fp);   
    fclose(fp);   
 //   mxLog("getFileSize=%ld", size);
    return size;   
}   

int JsonPrepare(const char *workDir, const char *bfDir, const char *extDir)
{
	// 4 temp files path: vfile idfile resfile extfile
	strcat(g_vFilePath, workDir);
	strcat(g_vFilePath, "vfile");

	strcat(g_idFilePath, workDir);
	strcat(g_idFilePath, "idfile");

	// URLList.file DeleteList.file
	strcat(g_UrlFilePath, workDir);
	strcat(g_UrlFilePath, "URLList.file");

	strcat(g_DelFilePath, workDir);
	strcat(g_DelFilePath, "DeleteList.file");

        //extURLList.file     extMoveList.file
        strcat(g_ExtUrlFilePath, workDir);
	strcat(g_ExtUrlFilePath, "extURLList.file");

        strcat(g_ExtMoveFilePath, workDir);
	strcat(g_ExtMoveFilePath, "extMoveList.file");

	// SSDAllFile, KeepList
	strcat(g_KeepFilePath, workDir);
	strcat(g_KeepFilePath, "keepList");

	strcat(g_SSDALLFilePath, workDir);
	strcat(g_SSDALLFilePath, "ssdAllList");

	strcat(g_bfTargetPath, bfDir);

    strcat(g_extDir, extDir);
	GenerateAllFilesList(g_bfTargetPath);
	return 0;
}


int cjsonfile(const char *xmlpath)
{
	
	cJSON * json;                    //, *id, *version, *prefix, *ext, *res;
	char  * pJsonContent  = NULL;
	char buf[1024]       = {0};
    
	memset(buf, 0, sizeof(buf));

	long jsonSize = getFileSize(xmlpath) + 10;//for safe
	pJsonContent  = (char *)malloc(jsonSize);
	if(NULL == pJsonContent)
	{
		// LOG: No memory
		mxLog("pJsonContent is NULL");
		return -1;
	}
	memset(pJsonContent, 0, jsonSize);
	
	
	// read Json file content to pJsonContent
	ReadJsonToMem(xmlpath, pJsonContent);

	//Parse Json
	if( !(json = cJSON_Parse(pJsonContent)))
	{
		free(pJsonContent);
		mxLog("parse json failed");
		return -1;
	}
	else
	{
		WriteIDfile(json);
		WriteVfile(json);
		AnalyzeExtAndResPart(json);
	}
	
	cJSON_Delete(json);
	free(pJsonContent);
	return 0;
}

int ReadJsonToMem(const char *pathName, char * mem)
{
	FILE *xmlfile = NULL;
    
    if(NULL == pathName || NULL ==mem)
    {
        mxLog("NULL pointer");
        return -1;
    }
    
	if((xmlfile = fopen(pathName, "r")) == NULL)
	{
		mxLog("open xmlfile failed");
		return -1;
	}
	
	char * pPosition = mem;
	long jsonSize = getFileSize(pathName) + 1;//for safe
	while((fgets(pPosition, jsonSize, xmlfile)) != NULL)
	{
		long readBytes = strlen(pPosition);
		pPosition += readBytes;
	}
	fclose(xmlfile);
	return 0;
}

int WriteIDfile(cJSON *json)
{
	FILE *idfile = NULL;
    if(NULL == json )
    {
        mxLog("NULL pointer");
        return -1;
    }
	if((idfile = fopen(g_idFilePath, "w+")) == NULL)
	{
		mxLog("open idfile failed");
		return -1;
	}
	cJSON * id = cJSON_GetObjectItem(json, "id");
    if(NULL == id)
    {
        mxLog("NULL pointer");
        return -1;
    }
	if(id->type == cJSON_String)
	{
		fputs(id->valuestring, idfile);
	}
	else
	{
		mxLog("no id this word");
		return -1;
	}
	
	fflush(idfile);
	fclose(idfile);
	return 0;
}

int WriteVfile(cJSON *json)
{
	FILE * vfile = NULL;
    if(NULL == json )
    {
        mxLog("NULL pointer");
        return -1;
    }
	if((vfile = fopen(g_vFilePath, "w+")) == NULL)
	{
		mxLog("open vfile failed");
		return -1;
	}
	cJSON * version = cJSON_GetObjectItem(json, "v");
    if(NULL == version)
    {
        mxLog("NULL pointer");
        return -1;
    }
	if( version->type == cJSON_String)
	{
		fputs(version->valuestring, vfile);
	}
	else
	{	
		mxLog("no v this word");
		return -1;
	}
	fflush(vfile);
	fclose(vfile);
	
	return 0;
}

// Note: for Res, please give ext->string as input
int GetResFullPath(char * resString, char * resStringValue, char *fullPath)
{
    if(NULL == resString || NULL == resStringValue || NULL == fullPath)
    {
        mxLog("NULL pointer");
        return -1;
    }
	char md5name[33] = {0};
	strcpy(fullPath, g_bfTargetPath);
	strcat(fullPath, resStringValue);
	strcat(fullPath, "/");
	GetMD5FromJsonPart(resString, md5name);
	strcat(fullPath, md5name);
   // mxLog("g_bfTargetPath=%s, resStringValue=%s, md5name=%s, fullPath=%s", g_bfTargetPath, resStringValue, md5name, fullPath);
	return 0;
}

// Note: for Ext,please give ext->valuestring as input
int GetExtFullPath(char * input, char *fullPath)
{
    if(NULL == input || NULL == fullPath)
    {
        mxLog("NULL pointer");
        return -1;
    }
	strcpy(fullPath, g_bfTargetPath);
	strcat(fullPath, input);
	return 0;
}

int GetMD5FromJsonPart(const char * input, char * md5)
{
	//"3f\/ef\/3f4a35b6f3f71a46411cceadaf5038ef"
	char * str = malloc(strlen(input) + 1);
    if( NULL == str || NULL == md5 )
    {
        return -1;
    }
    strcpy (str, input);
	char * token = strtok(str,"/");
	char * lastPart = token;
	while(token !=NULL)
	{
		lastPart = token;
		token = strtok(NULL, "/");
	}
    if(NULL == lastPart)
    {
        mxLog("NULL pointer, empty Input");
        return -1;
    }

	strcpy(md5, lastPart);
	
	return 0;
}

int GetFullURL(cJSON *json, char* urlPart, char * fullURL)
{
    if(NULL == json || NULL == urlPart || NULL == fullURL)
    {
        mxLog("NULL pointer, empty Input");
        return -1;
    }
	static cJSON * url_prefix = NULL;
	if(NULL == url_prefix)
	{
		url_prefix = cJSON_GetObjectItem(json, "prefix");
        if(NULL == url_prefix)
        {
            mxLog("NULL pointer");
            return -1;
        }
	}
    
    if( NULL != url_prefix && url_prefix->type == cJSON_String)
    {
        strcpy(fullURL, url_prefix->valuestring);
	    strcat(fullURL, urlPart);
    }
    else
    {   
        mxLog("no prefix in Json");
        return -1;
    }
    return 0;
	
}

int CheckIfFileExist(char * fullpath)
{
    if(NULL == fullpath)
    {
        mxLog("NULL pointer");
        return -1;
    }
	return access(fullpath, F_OK);
}


int WriteKeepListLine(FILE * keepfile, char * line)
{
    if(NULL == keepfile || NULL == line)
    {
        mxLog("NULL pointer");
        return -1;
    }
	fputs(line, keepfile);
    return 0;
}

int WriteURLListLine(FILE * urlFile, char * line)
{
    if(NULL == urlFile || NULL == line)
    {
        mxLog("NULL pointer");
        return -1;
    }
	fputs(line, urlFile);
    return 0;
}

int isInExtTargetDir(char *targetFullPath, char *md5)
{
    char md5Char[33] = {0};
    
    memset(md5Char, 0, sizeof(md5Char));
    //access can't include '\n'
    targetFullPath[strlen(targetFullPath) - 1] = '\0';
    if(0 == access(targetFullPath, F_OK))
    {
        CalcFileMD5(targetFullPath, md5Char);
        mxLog("md5 in json:%s, %s's md5:%s", md5, targetFullPath, md5Char);
        strcat(targetFullPath, "\n");
        return strcmp(md5, md5Char);
    }
    strcat(targetFullPath, "\n");
    return 1;
}

int AddExtToMoveListFile(FILE *extMoveFile, char *md5, char *targetFullPath)
{
    //Add extMove list
    char oneLine[MAX_CHAR_ONE_LINE] = {0};
    
    strcat(oneLine, md5);
    strcat(oneLine, "\t");
    strcat(oneLine, targetFullPath);
    fputs(oneLine, extMoveFile);
    return 0;
}


int isInExtTempDir(char *md5)
{
    char fullPath[MAXNAME] = {0};
    
    strcat(fullPath, g_extDir);
    strcat(fullPath, md5);
    //if the file renamed,need access to deal with
    if(0 == access(fullPath, F_OK))
    {
        mxLog("%s is exist at extDir,no need download", fullPath);
        return 0;
    }
    return 1;
}

int AddExtToDownloadListFile(FILE *extUrlfile, char *URL, char *md5,char *targetFullPath)
{
    char oneLine[MAX_CHAR_ONE_LINE] = {0};
    
    strcat(oneLine, URL);
    strcat(oneLine, "\t");
    strcat(oneLine, md5);
    strcat(oneLine, "\t");
    strcat(oneLine, targetFullPath);
    fputs(oneLine, extUrlfile);
    return 0;
}


int ParseExtPartJson(cJSON *json, FILE * extUrlfile, FILE *extMovefile, FILE * keepfile )
{
	char targetFullPath[MAXNAME]    = {0};
	char oneLine[MAX_CHAR_ONE_LINE] = {0};
    char md5[33]                    = {0};
    
	if(NULL == json || NULL == extUrlfile || NULL == keepfile)
    {
        mxLog("NULL pointer");
        return -1;
    }
	// Get ext content
	cJSON * ext = cJSON_GetObjectItem(json, "ext");
    if(NULL == ext)
    {
        mxLog("NULL pointer");
        return -1;
    }
	if(ext->type == cJSON_Object)
	{
		for(ext = ext->child; ext != NULL; ext = ext->next)
		{
            memset(targetFullPath, 0, sizeof(targetFullPath));
			// Get target path
			GetExtFullPath(ext->valuestring, targetFullPath);

            // Add Keep List
			strcat(targetFullPath, "\n");
			WriteKeepListLine(keepfile, targetFullPath);
            
            //Get md5
            memset(md5, 0, sizeof(md5));
            GetMD5FromJsonPart(ext->string, md5);
            
            if(0 == isInExtTargetDir(targetFullPath, md5))
            {
                // no need to download or move
                continue;
            }
            else
            {
                AddExtToMoveListFile(extMovefile, md5, targetFullPath);
                
                if(0 != isInExtTempDir(md5))
                {
                    memset(oneLine, 0, sizeof(oneLine));
                    GetFullURL(json, ext->string, oneLine);
                    AddExtToDownloadListFile(extUrlfile, oneLine, md5, targetFullPath);
                }
            }
		}
	}
	else
	{
		mxLog("no ext this word");
		return -1;
	}
    return 0;
}


int ParseResPartJson(cJSON *json,  FILE * urlfile,  FILE * keepfile)
{
	
	char fullPath[MAXNAME];
	char oneLine[MAX_CHAR_ONE_LINE];
    
	if(NULL == json || NULL == urlfile || NULL == keepfile)
    {
        mxLog("NULL pointer");
        return -1;
    }
	// Get ext content
	cJSON * res = cJSON_GetObjectItem(json, "res");
    if(NULL == res)
    {
        mxLog("NULL pointer");
        return -1;
    }
	if(res->type == cJSON_Object)
	{
		res = res->child;
		while(res != NULL)
		{
			memset(fullPath, 0, sizeof(fullPath));
			// Get full path, 
			GetResFullPath(res->string, res->valuestring, fullPath);

			// check if exist
			if( 0 == CheckIfFileExist(fullPath))
			{
				// file exist
				// Add Keep List
				strcat(fullPath, "\n");
				WriteKeepListLine(keepfile, fullPath);
			}
			else
			{
				// file not exist , need download
				// Add URL list
				memset(oneLine, 0, sizeof(oneLine));
                GetFullURL(json, res->string, oneLine);
                
				strcat(oneLine, "\t");
				GetMD5FromJsonPart(res->string, oneLine+strlen(oneLine));
                
				strcat(oneLine, "\t");
				strcat(oneLine, fullPath);
                strcat(oneLine, "\n");
				WriteURLListLine(urlfile, oneLine);
                //mxLog("oneLine(URL+MD5+fullPath) = %s", oneLine);
			}
			
			res = res->next;
		}
	}
	else
	{
		mxLog("no res this word");
		return -1;
	}
    return 0;
}
#if 0
// use c++ lib method
int GenerateDeleteFile(void )
{  
    char aLine[MAX_CHAR_ONE_LINE];
    FILE * allfile = NULL;
	if((allfile = fopen(g_SSDALLFilePath,"r")) == NULL)
	{
		return -1;
	}

	FILE * keepfile = NULL;
	if((keepfile = fopen(g_KeepFilePath,"r")) == NULL)
	{
		return -1;
	}

	FILE * delfile = NULL;
	if((delfile = fopen(g_DelFilePath,"w+")) == NULL)
	{
		mxLog("open delfile failed");
		return -1;
	}
    mxLog("GenerateDeleteFile");

    mxClearHashMap();
    // add g_KeepFilePath's every line into mxHashMap
    int hashID = 0;
    while(NULL != fgets(aLine, sizeof(aLine), keepfile))
    {
        long sLen = strlen(aLine);
        if(0 == sLen) continue;
        if(1 == sLen && '\n' == aLine[0]) continue;
        if(0 < sLen && *(aLine+sLen-1) == '\n') *(aLine+sLen-1) = '\0';
        
        AddCStrToHashMap(aLine, hashID++);
    }


    // Read every line of g_SSDALLFilePath
    while(NULL != fgets(aLine, sizeof(aLine), allfile))
    {
        long sLen = strlen(aLine);
        if(0 == sLen) continue;
        if(1 == sLen && '\n' == aLine[0]) continue;
        if(0 < sLen && *(aLine+sLen-1) == '\n') *(aLine+sLen-1) = '\0';
        int result = LookUpCStr(aLine);
        // if the line is in mxHashMap, then keep this file, So There is nothing to do;
        if(0 == result)
        {
            continue;
        }
        // else, the line is not in mxHashMap, means this file should be deleted, then add this line into g_DelFilePath
        else
        {
            // add \n to end of string
            *(aLine + sLen - 1) = '\n';
            *(aLine + sLen) = '\0';
            fputs(aLine,delfile);
        }
    }

    fflush(delfile);
    fclose(delfile);
    fclose(keepfile);
    fclose(allfile);
    return 0;
}

// use c++ lib method
int GenerateDeleteFile(void )
{  
    //char aLine[MAX_CHAR_ONE_LINE];
    char * aLine = malloc(MAX_CHAR_ONE_LINE);
	if(NULL == aLine)
	{
		 mxLog(" !!!!!!!!!!!!!!! aLine = malloc(MAX_CHAR_ONE_LINE); Failed");
		 return -1;
	}
	memset(aLine, 0, MAX_CHAR_ONE_LINE);
	
    FILE * allfile = NULL;
	if((allfile = fopen(g_SSDALLFilePath,"r")) == NULL)
	{
		return -1;
	}

	FILE * keepfile = NULL;
	if((keepfile = fopen(g_KeepFilePath,"r")) == NULL)
	{
		return -1;
	}

	FILE * delfile = NULL;
	if((delfile = fopen(g_DelFilePath,"w+")) == NULL)
	{
		mxLog("open delfile failed");
		return -1;
	}
    mxLog("GenerateDeleteFile");

    mxClearHashMap();
    // add g_KeepFilePath's every line into mxHashMap
    int hashID = 0;
    while(NULL != fgets(aLine, sizeof(aLine), keepfile))
    {
        long sLen = strlen(aLine);
        if(0 == sLen) continue;
        if(1 == sLen && '\n' == aLine[0]) continue;
        if(0 < sLen && *(aLine+sLen-1) == '\n')
        {
			*(aLine+sLen-1) = '\0';
        }
        
        AddCStrToHashMap(aLine, hashID++);
    }


    // Read every line of g_SSDALLFilePath
    while(NULL != fgets(aLine, sizeof(aLine), allfile))
    {
        long sLen = strlen(aLine);
        if(0 == sLen)    continue;
        if(1 == sLen && '\n' == aLine[0])    continue;
        if(0 < sLen && *(aLine+sLen-1) == '\n')    *(aLine+sLen-1) = '\0';
        mxLog("aLine:%s", aLine);
        int result = LookUpCStr(aLine);
        mxLog("LookUpCStr result=%d", result);
        // if the line is in mxHashMap, then keep this file, So There is nothing to do;
        if(0 == result)
        {
            continue;
        }
        // else, the line is not in mxHashMap, means this file should be deleted, then add this line into g_DelFilePath
        else
        {
            // add \n to end of string
            *(aLine + sLen - 1) = '\n';
            *(aLine + sLen) = '\0';
            fputs(aLine,delfile);
        }
    }

    fflush(delfile);
    fclose(delfile);
    fclose(keepfile);
    fclose(allfile);
    return 0;
}

#endif

// c pointer's pointer method
int GenerateDeleteFile(void )
{
	//mxClearHashMap();
	FILE * allfile = NULL;
    
	if((allfile = fopen(g_SSDALLFilePath, "r")) == NULL)
	{
		return -1;
	}

	FILE * keepfile = NULL;
	if((keepfile = fopen(g_KeepFilePath, "r")) == NULL)
	{
		return -1;
	}

	FILE * delfile = NULL;
	if((delfile = fopen(g_DelFilePath, "w+")) == NULL)
	{
		mxLog("open delfile failed");
		return -1;
	}
    mxLog("GenerateDeleteFile");

	char *SSDALL  = NULL;
	char **ppLine = NULL;
	char oneline[MAX_CHAR_ONE_LINE];
    
	unsigned long lineNums = GetFileLineNumber(g_SSDALLFilePath);
	unsigned long allBytes = getFileSize(g_SSDALLFilePath)+lineNums+1;
	SSDALL = (char *)malloc(allBytes); 
    ppLine = (char **)malloc(sizeof(char *) * (lineNums+1));
	
	if(NULL == SSDALL || NULL == ppLine)
	{
		mxLog("SSDALL or ppLine is NULL");
		return -1;
	}
	
    memset(SSDALL, 0, allBytes);
    memset(ppLine, 0, sizeof(char *) * (lineNums+1));
    
	unsigned long lineIdex = 0;
	*(ppLine+lineIdex) = SSDALL; // ppLine's first pointer is  to SSDALL's first char
	while(NULL != fgets(*(ppLine+lineIdex), MAX_CHAR_ONE_LINE, allfile))
	{
		*(ppLine + lineIdex + 1) = SSDALL += strlen(*(ppLine + lineIdex)) + 1;
		//*ppLine[lineIdex] = SSDALL + strlen(*ppLine[lineIdex]) + 1;
		lineIdex++;
	}

	while(NULL != fgets(oneline, sizeof(oneline), keepfile))
	{
		for(unsigned long i = 0; i < lineNums; i++)
		{
			if(NULL == *(ppLine+i))
			{
				continue;
			}
			if(!strcmp(oneline, *(ppLine+i)))
			{
				// the same
				*(ppLine+i) = NULL;	
			}
			
		}
		
	}
	for(unsigned long i =0; i<lineNums; i++)
	{
		if(NULL == *(ppLine+i))
		{
			continue;
		}
		else
		{
			fputs(*(ppLine+i), delfile);
		}				
	}
	fflush(delfile);
    free(*ppLine);
    free(ppLine);
    fclose(delfile);
    fclose(keepfile);
    fclose(allfile);
    return 0;
}

unsigned long GetFileLineNumber(char * path)
{
	FILE * fp;
    if(NULL == path )
    {
        mxLog("NULL pointer");
        return -1;
    }
	if(NULL == (fp = fopen(path, "r")))
	{
		mxLog("open file failed");
		return -1;
	}
	unsigned long i = 0;
	char s[MAX_CHAR_ONE_LINE] = {0};
	while(NULL != fgets(s, sizeof(s), fp))
	{
		i++;
	}
	return i;
}

int AnalyzeExtAndResPart(cJSON *json)
{
	FILE * urlfile = NULL;
    
    if(NULL == json )
    {
        mxLog("NULL pointer");
        return -1;
    }
	if((urlfile = fopen(g_UrlFilePath, "w+")) == NULL)
	{
		mxLog("open urlfile failed");
		return -1;
	}

	FILE * keepfile = NULL;
	if((keepfile = fopen(g_KeepFilePath, "w+")) == NULL)
	{
		mxLog("open keepfile failed");
		return -1;
	}

    FILE * extUrlfile = NULL;
	if((extUrlfile = fopen(g_ExtUrlFilePath, "w+")) == NULL)
	{
		mxLog("open g_ExtUrlFilePath failed");
		return -1;
	}
    FILE * extMovefile = NULL;
	if((extMovefile = fopen(g_ExtMoveFilePath, "w+")) == NULL)
	{
		mxLog("open g_ExtMoveFilePath failed");
		return -1;
	}
    
    // parse res part
	ParseResPartJson(json, urlfile, keepfile);

	// parse ext part
	ParseExtPartJson(json, extUrlfile, extMovefile, keepfile );

	fclose(urlfile);
	fclose(keepfile);
    fclose(extUrlfile);
    fclose(extMovefile);

	//Generate Delete List file according to keep file and All files list of SSD
	GenerateDeleteFile();
    return 0;
}

int GenerateAllFilesList(char *bfDir)
{
	char command[2*MAXNAME] = {0};
    
	strcat(command, "find ");
	strcat(command, bfDir);
	strcat(command, "bsldata/ -type f > ");
	strcat(command, g_SSDALLFilePath);
    if ( 0 != system(command)) 
    {
        mxLog("Find Failed");
    }	
    return 0;
}


int GetVauleIntFromJson(char *JsonFileName, char *key)
{
    int retVal = 0;
	if(NULL == JsonFileName || NULL == key)
	{
		mxLog("NULL pointer!");
		return -1;
	}
	cJSON * json = NULL;
	long memlen = getFileSize(JsonFileName) + 1;
	char* memforJson = (char *)malloc(memlen);
	if(NULL == memforJson)
	{
		mxLog("No mem!");
		return -1;
	}
	memset(memforJson, 0, memlen);
	ReadJsonToMem(JsonFileName, memforJson);
	
	if( !(json = cJSON_Parse(memforJson)))
	{
		free(memforJson);
		mxLog("cJSON_Parse Faile!");
		return -1;
	}
	
	cJSON * id = cJSON_GetObjectItem(json,key);
    if(NULL == id)
	{
		mxLog("NULL pointer!");
		return -1;
	}
	if(id->type == cJSON_Number)
	{
		retVal = id->valueint;
	}
	else
	{
		mxLog("json No such Node!");
		cJSON_Delete(json);
		free(memforJson);
		return -1;
	}
	
	cJSON_Delete(json);
	free(memforJson);
    return retVal;
}


int GetVauleStringFromJson(const char *JsonFileName, char *key, char *result)
{
    if(NULL == JsonFileName || NULL == key || NULL == result)
	{
		mxLog("NULL pointer!");
		return -1;
	}
	cJSON * json        = NULL;
	long    memlen      = getFileSize(JsonFileName) + 1;
	char *  memforJson  = (char *)malloc(memlen);
    
	if(NULL == memforJson)
	{
		mxLog("No mem!");
		return -1;
	}
	memset(memforJson, 0, memlen);
	ReadJsonToMem(JsonFileName, memforJson);
	
	if( !(json = cJSON_Parse(memforJson)))
	{
		free(memforJson);
		mxLog("cJSON_Parse Faile!");
		return -1;
	}
	
	cJSON * id = cJSON_GetObjectItem(json, key);
    if(NULL == id)
	{
		mxLog("NULL pointer!");
		return -1;
	}
	if(id->type == cJSON_String)
	{
		strcpy(result, id->valuestring);
	}
	else
	{
		mxLog("json No such Node!");
		cJSON_Delete(json);
		free(memforJson);
		return -1;
	}
	
	cJSON_Delete(json);
	free(memforJson);
    return 0;
}



