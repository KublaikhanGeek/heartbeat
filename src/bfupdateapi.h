#ifndef _bfupdeatapi_h_
#define _bfupdeatapi_h_ 


#include <stdio.h>
#include <string.h>
#include <time.h> 
#include <stdlib.h>

#ifndef TIMEOUT
#define TIMEOUT    200
#endif
#ifndef MAXURL
#define MAXURL     1024
#endif

#ifndef MAXNAME
#define MAXNAME    128
#endif

#ifndef MAXID
#define MAXID      128
#endif

#ifndef BFUPDATEAPI_C
#define BFUPDATEAPI_EXT
#else
#define BFUPDATEAPI_EXT extern
#endif

#define MAXHWINFOLEN 16

struct res_Data 
{
    char  JsonUrlAddr[MAXURL];
    char  JsonVersion[MAXID];
    char  JsonID[MAXID];
    char  localVersion[MAXID];
    char  localID[MAXID];
};

struct ngx_Data
{
    char      jsonHWInfo[MAXID];
    char      localVersion[MAXID];
    char      jsonVersion[MAXID];
    char      jsonUrlAddr[MAXURL];
    char      nginxURL[MAXURL];
    char      md5[33];
};

typedef struct tagMXData
{

    char * HWFile;                
    char * macFile;
    char * x86IP;
    char * NginxLocalVersionFile; 
    char * localIDFile;           
    char * LocalVersionFile;      
    char * etcPath;
    char * tmpPath; 
    char * extPath;
    char * ResourceJsonFile;      
    char * NginxJsonFile;         
    char * DownloadNginxFile;     
    char * NginxFile;             

    char * targetPath;            
    char * NginxTargetPath;       
    char * JsonURLHead;           
    char * NginxJsonURLHead;      

    char   hostMAC[13];
    char   encryptMAC[33];
    char   HWInfo[MAXHWINFOLEN];

    char  * reportDomain;
    char  * reportPath;
    char    reportPort[6];
    
    struct ngx_Data ngxData;
    struct res_Data resData;

} mx_Data;

BFUPDATEAPI_EXT int GetMAC(char *mac);

BFUPDATEAPI_EXT int EncryptMAC(char *mac, char *EncryptMAC);

BFUPDATEAPI_EXT int  OneByteToTwoChar(unsigned char value, char *twoChar);

BFUPDATEAPI_EXT int AssembleJsonURL(const char* urlhead, const char *id, char *JsonUrl);

BFUPDATEAPI_EXT int DownJson(const char *JsonUrlAddr, const char* jsonFileName);

BFUPDATEAPI_EXT int GetVersion(const char *VERFile, char *versionNum);

BFUPDATEAPI_EXT int GetID(const char *IDFile, char *id);

BFUPDATEAPI_EXT int DownloadResourceFile(const char *url, const char* pathname);

BFUPDATEAPI_EXT int DownloadNGXFile(const char *url, const char* md5, const char* pathname);

BFUPDATEAPI_EXT int CheckMD5(const char *namePath, const char *md5);

BFUPDATEAPI_EXT void GetMD5FromFile(const char *file, char *outMD5);

BFUPDATEAPI_EXT int MoveFile(const char * srcNamePath,  const char * targetNamePath);
BFUPDATEAPI_EXT void MoveBSLList(void);


BFUPDATEAPI_EXT int DecryptionFile(const char *Encryptfile, const char *Decryptedfile);
BFUPDATEAPI_EXT int GetHWInfo(const char *confile, char *info);

BFUPDATEAPI_EXT int UpdateNginxVersion(char * versionFile);

BFUPDATEAPI_EXT int MoveExtFileToExtDir(const char *srcNamePath, const char * targetNamePath);

int ReportDownloadResourceStart(mx_Data *pD);
int ReportResourceDownloadFinish(mx_Data *pD);
int ReportResourceDownloadFailed(mx_Data *pD);
int ReportStartUpdateResource(mx_Data *pD);
int ReportStartUpdateFireware(mx_Data *pD);
int ReportUpdateResourceSuccess(mx_Data *pD);
int ReportUpdateResourceFailed(mx_Data *pD);
int ReportUpdateFirewareSuccess(mx_Data * pD);
int ReportUpdateFirewareFailed(mx_Data * pD);





#endif


