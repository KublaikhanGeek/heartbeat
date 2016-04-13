/**********************
*
*Update for bsldate
***********************/
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "bfupdateapi.h"
#include "mxlib.h"
#include "mxcppapi.h"

typedef unsigned char*  T_ByteP;
typedef unsigned char   T_Byte;

mx_Data * pMXData = NULL;

int updateMyself(void);
int updateResource(void);
int updateNginx(void);
int ParseMXConf(const char * fileName, mx_Data * pD);
int VerifyHWMACfile(mx_Data * pD);
int makeHWMACconf(mx_Data * pD);
int PrepareWorkingDir(mx_Data * pD);
int ReadMXConfFiles(mx_Data * pD);
void CreateDirIfNotExist(char* dir);
void CleanUpMem(mx_Data * pD);
int DoUpdateNginx(mx_Data *pMXData);
int isMACAddrStr(char * str);
int TryUpdateMySelf(void);
void ResponUpdateTest(const char * input);



int InitialMain(mx_Data ** ppD)
{
    if(NULL == ppD)
    {
        mxLog("It's impossible, check your code!");
        return -1;
    }

    if(NULL != *ppD)
    {
        mxLog("It's impossible, check your code!");
        return -1;
    }
    
    *ppD = (mx_Data *)malloc(sizeof(mx_Data));
    if(NULL == *ppD)
    {
        //no memory
        mxLog("No Mem For pMXData. Fatal Error!!!!!");
        return -1;
    }
    memset(*ppD, 0, sizeof(mx_Data));

    if( 0 != access("./etc/conf", F_OK) && 0 != access("/etc/mx/conf", F_OK) )
    {
        return 0;
    }


    (*ppD)->HWFile                  = (char *)malloc(MAXNAME);
    (*ppD)->macFile                 = (char *)malloc(MAXNAME);
    (*ppD)->x86IP                   = (char *)malloc(MAXNAME);
    (*ppD)->NginxLocalVersionFile   = (char *)malloc(MAXNAME);
    (*ppD)->localIDFile             = (char *)malloc(MAXNAME);
    (*ppD)->LocalVersionFile        = (char *)malloc(MAXNAME);
    (*ppD)->etcPath                 = (char *)malloc(MAXNAME);
    (*ppD)->tmpPath                 = (char *)malloc(MAXNAME);
    (*ppD)->extPath                 = (char *)malloc(MAXNAME);
    (*ppD)->ResourceJsonFile        = (char *)malloc(MAXNAME);
    (*ppD)->NginxJsonFile           = (char *)malloc(MAXNAME);
    (*ppD)->DownloadNginxFile       = (char *)malloc(MAXNAME);
    (*ppD)->NginxFile               = (char *)malloc(MAXNAME);
    (*ppD)->targetPath              = (char *)malloc(MAXNAME);
    (*ppD)->NginxTargetPath         = (char *)malloc(MAXNAME);
    (*ppD)->JsonURLHead             = (char *)malloc(MAXNAME);
    (*ppD)->NginxJsonURLHead        = (char *)malloc(MAXNAME);
    (*ppD)->reportDomain            = (char *)malloc(MAXNAME);
    (*ppD)->reportPath              = (char *)malloc(MAXNAME);
    if(NULL == (*ppD)->HWFile                 ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->macFile                ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->x86IP                  ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->NginxLocalVersionFile  ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->localIDFile            ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->LocalVersionFile       ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->etcPath                ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->tmpPath                ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->extPath                ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->ResourceJsonFile       ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->NginxJsonFile          ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->DownloadNginxFile      ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->NginxFile              ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->targetPath             ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->NginxTargetPath        ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->JsonURLHead            ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->NginxJsonURLHead       ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->reportDomain           ){mxLog("NULL,Nomem");return -1;}
    if(NULL == (*ppD)->reportPath             ){mxLog("NULL,Nomem");return -1;}

    return 0;
}


