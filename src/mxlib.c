#define _MXLIB_C_

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "mxlib.h"
#include "bfupdateapi.h"
#include "mxcppapi.h"
//header file for report interface
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "md5.h"




#ifndef MAXNAME
#define MAXNAME 128
#endif

#ifndef MAX_CHAR_ONE_LINE
#define MAX_CHAR_ONE_LINE 1024
#endif

extern char g_UrlFilePath[MAXNAME];
extern char g_DelFilePath[MAXNAME];
extern char g_vFilePath[MAXNAME];
extern char g_idFilePath[MAXNAME];
extern char g_KeepFilePath[MAXNAME];
extern char g_SSDALLFilePath[MAXNAME];
extern char g_bfTargetPath[MAXNAME];
extern char g_extDir[MAXNAME];
extern char g_ExtUrlFilePath[MAXNAME];
extern char g_ExtMoveFilePath[MAXNAME];


extern mx_Data *pMXData;

extern int cjsonfile(const char *xmlpath );
extern int JsonPrepare(const char *workDir, const char *bfDir, const char *extDir);


static FILE * pFileReadURLRecords = NULL;
int InitReadURLList(void)
{
	if(NULL == (pFileReadURLRecords = fopen(g_UrlFilePath, "r")))
	{
		mxLog("open pFileReadURLRecords failed");
		return -1;
	}
	return 0;
}

int InitReadExtURLList(void)
{
	if(NULL == (pFileReadURLRecords = fopen(g_ExtUrlFilePath, "r")))
	{
		mxLog("open pFileReadURLRecords failed");
		return -1;
	}
	return 0;
}


/*
void FinishReadURLList()
{
	fclose(pFileReadURLRecords);
}
*/
int ReadNextURLList(char * URL, char * md5, char * name)
{
    if(NULL == URL || NULL == md5 ||NULL == name)
	{
		mxLog("NULL pointer!");
		return -1;
	}
	if(NULL == pFileReadURLRecords )
	{
		mxLog("pFileReadURLRecords is NULL");
		return -1;
	}
	char line[MAX_CHAR_ONE_LINE] = {0};
	if(NULL == fgets(line, MAX_CHAR_ONE_LINE, pFileReadURLRecords))
	{
		fclose(pFileReadURLRecords);
		mxLog("pFileReadURLRecords is over");
		return -1;
	}

	//Line Format 
	//URLURLURLURLURL \t MD5MD5MD5 \t NAMENAMENAME
	char *token = strtok(line, "\t");
	if(NULL == token)
	{
		mxLog("token1 is NULL");
		return -1;
	}
	if(NULL != URL)
	{
		strcpy(URL, token);
	}
	
	token = strtok(NULL, "\t");
	if(NULL == token)
	{
		mxLog("token2 is NULL");
		return -1;
	}
	if(NULL != md5)
	{
		strcpy(md5, token);
	}

	token = strtok(NULL, "\t");
	if(NULL == token)
	{
		mxLog("token3 is NULL");
		return -1;
	}
	if(NULL != name)
	{
		strcpy(name, token);
	}
	
	return 0;
}

int ReadNextName(char * name)
{
    if(NULL == name)
	{
		mxLog("NULL pointer!");
		return -1;
	}
	return ReadNextURLList(NULL, NULL, name);
}


static FILE * pFileReadDeletListRecords = NULL;
int InitReadDeleteList(const char * tmpPath)
{
    if(NULL == tmpPath )
    {
        mxLog("NULL pointer");
        return -1;
    }
	if(NULL == (pFileReadDeletListRecords = fopen(g_DelFilePath,"r")))
	{
		mxLog("open pFileReadDeletListRecords failed!");
		return -1;
	}
	return 0;
}

void FinishReadDeleteList()
{
	fclose(pFileReadDeletListRecords);
}

int ReadNextDeleteList(char * toDel)
{
	if(NULL == pFileReadDeletListRecords  || NULL == toDel)
	{
		mxLog("pFileReadDeletListRecords or toDel is NULL");
		return -1;
	}
	
	if(NULL == fgets(toDel, MAX_CHAR_ONE_LINE, pFileReadDeletListRecords))
	{
		fclose(pFileReadDeletListRecords);
		mxLog("toDel is over");
		return -1;
	}
    return 0;
}


