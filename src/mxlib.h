
#ifndef _MXLIB_H_
#define _MXLIB_H_

#ifndef _MXLIB_C_
#define MXLIB_EXT extern 
#else 
#define MXLIB_EXT
#endif 
#include <time.h>
// Fri Apr 24 15:05:25 2015 - File:mxlib.c Line:10 Mydebug info 

#define mxLog(LOGFORMATE,...) \
	do{\
		mxMyLogHead();\
		printf("F:%s",__FILE__);\
		printf(" %s()",__func__);\
	    printf(" L:%d ",__LINE__);\
		printf(LOGFORMATE,##__VA_ARGS__);\
		printf("\n");\
		fflush(stdout);\
		}while(0)

#ifndef MAXNAME
#define MAXNAME 128
#endif
    
#ifndef MAX_CHAR_ONE_LINE
#define MAX_CHAR_ONE_LINE 1024
#endif


typedef struct reportData{
    char    HWInfo          [8];
    char    mac             [13];
    char    macFile         [64];
    char    targetPath      [128];
    char    encryptMac      [33];
    char    reportDomain    [128];
    char    reportPath      [64];
    char    port            [4];
    char    faultReportFile [64];
}Report_Data;





MXLIB_EXT int InitReadURLList();
MXLIB_EXT int InitReadExtURLList(void);

MXLIB_EXT int ReadNextURLList(char * URL, char * md5, char * name);
MXLIB_EXT int ReadNextName(char * name);
MXLIB_EXT int InitReadDeleteList(const char * tmpPath);
MXLIB_EXT void FinishReadDeleteList();
MXLIB_EXT int ReadNextDeleteList(char * toDel);
MXLIB_EXT int AnalyzeJson(const char * jsonFile, const char * workDir, const char * bfDir, const char * extDir);


MXLIB_EXT int GetMD5FromJsonPart(const char * input, char * md5);
MXLIB_EXT long getFileSize(const char * strFileName);
MXLIB_EXT int DeleteFileAccordingToDeleteListFile(void);
MXLIB_EXT int UpdateLocaleVersion(const char * LocalFullPath);
MXLIB_EXT int UpdateLocaleID(const char * LocalFullPath);


MXLIB_EXT void mxMyLogHead(void);
MXLIB_EXT void mxMyLogBody(char* format, ...);



MXLIB_EXT  int GetVauleStringFromJson(const char *JsonFileName, char *key, char *result);
MXLIB_EXT  int GetVauleIntFromJson(char *JsonFileName, char *key);

MXLIB_EXT void DateString(char const *date, char *buff);

MXLIB_EXT int isWelcomeInExtMoveList(char * welcomeMD5);
MXLIB_EXT int DecompressWelcomZip(char * welcomeMD5);
MXLIB_EXT int MoveWelcomeFolderToTarget(void);
MXLIB_EXT int MoveExtFilesToTarget(void);

MXLIB_EXT int EncryptJson(char *json, unsigned char *encryptjson, unsigned int size);
MXLIB_EXT int Sending(int port, char *hostname, unsigned char *request, int requestLen);
MXLIB_EXT int ReportJson(int port, char *hostname, char *reportPath, char *encryptMac, char *json);
MXLIB_EXT int StringRandom(char *string, int lengh);



MXLIB_EXT int GetReportMAC(char * MACFile, char *mac);
MXLIB_EXT int VerifyHWInfo(char * hwInfo, char *hwName);
MXLIB_EXT int GetCPUNumber(int * number);
MXLIB_EXT int CpuUsage(char * hwInfo, float cpu[], int cpuNumber);
MXLIB_EXT int ConnectNum (int *ccc);
MXLIB_EXT int SystemLoad (float sysload[]);
MXLIB_EXT int MemoryUsage (unsigned long men[]);

MXLIB_EXT int NginxServiceCheck (Report_Data *reportData);
MXLIB_EXT int CreateHeartJson(char * json, int ccc, unsigned long mem[], float cpu[], int cpuNumber, float sysload[]);
MXLIB_EXT int CreateFaultJson(int typeNumber, char * json);

MXLIB_EXT int OverWriteStringToFile(char * filePath, char * string);
MXLIB_EXT int CheckTwoFileMD5(const char *fileName1, const char *fileName2);
















#endif