int main (int argc, const char **argv)
{
    int ret = 0;
    
    if(0 == access("./init.sh", F_OK))
    {
        system("sh ./init.sh");
        system("mv -f init.sh init.sh.bak");
    }

    if( 2 == argc)
    {   
        ResponUpdateTest(argv[1]);
        return 0;
    }
    else
    {
        if( 0 == TryUpdateMySelf())
        {
            //update success, stop running myself.
            return 0;
        }
        //No update, continue running.
    }
    
    if(0 != InitialMain(&pMXData))
    {
        mxLog("InitialConfig Failed");
        return -1;
    }
    
    if(0 != ReadMXConfFiles(pMXData))
    {
        mxLog("InitialConfig Failed");
        return -2;
    }
    
    if(0 != PrepareWorkingDir(pMXData))
    {
        mxLog("PrepareWorkingDir Failed");
        return -3;
    }

    // check mac conf exist or not
    if(0 != VerifyHWMACfile(pMXData))
    {
        // not exist, create MAC conf
        mxLog("not exist, create MAC conf");
        if(0 != makeHWMACconf(pMXData))
        {
            mxLog("makeHWMACconf Failed and VerifyHWMACfile Failed");
            return -4;
        }
    }

    GetMAC(pMXData->hostMAC);
    mxLog("Start  MAC %s", pMXData->hostMAC);
    if(12 != strlen(pMXData->hostMAC))
    {
        mxLog("Wrong MAC = %s", pMXData->hostMAC);
        return -5;
    }
    
    EncryptMAC(pMXData->hostMAC, pMXData->encryptMAC);
    if(32 != strlen(pMXData->encryptMAC))
    {
        mxLog("Wrong encryptMAC = %s", pMXData->encryptMAC);
        return -6;
    }
    mxLog("EncryptMAC: %s", pMXData->encryptMAC);

    GetHWInfo(pMXData->HWFile, pMXData->HWInfo);
    mxLog("HWInfo: %s", pMXData->HWInfo);
    
    
    ret = updateNginx();
    if(0 != ret)
    {
       mxLog("updateNginx Failed!");
    }
    else
    {
       mxLog("updateNginx Success!");
    }

    ret = updateResource();
    if(0 != ret)
    {
        mxLog("updateResource Failed!");
    }
    else
    {
        mxLog("updateResource Success!");
    }

   
#if 0
    ret = updateMyself();
    if(0!=ret)
    {
        mxLog("updateMyself Failed!");
    }
    else
    {
        mxLog("updateMyself Success!");
    }
#endif

    if( 0 == access("./etc/conf", F_OK) || 0 == access("/etc/mx/conf", F_OK) )
    {
        CleanUpMem(pMXData);
    }
    mxLog("THE END!");
    return 0;
}



int updateMyself(void)
{
    return 0;
}


int updateNginx(void)
{    
    memset(pMXData->ngxData.localVersion, 0, MAXID);
    GetVersion(pMXData->NginxLocalVersionFile, pMXData->ngxData.localVersion);
    AssembleJsonURL(pMXData->NginxJsonURLHead, pMXData->encryptMAC, pMXData->ngxData.jsonUrlAddr);
    mxLog("ngxData.jsonUrlAddr=%s", pMXData->ngxData.jsonUrlAddr);
     
    if (0 != DownJson(pMXData->ngxData.jsonUrlAddr, pMXData->NginxJsonFile))
    {   //Get info err
        mxLog("ngx DownJson ERR");
        return -2;
    }

    if( 1 != GetVauleIntFromJson(pMXData->NginxJsonFile, "status"))
    {
        mxLog("NginxJsonFile status is not OK");
        return -3;
    }
    
    GetVauleStringFromJson(pMXData->NginxJsonFile, "version", pMXData->ngxData.jsonVersion);
    mxLog("NginxJsonFile version=%s", pMXData->ngxData.jsonVersion);

    GetVauleStringFromJson(pMXData->NginxJsonFile, "arch", pMXData->ngxData.jsonHWInfo);
    mxLog("NginxJsonFile arch=%s", pMXData->ngxData.jsonHWInfo);

    //arch compare
    if( 0 != strcmp( pMXData->HWInfo, pMXData->ngxData.jsonHWInfo))
    {
        // HW not match
        mxLog("Hardware Wrong, Please contact Baofeng!");
        return -2;
    } 

    //Version compare
    if ( 0 == strcmp(pMXData->ngxData.jsonVersion, pMXData->ngxData.localVersion))
    {
        //no need update
        mxLog("no need update!");
        return 0;
    }   

    // Get download URL and md5 from json
    GetVauleStringFromJson(pMXData->NginxJsonFile, "url", pMXData->ngxData.nginxURL);
    mxLog("Nginx URL head=%s", pMXData->ngxData.nginxURL);
    
    GetVauleStringFromJson(pMXData->NginxJsonFile, "md5", pMXData->ngxData.md5);
    mxLog("Nginx MD5=%s", pMXData->ngxData.md5);

    //strat to download firmware, need report to bf, 0x0006
	ReportStartUpdateFireware(pMXData);

    int retry, retval;
    for (retry = 0; retry < 10; retry++)
    {
        //DOWNLOAD Nginx.zip
        if (0 == (retval = DownloadNGXFile(pMXData->ngxData.nginxURL, pMXData->ngxData.md5, pMXData->DownloadNginxFile)))
        {
            break;    
        }
    }

    if(retry >= 10 && retval != 0)
    {
        mxLog("Download Nginx.zip Failed, give up");
        return -1;
            
    }

    DecryptionFile(pMXData->DownloadNginxFile, pMXData->NginxFile);
    mxLog("DecryptionFile NginxFile, path:%s", pMXData->NginxFile);

    //DO UPDATE Nginx
    if(0 != DoUpdateNginx(pMXData))
    {
        return -1;
    }
    // tar xcf NginxTargetPath/   NginxTargetPath../nginx.old.tar.gz
    // rm -rf NginxTargetPath/nginx/*
    // tar xvf NginxFile -C NginxTargetPath
 
    return 0;
}