int AnalyzeJson(const char * jsonFile, const char * workDir, const char * bfDir, const char * extDir)
{
	int ret = -1;
	
	ret = JsonPrepare(workDir, bfDir ,extDir);
	if(0 != ret)
	{
		mxLog("JsonPrepare failed");
		return ret;
	}
	
	// read json, generate vfile, idfile, resfile, extfile.
	ret = cjsonfile(jsonFile);
	if(0 != ret)
	{
		mxLog("cjsonfile failed");
		return ret;
	}

	return ret;
}


int DeleteFileAccordingToDeleteListFile(void)
{
	FILE * deletelist;
	if(NULL == (deletelist= fopen(g_DelFilePath,"r")))
	{
		mxLog("open deletelist failed!");
		return -1;
	}
    mxLog("delete start!");
	char command[MAX_CHAR_ONE_LINE] = {0};
	strcat(command, "rm ");
	while(NULL != fgets(command + 3, MAX_CHAR_ONE_LINE - 3, deletelist))
	{
	    mxLog("delete command=%s", command);
	    if ( 0 != system(command)) 
	   	{
	        mxLog("Remove File Failed");
	    }
	}
	fclose(deletelist);

    //finish delete Files,
    // clean up empty forlder
    //"find . -type d -empty -exec rmdir {} \;"
#ifdef UNLockRmdir
    memset(command, 0, sizeof(command));
    strcat(command, "find ");
    strcat(command, ((mx_Data *)pMXData)->targetPath);
    strcat(command, "bsldata/ ");
    strcat(command, "-type d -empty -exec rmdir {} \\;");
    for(int i = 0; i < 3; i++)
    {
        mxLog("clean up empty command=%s", command);
        if ( 0 != system(command)) 
        {
            mxLog("Clean Failed");
        }
    }
#endif

	return 0;
}

int UpdateLocaleVersion(const char * LocalFullPath)
{
    char command[MAX_CHAR_ONE_LINE] = {0};
    
    strcat(command, "cp -f ");
    strcat(command, g_vFilePath);
    strcat(command, " ");
    strcat(command, LocalFullPath);
    // mxLog("UpdateLocaleVersion command=%s", command);

    if ( 0 != system(command)) 
   	{
        mxLog("CP File Failed");
    }
    return 0;
}

int UpdateLocaleID(const char * LocalFullPath)
{
	char command[MAX_CHAR_ONE_LINE] = {0};
    
	strcat(command,"cp -f ");
	strcat(command, g_idFilePath);
	strcat(command, " ");
	strcat(command, LocalFullPath);
	//mxLog("UpdateLocaleID command=%s", command);
    if ( 0 != system(command)) 
   	{
        mxLog("CP File Failed");
    }
    return 0;
}

void mxMyLogHead(void)
{
	// Fri Apr 24 15:05:25 2015
	char timeStr[50] = {0};
	time_t now = time(NULL);
	if(-1 != now)
	{
		sprintf(timeStr, "%s", ctime(&now));
        if(1 < strlen(timeStr))
        {
            *(timeStr + strlen(timeStr)-1) = ' ';
            printf("%s", timeStr);
        }
	}
}

void mxMyLogBody(char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	printf(format, ap);
	va_end(ap);
}

void DateString(char const *date, char *buff) 
{ 
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff) - month_names) / 3 + 1;
    sprintf(buff, "%d%02d%02d", year, month, day);
}

int isWelcomeInExtMoveList(char * welcomeMD5)
{
    FILE *extMoveFile = NULL;
    if(NULL == (extMoveFile = fopen(g_ExtMoveFilePath, "r")))
    {
        mxLog("open failed:%s", g_ExtMoveFilePath);
    }
    char line[MAX_CHAR_ONE_LINE] = {0};
    while(NULL != fgets(line, MAX_CHAR_ONE_LINE, extMoveFile))
    {
        if((0 == FindSubStrInString(line, "welcome.zip"))
		        && (*(line + (strlen(line) -2)) == 'p'))
        {
            char *token;
            
            token = strtok(line, "\t");
            
            if(NULL != token && 32 == strlen(token))
            {
                memcpy(welcomeMD5, token, 32);
                welcomeMD5[32] = '\0';
                fclose(extMoveFile);
                return 0;
            }
            else
            {
                fclose(extMoveFile);
                mxLog("fatal Error");
                return -1;
            } 
        }
    }
    fclose(extMoveFile);
    return 1;
}