int DoUpdateNginx(mx_Data *pMXData)
{
    char command[MAXURL] = {0};

    // unzip nginx
    //unzip test.zip -d /root/ 
    memset(command, 0, sizeof(command));
    strcat(command, "unzip -o ");
    strcat(command, pMXData->NginxFile);
    strcat(command, " -d ");
    strcat(command, pMXData->tmpPath);
    strcat(command, "nginx_tmp");
    mxLog("unzip nginx =%s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Unzip File Failed");
        return -1;
    }

    //if there is start.sh run it
    memset(command, 0, sizeof(command));
    strcat(command, pMXData->tmpPath);
    strcat(command, "nginx_tmp/start.sh");
    if(0 == access(command, F_OK))
    {   
        memset(command, 0, sizeof(command));
        strcat(command, "chmod +x ");
        strcat(command, pMXData->tmpPath);
        strcat(command, "nginx_tmp/start.sh");
        mxLog("chmod start.sh =%s", command);
        if ( 0 != system(command)) 
        {
            mxLog("Chmod File Failed");
            return -1;
        }
    
        memset(command, 0, sizeof(command));
        strcat(command, pMXData->tmpPath);
        strcat(command, "nginx_tmp/start.sh");  
        mxLog("start.sh run =%s", command);
        if ( 0 != system(command)) 
        {
            mxLog("Execute the script failure");
            //update fireware failed, need report to bf, 0x000c
			ReportUpdateFirewareFailed(pMXData);
            return -1;
        }
        else
        {
            if(0 != UpdateNginxVersion(pMXData->NginxLocalVersionFile))
            {
                return -1;
            }
            //update fireware success, need report to bf, 0x000b
			ReportUpdateFirewareSuccess(pMXData);
        }
    }
    else
    {
        mxLog("Not start.sh, finish");
        return -1;
    }
    return 0;
}

int DealWithRes(void)
{
    char      URLAddr[MAXURL]               = {0};
    char      md5[33]                       = {0};
	char      LocalFullPathName[MAXNAME]    = {0};
    char      TargetFullPathName[MAXNAME]   = {0};
    int       MAXRETRY                      = 0;
    int       FailCount                     = 0;
    int       downloadCount                 = 0;

    InitReadURLList();

    while (0 == ReadNextURLList(URLAddr, md5, TargetFullPathName) && MAXRETRY < 100)
    {
        int retry = 0;
        // Get a new URL to download success. and if failed no more than 100 continuously 
        memset(LocalFullPathName, 0, sizeof(LocalFullPathName));
        strcat(LocalFullPathName, pMXData->tmpPath);
        strcat(LocalFullPathName, md5);
        for ( retry = 0;  retry < 10; retry++) 
        {
            // download unsuccess or MD5 Failed, retry
            if(0 == DownloadResourceFile(URLAddr, LocalFullPathName))
            {
                // Download OK
                MAXRETRY = 0; // reset maxretry
                MoveFile(LocalFullPathName, TargetFullPathName);
                mxLog("Download OK");
                downloadCount++;
                break;
            }
            else
            {
                mxLog("retry...");
                MAXRETRY++;
            }
        }
        if(retry >= 10)
        {
            mxLog("retry  10 times, still fail");
            FailCount++;
        }
        mxLog("downloadCount Pass item: %d", downloadCount);
        memset(URLAddr, 0, sizeof(URLAddr));
    }
    
    if(0 != FailCount || MAXRETRY >= 100)
    {
        mxLog("some file failed, Retry update Next time. FailCount=%d, MAXRETRY=%d", FailCount, MAXRETRY);
        //some file failed, need report to bf, 0x0004
        ReportResourceDownloadFailed(pMXData);
        return -1;
    }
    else
    {
        return 0;
    }

}

int DownloadExtInExtURLList()
{
    char      URLAddr[MAXURL]               = {0};
    char      md5[33]                       = {0};
	char      LocalFullPathName[MAXNAME]    = {0};
    char      TargetFullPathName[MAXNAME]   = {0};
    int       MAXRETRY                      = 0;
    int       FailCount                     = 0;
    int       downloadCount                 = 0;

    InitReadExtURLList();

    while (0 == ReadNextURLList(URLAddr, md5, TargetFullPathName) && MAXRETRY < 100)
    {
        int retry = 0;
        // Get a new URL to download success. and if failed no more than 100 continuously 
        memset(LocalFullPathName, 0, sizeof(LocalFullPathName));
        strcat(LocalFullPathName, pMXData->tmpPath);
        strcat(LocalFullPathName, md5);
        for ( retry = 0;  retry < 10; retry++) 
        {
            // download unsuccess or MD5 Failed, retry
            if(0 == DownloadResourceFile(URLAddr, LocalFullPathName))
            {
                // Download OK
                MAXRETRY = 0; // reset maxretry
                if(0 != MoveFile(LocalFullPathName, pMXData->extPath))
                {
                    return -1;
                }
                mxLog("Download ext file ok:%s", TargetFullPathName);
                downloadCount++;
                break;
            }
            else
            {
                mxLog("retry...");
                MAXRETRY++;
            }
        }
        if(retry >= 10)
        {
            mxLog("retry 10 times, still fail");
            FailCount++;
        }
        mxLog("downloadCount Pass item: %d", downloadCount);
        memset(URLAddr, 0, sizeof(URLAddr));
    }
    
    if(0 != FailCount || MAXRETRY >= 100)
    {
        mxLog("some file failed, Retry update Next time. FailCount=%d, MAXRETRY=%d", FailCount, MAXRETRY);
        //some file failed, need report to bf, 0x0004
        ReportResourceDownloadFailed(pMXData);
        return -1;
    }
    else
    {
        //download resource over, need report to bf, 0x0003
	    ReportResourceDownloadFinish(pMXData);
        return 0;
    }

}


int MoveExtInExtMoveList()
{
    //start move data, need report to bf, 0x0005
	ReportStartUpdateResource(pMXData);
    char welcomeMD5[33] = {0};

    if(0 == isWelcomeInExtMoveList(welcomeMD5))
    {
        if(0 == DecompressWelcomZip(welcomeMD5))
        {
            if(0 != MoveWelcomeFolderToTarget())
            {
                return -1;
            }
            else
            {
                /*Add execute "env_config.sh" */
                if (0 == strcmp(pMXData->HWInfo, "x86"))
                {
                    if ( 0 != system("/opt/bf/nginx/env_config.sh")) 
                    {
                        mxLog("Execute the env's script failure!");
                    }
                }
                else if (0 == strcmp(pMXData->HWInfo, "mips"))
                {
                    if ( 0 != system("/opt/bf/scripts/env_config.sh")) 
                    {
                        mxLog("Execute the env's script failure!");
                    }
                }    
                else
                {
                    mxLog("Unknow Hardware!");
                }
            }
        }
        else
        {         
            return -1;
        }
    }
    return MoveExtFilesToTarget();
}

int DealWithExt(void)
{
    if(0 != DownloadExtInExtURLList())
    {
        return -1;
    }
    else if (0 != MoveExtInExtMoveList())
    {
        //update content failed, need report to bf, 0x0008
        ReportUpdateResourceFailed(pMXData);
        return -2;
    }
    else
    {
        return 0;
    }
}

int updateResource()
{

    mxLog("start updateResource");
    
    
    memset(pMXData->resData.localVersion, 0, MAXID);
    memset(pMXData->resData.localID, 0, MAXID);
    
    GetVersion(pMXData->LocalVersionFile, pMXData->resData.localVersion);
    mxLog("Version info in Local conf is %s", pMXData->resData.localVersion);
    
    GetID(pMXData->localIDFile, pMXData->resData.localID);
    mxLog("ID string in local conf is %s", pMXData->resData.localID);

    AssembleJsonURL(pMXData->JsonURLHead, pMXData->encryptMAC, pMXData->resData.JsonUrlAddr);

    // Get update info from BaoFeng Website
    if (0 != DownJson(pMXData->resData.JsonUrlAddr, pMXData->ResourceJsonFile))
    {
        //Get info err
        return -2;
    }

    if( 1 != GetVauleIntFromJson(pMXData->ResourceJsonFile, "status"))
    {
        mxLog("ResourceJsonFile status is not OK");
        return -3;
    }

    AnalyzeJson(pMXData->ResourceJsonFile, pMXData->tmpPath, pMXData->targetPath, pMXData->extPath);

    //Get New version info from downloaded Json
    GetVauleStringFromJson(pMXData->ResourceJsonFile, "id", pMXData->resData.JsonID);
    GetVauleStringFromJson(pMXData->ResourceJsonFile, "v",  pMXData->resData.JsonVersion);
    mxLog("JsonID=%sEND localID=%sEND", pMXData->resData.JsonID, ((mx_Data *)pMXData)->resData.localID);
    mxLog("JsonVersion=%sEND localversion=%sEND", pMXData->resData.JsonVersion, ((mx_Data *)pMXData)->resData.localVersion);
   
    // check if update needed
    if( 0 == strcmp(((mx_Data *)pMXData)->resData.JsonID, ((mx_Data *)pMXData)->resData.localID) 
      && 0 == strcmp(((mx_Data *)pMXData)->resData.JsonVersion, ((mx_Data *)pMXData)->resData.localVersion))
    {
        //no need update
        mxLog("no need update");
        return 0;
    }

    //strat to download resource, need report to bf, 0x0002
    ReportDownloadResourceStart(pMXData);

    mxLog("... update on going ...dealWithRes...");
    if(0 != DealWithRes())
    {
        mxLog("Err::dealWithRes not fully success, retry update later...");
        return -1;
    }
    mxLog("... update on going ...dealWithExt...");
    if(0 != DealWithExt())
    {
        mxLog("Err::dealWithExt not fully success, retry update later...");
        return -1;
    }

    mxLog("Delete Useless Files.");
    DeleteFileAccordingToDeleteListFile();
    
    mxLog("Update Finished, refresh Version and ID file");    
    UpdateLocaleVersion(pMXData->LocalVersionFile);
	UpdateLocaleID(pMXData->localIDFile);
    //update content success, need report to bf, 0x0007
    ReportUpdateResourceSuccess(pMXData);
    
    return 0;
}