int DecompressWelcomZip(char * welcomeMD5)
{
    if(NULL == welcomeMD5)
    {
        mxLog("NULL welcomeMD5");
        return -1;
    }
    char command[MAXURL] = {0};
    strcat(command, "unzip -o ");
    strcat(command, pMXData->extPath);
    strcat(command, welcomeMD5);
    strcat(command, " -d ");
    strcat(command, pMXData->tmpPath);
    if(0 != system(command))
    {
        mxLog("command ERR:%s", command);
        return -1;
    }
    return 0;
}

int MoveWelcomeFolderToTarget(void)
{
    char command[MAXURL] = {0};
    strcat(command, "mv ");
    strcat(command, pMXData->targetPath);
    strcat(command, "welcome ");
    strcat(command, pMXData->tmpPath);
    strcat(command, "welcome.bak");
    if(0 != system(command))
    {
        mxLog("command ERR:%s", command);
    }
    memset(command, 0, MAXURL);
    strcat(command, "mv ");
    strcat(command, pMXData->tmpPath);
    strcat(command, "welcome ");
    strcat(command, pMXData->targetPath);
    if(0 != system(command))
    {
        mxLog("command ERR:%s", command);
        return -1;
    }
    return 0;
}

int MoveExtFilesToTarget(void)
{
    FILE *extMoveFile = NULL;
    if(NULL == (extMoveFile = fopen(g_ExtMoveFilePath, "r")))
    {
        mxLog("open failed:%s", g_ExtMoveFilePath);
    }
    char command[MAX_CHAR_ONE_LINE] = {0};
    memset(command, 0, MAX_CHAR_ONE_LINE);
    strcat(command, "mv -f ");
    strcat(command, pMXData->extPath);
    int headLength = strlen(command);
    int errStatus = 0;
    while(NULL != fgets(command + headLength, MAX_CHAR_ONE_LINE, extMoveFile))
    {
        if(0 != system(command))
        {
            mxLog("command failed:%s", command);
            errStatus = -1;
        }
    }
    
    fclose(extMoveFile);

    // clean up ext directorys
    memset(command, 0, MAX_CHAR_ONE_LINE);
    strcat(command, "rm -rf ");
    strcat(command, pMXData->extPath);
    if(0 != system(command))
    {
        mxLog("command failed:%s", command);
    }
    
    return errStatus;
    
}


int StringRandom(char *string, int lengh)
{
    if (NULL == string || lengh < 1)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    srand(time(NULL));
    for (int i = 0; i < lengh-1; i++)
    {
        string[i] = ('a' + rand() % 26);
    }
    string[lengh -1] = '\0';
    
    return 0;
}


int ReportJson(int port, char *hostname, char *reportPath, char *encryptMac, char *json)
{

    const char * post           = "POST ";
    const char * postHttp       = " HTTP/1.1\r\nHost: ";
    const char * postHead1      = "\r\n"
                                  "Connection: keep-alive\r\n"
                                  "Cache-Control: no-cache\r\n"
                                  "Origin: mxrj\r\n"
                                  "Content-Type: multipart/form-data; boundary=";
    const char * postHead2      = "\r\nAccept: */*\r\n"
                                  "Accept-Encoding: gzip, deflate\r\n"
                                  "Accept-Language: en-US,en;q=0.8\r\n"
                                  "Content-Length: ";
    const char * crlf           = "\r\n";
    const char * postbodyId     = "\r\nContent-Disposition: form-data; name=\"id\"\r\n\r\n";
    const char * postbodyData   = "\r\nContent-Disposition: form-data; name=\"data\"\r\n"
                                  "Content-Transfer-Encoding: binary\r\n\r\n";

    char boundaryName[16] = {0};

    if (NULL == hostname || NULL == reportPath || NULL == encryptMac || NULL == json)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    
    StringRandom(boundaryName, sizeof(boundaryName));
    boundaryName[0] = '-';    
    boundaryName[1] = '-';    
//    mxLog("DBUG BoundaryName is:%s", boundaryName);

     
    unsigned int    jsonLen         = strlen(json);
    int             macLen          = strlen(encryptMac);
    int             boundaryNameLen = strlen(boundaryName);
    
    int postBodyLen = macLen                    + jsonLen 
                    + strlen(postbodyId)        + strlen(postbodyData) 
                    + 3 * boundaryNameLen       + 3 * strlen(crlf) + 2;
    
    char contentLen[5] = {0};
    sprintf(contentLen, "%d", postBodyLen);    
    
    int postMsgLen =  strlen(post)              + strlen(reportPath)  
                    + strlen(postHttp)          + strlen(hostname) 
                    + strlen(postHead1)         + strlen(postbodyId) 
                    + strlen(postHead2)         
                    + jsonLen                   + macLen
                    + strlen(postbodyData)      + strlen(contentLen)
                    + 4 * boundaryNameLen       + 5 * strlen(crlf);

    unsigned char * postMessage = (unsigned char *)malloc(postMsgLen);
    if (NULL == postMessage)
    {
        mxLog("No Mem For PostMessage. Error!!");
        return -1;
    }
//    mxLog("DBUG Post Message Lengh:%d", postMsgLen);
    
    memset(postMessage, 0, postMsgLen);
    
    strcat((char *)postMessage, post);
    strcat((char *)postMessage, reportPath);
    strcat((char *)postMessage, postHttp );
    strcat((char *)postMessage, hostname);
    strcat((char *)postMessage, postHead1);    
    strcat((char *)postMessage, boundaryName+2);    
    strcat((char *)postMessage, postHead2);    
    strcat((char *)postMessage, contentLen);
    strcat((char *)postMessage, crlf);    
    strcat((char *)postMessage, crlf); 
//    mxLog("DBUG before lengh:%d", (int)strlen((const char *)postMessage));
    strcat((char *)postMessage, boundaryName);    
    strcat((char *)postMessage, postbodyId);
    strcat((char *)postMessage, encryptMac);    
    strcat((char *)postMessage, crlf);    
    strcat((char *)postMessage, boundaryName);    
    strcat((char *)postMessage, postbodyData);
    

    unsigned char * encryptJson = (unsigned char * )json;
    if (0 != EncryptJson(json, encryptJson, jsonLen))
    {
        mxLog("EncryptJson faile!");
        free(postMessage);
        return -1;
    }
  
    int offset = strlen((const char *)postMessage);
    memcpy(postMessage + offset, encryptJson, jsonLen);
    
    offset = offset + jsonLen;
    memcpy(postMessage + offset, crlf, strlen(crlf));
    
    offset = offset + strlen(crlf);    
    memcpy(postMessage + offset, boundaryName, boundaryNameLen);
    

    offset = offset + boundaryNameLen;
    memcpy(postMessage + offset, "--", 2);

    offset += 2;
    memcpy(postMessage + offset, crlf, strlen(crlf));

    int ret = Sending(port, hostname, postMessage, postMsgLen);

    free(postMessage);

    return ret;
    
}




int Sending(int port, char *hostname, unsigned char *request, int requestLen)
{
    int sockfd, bytes, sent;
    int result = 0;
    char response[64] = {0};
    struct hostent  *   server;
    struct sockaddr_in  serv_addr;


    if (NULL == hostname || NULL == request)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    
#if 0
    mxLog("DBug Request:");
    for (int i = 0; i < requestLen; i++)
     {   printf("%c", *(request+i));}
    mxLog("DBug Request end.");
#endif

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        mxLog("ERROR opening socket");
        return -1;
    }
    
    /* lookup the ip address */
    server = gethostbyname(hostname);
    if (NULL == server)
    {
        mxLog("ERROR! Unable to get host IP!");        
        close(sockfd);
        return -1;
    }
    
    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family    = AF_INET;
    serv_addr.sin_port      = htons(port);
    
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    /* connect the socket */
    if ( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
    {
        mxLog("ERROR connecting");
    }
    
    /* send the request */
    mxLog("Send total:%d", requestLen);
    sent = 0;
    do
    {
        bytes = send(sockfd, request + sent, requestLen - sent, 0);
        if (bytes < 0)
        {
            mxLog("ERROR writing message to socket");
        }
        
        if (0 == bytes)
        {
            break;  //finish send  
        }
        sent += bytes;
    } while (sent < requestLen);
    mxLog("Send success:%d", sent);
    
    /* receive the response */
    memset(response, 0, sizeof(response));
   
    bytes = recv(sockfd, response, 64, 0);
    if (bytes <= 0)
    {
        mxLog("ERROR reading response from socket");
    }
    else 
        {
            mxLog("Response:\n%s", response);
            result = FindSubStrInString((const char* )response, "200 OK");               
        }

     
    /* close the socket */
    close(sockfd);
    
    return result;
}