#define READCONFITEM(NAME, VALUE) do{\
    int ret;                                               \
    if(0 != (ret = ReadValueFromConf(fileName, NAME, VALUE))) \
    {                                                      \
        mxLog("ReadValueFromConf  ERR code=%d", ret);      \
        mxLog("ReadConf PASS KEY=%s, Value=%s", NAME, VALUE);\
        return -1;                                         \
    }                                                      \
    else{mxLog("ReadConf PASS KEY=%s, Value=%s", NAME, VALUE);}\
}while(0)                                                          

int ParseMXConf(const char * fileName, mx_Data * pD)
{
    if (NULL == fileName || NULL == pD)
    {
    	mxLog("null");
        return -1;
    }
    
    READCONFITEM("HWFile"                    ,        pD->HWFile                         );
    READCONFITEM("macFile"                   ,        pD->macFile                        );
    READCONFITEM("x86IP"                     ,        pD->x86IP                          );
    READCONFITEM("NginxLocalVersionFile"     ,        pD->NginxLocalVersionFile          );
    READCONFITEM("localIDFile"               ,        pD->localIDFile                    );
    READCONFITEM("LocalVersionFile"          ,        pD->LocalVersionFile               );
    READCONFITEM("etcPath"                   ,        pD->etcPath                        );
    READCONFITEM("tmpPath"                   ,        pD->tmpPath                        );
    READCONFITEM("extPath"                   ,        pD->extPath                        );
    READCONFITEM("ResourceJsonFile"          ,        pD->ResourceJsonFile               );
    READCONFITEM("NginxJsonFile"             ,        pD->NginxJsonFile                  );
    READCONFITEM("DownloadNginxFile"         ,        pD->DownloadNginxFile              );
    READCONFITEM("NginxFile"                 ,        pD->NginxFile                      );
    READCONFITEM("targetPath"                ,        pD->targetPath                     );
    READCONFITEM("NginxTargetPath"           ,        pD->NginxTargetPath                );
    READCONFITEM("JsonURLHead"               ,        pD->JsonURLHead                    );
    READCONFITEM("NginxJsonURLHead"          ,        pD->NginxJsonURLHead               );
    READCONFITEM("ReportDomain"              ,        pD->reportDomain                   );
    READCONFITEM("ReportPath"                ,        pD->reportPath                     );
    READCONFITEM("ReportPort"                ,        pD->reportPort                     );
        
#if 0
    if(0 != (ret = ReadValueFromConf(fileName, "HWFile", pD->HWFile)))
    {
        mxLog("ReadValueFromConf ERR code=%d", ret);
        return -1;
    }
#endif

    return 0;
}

#undef READCONFITEM

int ReadMXConfFiles(mx_Data * pD)
{
    //check conf file from current path ./etc/conf, /etc/mx/conf, 
    if(0 == access("./etc/conf", F_OK) && 0 == ParseMXConf("./etc/conf", pD))
    {
        mxLog("using ./etc/conf");
    }
    else if(0 == access("/etc/mx/conf", F_OK) && 0 == ParseMXConf("/etc/mx/conf", pD))
    {
        mxLog("using /etc/mx/conf");
    }
    else
    {
        mxLog("No conf, using default value");

        pD->HWFile                 = "./etc/HWInfo.conf";                     
        pD->macFile                = "./etc/mac.conf";
        pD->x86IP                  = "192.168.100.6";
        pD->NginxLocalVersionFile  = "./etc/localNginxVersion.conf";          
        pD->localIDFile            = "./etc/localIDFile.conf";                
        pD->LocalVersionFile       = "./etc/localVersionFile.conf";    
        pD->etcPath                = "./etc/";  
        pD->tmpPath                = "./tmp/";
        pD->extPath                = "./ext/";
        pD->ResourceJsonFile       = "./tmp/ResourceJsonFile";                
        pD->NginxJsonFile          = "./tmp/NginxJsonFile";                   
        pD->DownloadNginxFile      = "./tmp/nginx.encrypt.zip";               
        pD->NginxFile              = "./tmp/nginx.zip";                       
        pD->targetPath             = "./root/Folder/iso/";                       
        pD->NginxTargetPath        = "./root/Folder/bf/";
        pD->JsonURLHead            = "http://update.jiayouzhan.tv/dev/pkgcontent"; 
        pD->NginxJsonURLHead       = "http://update.jiayouzhan.tv/dev/firmware";    
        pD->reportDomain           = "devents.jiayouzhan.tv";
        pD->reportPath             = "/report";
        strcpy(pD->reportPort, "80\0");   //pD->ReportPort is not char *, while is portNumber[6]
        
#if DEBUG
    return -1;
#endif 
    }
    return 0;
}