int EncryptJson(char *json, unsigned char *encryptjson, unsigned int size)
{
    unsigned char keys[64] = {
        0xaf, 0xef, 0xff, 0x11, 0x23, 0xf6, 0x90, 0x0f,
        0xbf, 0xfe, 0xef, 0x22, 0x32, 0x89, 0xcc, 0xe1,
        0x19, 0x20, 0xf1, 0xe2, 0x33, 0x00, 0xe9, 0xe9,
        0xa9, 0xbb, 0xf3, 0x99, 0x11, 0x22, 0x33, 0x44,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xd2, 0xee,
        0xe9, 0xc9, 0xa9, 0xb9, 0xd9, 0xd1, 0x1d, 0xe1,
        0x1e, 0xf1, 0x1f, 0xdd, 0x93, 0x16, 0x64, 0x32 };
    
    if (NULL == json || NULL == encryptjson)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    unsigned char blockNumber = size / 64;
    unsigned char restByte    = size % 64;
    while(blockNumber--)
    {
        for(int index = 0; index < 64; index++)
        {
            *(encryptjson++) = *(json++) ^ *(keys + index);
        }
    }
    
    while(restByte--)
    {
        *(encryptjson + restByte) = *(json + restByte) ^ *(keys + restByte);
    }
    
    return 0;
}


/*********************************************************************
**独立上报接口
***********************************************************************/
int GetReportMAC(char * MACFile, char *mac)
{    
    char   buf[36] = {0};   //mac address normally is 12 char, triple for safe.
    char * p       = buf;
    FILE * fp      = NULL;

    if (NULL == MACFile || NULL == mac)
    {
        mxLog("Function parameter error!");
        return -1;
    }

    if ((fp = fopen(MACFile, "r")) == NULL)
    {
        mxLog("Open macfile failed");
        return -1;
    }
    
    fgets(buf, sizeof(buf), fp);
    mxLog("Get MAC buf is:%s", buf);

    int machar = 0;   
    while ('\0' != *p && machar < 12)
    {
        if ((*p >= '0' && *p<= '9') || ( *p >= 'a' && *p <= 'f'))
        {
            *mac = *p;
            mac++;
            machar++;
        }
        p++;
    }    
    
    *mac = '\0';
    if(12 != machar)
    {
        mxLog("GetMAC Wrong MAC: %s", buf);
        return -1;
    }        

    if (fclose(fp) < 0)
    {
        mxLog("Close macFile failed!");
        return -1;
    }
    return 0;
}


int GetCPUNumber(int * number)
{
    if (NULL == number)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    FILE * fp = popen("cat /proc/cpuinfo|grep processor|wc -l", "r");
    if (NULL == fp)
    {
        mxLog("Error!");
        return -1;
    }
    if (1 != fscanf(fp, "%d", number))
    {
        pclose(fp);
        mxLog("Get cpu number failed!");
        return -1;
    }    
    pclose(fp);
    return 0;

}

int VerifyHWInfo(char * hwInfo, char *hwName)
{
    if (NULL == hwInfo || NULL == hwName)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    
    if (0 != strcmp(hwInfo, hwName))
    {
        return 1;
    }
    return 0;
}


int CreateHeartJson(char * json, int ccc, unsigned long mem[], float cpu[], int cpuNumber, float sysload[])
{
    char cpuBuf[64] = "\"cpu\":[";

    if (NULL == json || NULL == mem ||NULL == cpu || NULL == sysload)
    {
        mxLog("Create Heart Json error!");
        return -1;
    }

    for (int i = 0; i < cpuNumber; i++)
    {
        sprintf(cpuBuf + strlen(cpuBuf), "\"%4.2f\",", *(cpu+i));
    }
    cpuBuf[strlen(cpuBuf) -1] = '\0';
    strcat(cpuBuf, "]}");
    

    sprintf(json, "{\"type\":\"0x0001\",\"ccc\":%d,\"mem\":[\"%lu\",\"%lu\",\"%lu\",\"%lu\",\"%lu\"],\"sysload\":[\"%4.2f\",\"%4.2f\",\"%4.2f\"],", 
                    ccc, mem[0], mem[1], mem[2], mem[3], mem[4], 
                    sysload[0], sysload[1], sysload[2]);
    
    strcat(json, cpuBuf);

    return 0;

}