int isMACAddrStr(char * str)
{
	//f8:b1:56:c1:c8:0b
	
	
	if(18 == strlen(str)  )
	{
		if('\n' != str[17]) 
		{
		    mxLog("return");
			return -1;
		}
		else
		{
		    mxLog("return");
			str[17] = '\0';
		}
	}

	if(17 != strlen(str))
	{
	mxLog("return");
		return -1;
	}
	
	for(int i = 0; i < 17; i++)
	{
		if(2==i || 5==i || 8==i || 11==i || 14==i)
		{
			if(':' != str[i])
			{
			mxLog("return");
				return -1;
			}
		}
		else
		{
			//0,1:3,4:6,7:9,10:12,13:15,16
			if(!(('0' <= str[i] && '9' >= str[i]) || ('a' <= str[i] && 'f' >= str[i])))
			{
			mxLog("return");
				return -1;
			}
		}
	}
    mxLog("return");

	return 0;
}

int VerifyHWMACfile(mx_Data * pD)
{
    char line[MAXNAME] = {0};
    
    FILE * file = fopen(pD->macFile, "r");
    if(NULL == file)
    {
        return -1;
    }

    fgets(line, sizeof(line), file);
    fclose(file);
    line[17] = '\0';

    // check format
    return isMACAddrStr(line);
    
}



int SaveTempMacForX86(char * tmpDir)
{
	char command[MAXURL] = {0};
    
	strcat(command, "arp -a ");
    strcat(command, pMXData->x86IP);
    strcat(command, " > ");
	strcat(command, tmpDir);
    strcat(command, ".tmpMAC");
    if ( 0 != system(command)) 
    {
        mxLog("Achieve MAC Failed");
    }
    return 0;
}

int SaveTempMacForMIPS(char * tmpDir)
{
    char command[MAXURL] = {0};
    
	strcat(command, "iwconfig > ");
	strcat(command, tmpDir);
    strcat(command, ".tmpMAC");
    if ( 0 != system(command)) 
    {
        mxLog("Achieve MAC Failed");
    }
    return 0;
}

int WriteMacNetNametoFile(FILE * fp, char *mac, char *net_name)
{
	char line[MAXURL] = {0};
    
	strcat(line, mac);
	strcat(line, "    ");
	strcat(line, net_name);
    strcat(line, "\n");
	fputs(line, fp);
	return 0;
}

int GetNetNameInLine(char * line, char * net_name)
{
	char myLine[MAXURL] = {0};
    
	strcpy(myLine, line);
	if(myLine[0] == ' ' || myLine[0] == '\n')
	{
		return 0;
	}
	char *token = strtok(myLine, " ");
	if(0 == strlen(token))
	{
		return 0;
	}
	strcpy(net_name, token);
	return 0;
}

int BigCharToSmallChar(char *str)
{
	while(*str != '\0')
	{
		if('A' <= *str && *str <= 'Z')
		{
			*str = *str - 'A' + 'a';
		}
		str++;
	}
	return 0;
}

int GetMacInLine(char * line, char * mac)
{
	char   myLine[MAXURL]  = {0};
	char * token           = NULL;
    
    strcpy(myLine, line);
    char *p = myLine;
	while(NULL != (token = strtok(p, " ")))
	{
		p = NULL;
        mxLog("Before = %s", token);
		BigCharToSmallChar(token);
        mxLog("After = %s", token);
		if(0 == isMACAddrStr(token))
		{

			strcpy(mac, token);
			*(mac + strlen(token) +1) = '\0';
            mxLog("MAC = %s", mac);
			break;
		}
	}
	return 0;
}

int SaveMacToFile(char *tmpDir, char *macFileFullName)
{
	char   line[MAXURL]   = {0};
	char   net_name[18]   = {0};
	char   mac[18]        = {0};
	FILE * tmpFile        = NULL;
	FILE * macfile        = NULL;

	strcat(line, tmpDir);
    strcat(line, ".tmpMAC");
	if(NULL == (tmpFile = fopen(line, "r")))
	{
		return -1;
	}
	if(NULL == (macfile = fopen(macFileFullName, "w+")))
	{
		fclose(tmpFile);
		return -1;
	}
	// Read A block
	memset(net_name, 0, sizeof(net_name));
    memset(mac, 0, sizeof(mac));
    memset(line, 0, sizeof(line));
	while(NULL != fgets(line, sizeof(line), tmpFile))
	{
		// if a new block, clear net_name and mac
		mxLog("line=%s", line);
		if(line[0] != ' ' && line[0] != '\n')
		{
			if(0 != strlen(mac))
			{
			    mxLog("WriteMacNetNametoFile mac=%s, net_name=%s", mac, net_name);
				WriteMacNetNametoFile(macfile, mac, net_name);
			}
			memset(net_name, 0, sizeof(net_name));
			memset(mac, 0, sizeof(mac));
		}
		GetNetNameInLine(line, net_name);
		GetMacInLine(line, mac);
        
	}
	if(0 != strlen(mac))
	{
	    mxLog("WriteMacNetNametoFile mac=%s, net_name=%s", mac, net_name);
		WriteMacNetNametoFile(macfile, mac, net_name);
	}

	fclose(tmpFile);
	fclose(macfile);
	return 0;
}