/******************************
 * Name : ConnectNum
 * Return: Success: Number of connections
 *          Failed: -1
 * Author: Alex HUA
 * Description:
 ***********************************/
int ConnectNum (int *ccc)
{
    if (NULL == ccc)
    {
        mxLog("Function parameter error!");
        return -1;
    }

    FILE *fp = popen("netstat -nt | grep tcp | awk '{print $5}' | cut -d ':' -f 1 | sort | uniq -c | wc -l", "r");
    if (NULL == fp)
    {
        mxLog("Error!");
        return -1;
    }
    if ( 1 != fscanf(fp, "%d", ccc))
    {
        pclose(fp);
        mxLog("Get connet number failed!");
        return -1;
    }

    pclose(fp);
    return 0;

}



/******************************
 * Name : SystemLoad
 * Return: Success: 0
 *          Failed: other
 * Author: Alex HUA
 * Description:
 ***********************************/
int SystemLoad (float sysload[])
{
    if (NULL == sysload)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    //mips    uptime | cut -d "," -f 3,4,5 | sed 's/load average://g'
    // 22:45:46 up 1 day,  2:35,  load average: 0.07, 0.05, 0.04
    //x86_64  uptime | cut -d "," -f 3,4,5 | sed 's/load average://g' | sed 's/,//g'
    // 08:51:26 up 11 min,  2 users,  load average: 0.00, 0.01, 0.04
    
    //uptime | cut -d ":" -f 4 | sed 's/,//g'
    FILE *fp = popen("uptime | awk -F 'load average: ' '{print $2}' | sed 's/,//g'", "r");
    if (NULL == fp)
    {
        mxLog("Error!");
        return -1;
    }
    if ( 3 != fscanf(fp, "%f%f%f", sysload, sysload+1, sysload+2))
    {
        pclose(fp);
        return -1;
    }
    pclose(fp);
    return 0;
   
}



/******************************
 * Name : CpuUsage
 * Return: Success: 0
 *          Failed: -1
 * Author: Alex HUA
 * Description:
 ***********************************/
int CpuUsage(char * hwInfo, float cpu[], int cpuNumber)
{
    FILE *fp = NULL;
    
    if (NULL == hwInfo || NULL == cpu)
    {
        mxLog("Function parameter error!");
        return -1;
    }    

    //mips    top -n 1 | grep CPU | grep -v grep | grep -v PID | awk '{print $4}' | sed 's/%//g'
    //CPU:    8% usr   8% sys   0% nic  83% idle   0% io   0% irq   0% sirq
    //x86_64  top -b -n 1 | grep Cpu | awk '{print $4}'
    //%Cpu(s):  0.1 us,  0.3 sy,  0.0 ni, 99.4 id,  0.1 wa,  0.0 hi,  0.0 si,  0.0 st
    
    if (0 == VerifyHWInfo(hwInfo, "x86"))   // HWInfo : x86
    {
        fp = popen("top -b -n 1 | grep Cpu | sed 's/[a-z][a-z],//g' | cut -d ':' -f 2 | awk '{print $1}'", "r");
    }
    else if (0 == VerifyHWInfo(hwInfo, "mips"))   // HWInfo : mips
    {
        fp = popen("top -n 1 | grep CPU | grep -v grep | grep -v PID | awk '{print $4}' | sed 's/%//g'", "r");
    }
    else
    {
        mxLog("Unknow Hardware!");
        return -1;
    }

    if (NULL == fp)
    {
        mxLog("Error!");
        return -1;
    }

    for (int i = 0; i < cpuNumber; i++)
    {
        if ( 1 != fscanf(fp, "%f", cpu+i))
        {
            pclose(fp);
            return -1;

        }
        mxLog("Cpu%d(us):%4.2f", i, cpu[i]);
    }    

    pclose (fp);
    return 0;
    
}



/******************************
 * Name : MemoryUsage
 * Return: Success: 0
 *          Failed: -1
 * Author: Alex HUA
 * Description:
 ***********************************/