int SaveMacToMacFile(char * hwVer, char * tmpDir, char * macFileFullName)
{
    mxLog("SaveMacToMacFile hwVer=%s", hwVer);
	if(0 == strcmp("x86", hwVer))
	{
	    mxLog("SaveMacToMacFile x86");
		SaveTempMacForX86(tmpDir);
		SaveMacToFile(tmpDir, macFileFullName);
	}
	else if(0 == strcmp("mips", hwVer))
	{
	    mxLog("SaveMacToMacFile mips");
		SaveTempMacForMIPS(tmpDir);
		SaveMacToFile(tmpDir, macFileFullName);
	}
	else
	{
	    mxLog("SaveMacToMacFile Fail");
		return -1;
	}
    mxLog("SaveMacToMacFile OK");
    return 0;
}


int makeHWMACconf(mx_Data * pD)
{
    GetHWInfo(pMXData->HWFile, pMXData->HWInfo);
    return SaveMacToMacFile(pMXData->HWInfo, pD->tmpPath, pD->macFile);
}

int makeHWVerconf(mx_Data * pD)
{
    char command[MAXURL] = {0};
    
    strcat(command, "uname -a > ");
    strcat(command, pD->tmpPath);
    strcat(command, ".tmpUname");
    mxLog("command =%s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Check System Version Failed");
    }
    memset(command, 0, sizeof(command));
    strcat(command, pD->tmpPath);
    strcat(command, ".tmpUname");
        

    FILE * fileR = fopen(command, "r");
    if(NULL == fileR)
    {
        return -1;
    }
    
    FILE * fileW = fopen(pD->HWFile, "w+");
    if(NULL ==fileW)
    {
        fclose(fileR);
        return -1;
    }

    memset(command, 0, sizeof(command));
    fgets(command, sizeof(command), fileR);
    if(0 == FindSubStrInString(command, "x86"))
    {
        fputs("x86", fileW);
    }
    else if(0 == FindSubStrInString(command, "mips"))
    {
        fputs("mips", fileW);
    }
    else if(0 == FindSubStrInString(command, "mtk"))
    {
        fputs("mtk", fileW);
    }
    else 
    {
        fputs("Unknow Hardware", fileW);
    }
    

    fflush(fileW);
    fclose(fileW);
    pclose(fileR);
    return 0;
}


int PrepareWorkingDir(mx_Data * pD)
{
    if( 0 == (
        pD
        && pD->HWFile                 
        && pD->macFile   
        && pD->x86IP
        && pD->NginxLocalVersionFile  
        && pD->localIDFile            
        && pD->LocalVersionFile       
        && pD->tmpPath  
        && pD->etcPath 
        && pD->extPath
        && pD->ResourceJsonFile       
        && pD->NginxJsonFile          
        && pD->DownloadNginxFile      
        && pD->NginxFile              
        && pD->targetPath             
        && pD->NginxTargetPath        
        && pD->JsonURLHead                
        && pD->NginxJsonURLHead
        && pD->reportDomain
        && pD->reportPath
        && pD->reportPort   ))
    {
        mxLog("some config read err, empty pointer");
        return -1;
    }
    char command[MAXURL] = {0};

    //clean up temp forlder
    memset(command, 0, sizeof(command));
    strcat(command, "rm -rf ");
    strcat(command, pD->tmpPath);
    if ( 0 != system(command)) 
    {
        mxLog("Remove File Failed");
    }
    mxLog("delete temp dir command =%s", command);

    CreateDirIfNotExist(pD->extPath );
    CreateDirIfNotExist(pD->tmpPath );
    CreateDirIfNotExist(pD->etcPath );
    CreateDirIfNotExist(pD->targetPath );
    CreateDirIfNotExist(pD->NginxTargetPath );

    memset(command, 0, sizeof(command));
    strcat(command, "touch ");
    strcat(command, pD->NginxLocalVersionFile);
    //mxLog("command =%s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Touch File Failed");
    }

    memset(command, 0, sizeof(command));
    strcat(command, "touch ");
    strcat(command, pD->localIDFile);
    //mxLog("command =%s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Touch File Failed");
    }

    memset(command, 0, sizeof(command));
    strcat(command, "touch ");
    strcat(command, pD->LocalVersionFile);
    //mxLog("command =%s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Touch File Failed");
    }

    memset(command, 0, sizeof(command));
    strcat(command, "uname -a > ");
    strcat(command, pD->tmpPath);
    strcat(command, ".tmpUname");
    //mxLog("command =%s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Check System Version Failed");
    }
    
    makeHWVerconf(pD);
    
    return 0;
}