int MemoryUsage (unsigned long mem[])
{
    if (NULL == mem)
    {
        mxLog("Function parameter error!");
        return -1;
    }

    //mips     free | grep Mem | sed 's/Mem://g'
    //             total         used         free       shared      buffers
    //Mem:        255320       211440        43880            0        22116
    //x86      free | grep Mem
    //              total        used        free      shared  buff/cache   available
    //Mem:        1884376      102004     1608144        8524      174228     1632024
    
    FILE *fp = popen("free | grep Mem | sed 's/Mem://g'", "r");
    if (NULL == fp)
    {
        mxLog("Error!");
        return -1;
    }
    if ( 5 != fscanf(fp, "%lu%lu%lu%lu%lu", mem, mem+1, mem+2, mem+3, mem+4))
    {
        pclose(fp);
        return -1;
    }

    pclose(fp);
    return 0;
  
}




int NginxServiceCheck(Report_Data *reportData)
{
    const char * file = "bsldata/index.android.json";
    char srcFile[128] = {0};
    char command[128] = {0};
    
    if (NULL == reportData)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    strcat(srcFile, reportData->targetPath);
    strcat(srcFile, file);
    mxLog("Local source file is \"%s\"", srcFile);
    
    memset(command, 0, sizeof(command));
    system("rm -rf TestNginxFile");

    if (0 == VerifyHWInfo(reportData->HWInfo, "x86"))   //x86
    {
        strcat(command, "wget 192.168.100.2/");    
    }
    else if (0 == VerifyHWInfo(reportData->HWInfo, "mips"))   //mips
    {
        strcat(command, "wget 192.168.1.1/");
    }
    else
    {
        mxLog("Unknow Hardware!");
        return -1;
    }
    strcat(command, file);
    strcat(command, " -O ./TestNginxFile");
    mxLog("Download command is:%s", command);
    if (0 != system(command))    //wget 192.168.1.1/bsldata/index.android.json -O ./TestNginxFile
    {
        mxLog("Local download Failed!");
    }

    if (0 != CheckTwoFileMD5(srcFile, "./TestNginxFile"))
    {
        return 1;
    }
    return 0;
  
}




int CreateFaultJson(int typeNumber, char * json)
{
    char    cfvBuf[32]  = {0};
    char    cpgvBuf[32] = {0};

    if (NULL == json)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    
    if (0 != GetVersion("/mx/etc/localNginxVersion.conf", cfvBuf))
    {
        mxLog("Get local Nginx version failed!");
        return -1;
    }
    mxLog("Local Nginx version: %s", cfvBuf);

    if (0 != GetVersion("/mx/etc/localVersionFile.conf", cpgvBuf))
    {
        mxLog("Get local resource version failed!");
        return -1;
    }    
    mxLog("Local resource version: %s", cpgvBuf);
    
    sprintf(json, "{\"type\":\"0x000%x\",\"cfv\":\"%s\",\"cpgv\":\"%s\"}", typeNumber, cfvBuf, cpgvBuf);
    
    return 0;
}




int OverWriteStringToFile(char * filePath, char * string)
{
    FILE * fp = NULL;

    if (NULL == filePath || NULL == string)
    {
        mxLog("Function parameter error!");
        return -1;
    }
    
    fp = fopen(filePath, "w+");
    if (NULL == fp)
    {
        mxLog("fopen error!");
        return -1;
    }
    
    int writeNumber = fputs(string, fp);
    if (0 != fclose(fp))
    {
        mxLog("fclose error!");
    }
    
    if (strlen(string) != (size_t)writeNumber)
    {
        return -1;
    }

    return 0;       
}






int CheckTwoFileMD5(const char *fileName1, const char *fileName2)
{
    char md5File1[33] = {0};
    char md5File2[33] = {0};

    if (NULL == fileName1 || NULL == fileName2)
    {
        mxLog("Function parameter error!");
        return -1;
    }

    if ( 0 != (access(fileName1, F_OK)))
    {
        mxLog("\"%s\" is not exist!", fileName1);
        return -1;
    }
    if ( 0 != (access(fileName2, F_OK)))
    {
        mxLog("\"%s\" is not exist!", fileName2);
        return -1;
    }

    CalcFileMD5(fileName1, md5File1);
    mxLog("\"%s\" MD5 is:%s", fileName1, md5File1);
    
    CalcFileMD5(fileName2, md5File2);
    mxLog("\"%s\" MD5 is:%s", fileName2, md5File2);

    if (0 != strcmp(md5File1, md5File2))
    {
        return 1;
    }
    
    return 0;
}



	
#undef _MXLIB_C_