void CreateDirIfNotExist(char* dir)
{
    char buf[MAXURL] = "if [ ! -x ";
    
	strcat(buf, dir);
	strcat(buf, " ]; then mkdir -p ");
	strcat(buf, dir);
	strcat(buf, "; fi");
    mxLog("CreateDirIfNotExist command=%s", buf);
	    if ( 0 != system(buf)) 
    {
        mxLog("Make Directory Failed");
    }

    memset(buf, 0, sizeof(buf));
    strcat(buf, "chmod -R 777 ");
    strcat(buf, dir);
    //mxLog("command=%s", buf);
	if ( 0 != system(buf))
    {
        mxLog("Chmod Directory Failed");
    }
}

void CleanUpMem(mx_Data * pD)
{

    free( pD->HWFile                                           );    
    free( pD->macFile                                          ); 
    free( pD->x86IP                                            );
    free( pD->NginxLocalVersionFile                            );    
    free( pD->localIDFile                                      );    
    free( pD->LocalVersionFile                                 );    
    free( pD->etcPath                                          );    
    free( pD->tmpPath                                          );    
    free( pD->extPath                                          );   
    free( pD->ResourceJsonFile                                 );    
    free( pD->NginxJsonFile                                    );    
    free( pD->DownloadNginxFile                                );    
    free( pD->NginxFile                                        );    
    free( pD->targetPath                                       );    
    free( pD->NginxTargetPath                                  );    
    free( pD->JsonURLHead                                      );    
    free( pD->NginxJsonURLHead                                 );    
    free( pD->reportDomain                                     );
    free( pD->reportPath                                       );
    free( pD);
}


void ResponUpdateTest(const char * input)
{
    if (0 == strcmp (input, "UpdateTest")) 
    {
        char date[9] = {0};
        DateString(__DATE__, date);
        printf("MXinfo %s", date);
    }
    return;
}

int TryUpdateMySelf(void)
{
    if (0 != (access("./mxupdate.new", F_OK))) 
    {
        // not exist, return, update failed.
        // but it's normal, do not nerves.
        return -1;
    }
    else
    {
        // test .new file
        char res[200]   = {0};
        char date[9]    = {0};
        FILE * fp = popen("./mxupdate.new UpdateTest", "r");
        if(NULL == fp)
        {
            system ("rm -f mxupdate.new");
            return -2;
        }
        fgets(res, sizeof(res), fp);
        pclose(fp);
        DateString(__DATE__, date);
        if(0 == strncmp(res, "MXinfo", 6) && 0 < strncmp(res+7, date, 8))
        {
            // check pass, update myself
            mxLog("check pass, update myself my date:%s .newFile date:%s", date, res+7);
            if(0 != system ("mv -f mxupdate mxupdate.old")) 
            {
                return -4;
            }

            if(0 != system ("mv -f mxupdate.new mxupdate"))
            {
                mxLog("mxupdate Update Failed!!");
                while( 0 != system ("mv -f mxupdate.old mxupdate"))
                {
                    // Yes, I want keep cycle deadly here, 
                    // because if mxupdate is gone, and I can't move .old to mxupdate, 
                    // Then, there is nothing I can do.
                    mxLog("mxupdate Update Failed, Recovering........");
                    system ("sh ./UpdateRescue.sh");
                    // You can put an updaterescue.sh to save something.
                }
                mxLog("mxupdate Update Failed, But Recovering Success.");
                while( 0 != system ("rm -f mxupdate.new"))
                {
                    mxLog("Clean up Bad mxupdate.new, Failed, Try Again.");
                }
                mxLog("Clean up Bad mxupdate.new, Success.");
                system("rm -f ./afterupdatemyself.sh");
                mxLog("Clean up Bad afterupdatemyself.sh.");
                return -5;
            }
            else
            {
                //update OK
                mxLog("mxupdate Update Myself Success!!");
                if(0 !=system ("sh ./afterupdatemyself.sh"))
                {
                    mxLog("No afterupdatemyself.sh file or run it failed.");
                }
                else
                {
                    system("rm -f ./afterupdatemyself.sh");
                }
                return 0;
            }
        }
        else
        {
            // MXinfo not match or, the version is older than me.
            mxLog("MXinfo not match or, the version is older than me. my date:%s .newFile date:%s",date,res+7);
            system("rm -f ./afterupdatemyself.sh");
            system("rm -f mxupdate.new");
            return -3;
        }
    }
    return -1; // default fail. only success return 0;
    
}



