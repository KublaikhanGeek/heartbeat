
#define BFUPDATEAPI_C

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "bfupdateapi.h"
#include "mxlib.h"
#include "mxcppapi.h"
#include "md5.h"

extern mx_Data *pMXData;



/***********************************
*Name: GetMAC
*Usage: Read MACFile
*Input: void
*Output: mac string
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int GetMAC(char *mac)
{
    FILE * fp       = NULL;
    char   buf[36]  = {0}; //mac address normally is 12 char, triple for safe.
    char * p        = buf;

    if (NULL == mac)
    {
    	mxLog("mac is null");
        return -1;
    }

    if ((fp = fopen(pMXData->macFile, "r")) == NULL)
    {
		mxLog("open macfile failed");
        return -1;
    }
    mxLog("open macfile OK");

    fgets(buf, sizeof(buf), fp);
    mxLog("GetMAC : %s", buf);

    int machar = 0;   
    while ('\0' != *p && machar < 12)
    {
        if ((*p >= '0' && *p <= '9') || ( *p >= 'a' && *p <= 'f'))
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
        mxLog("Close confile failed!");
        return -1;
    }
    return 0;
}


/***********************************
*Name: EncryptMAC
*Usage: 
*Input: 
*Output: 
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int EncryptMAC(char *mac, char *EncryptMAC)
{
    unsigned char keys[13]      = {0xcc, 0xcb, 0xa1, 0xfe, 0xaf, 0x9a, 0x8b, 0x33, 0xde, 0xee, 0x52, 0xff, 0x00};	
    unsigned char result[12]    = {0};
    unsigned char randValue[4];
    unsigned char finalValue[16];

    if (NULL == mac || NULL == EncryptMAC || 12 != strlen(mac))
    {
    	mxLog("char * is NULL");
        return -1;
    }
	mxLog("keys=%s", keys);
    //step A;
    for (int i = 0; i < 12; i++)
    {
        result[i] = mac[i] ^ keys[i];
        //mxLog("result=%X,  mac=%X, key=%X, i=%X", result[i],mac[i], keys[i], i);
    }

	//Step B;
	// rand result max is 0x7FFF
	srand((unsigned int)time(NULL)); 
	randValue[0] = (unsigned char)rand();
	randValue[1] = (unsigned char)rand();
	randValue[2] = (unsigned char)rand();
	randValue[3] = (unsigned char)rand();

	//Step C
	for (int i = 0; i < 3; i++) //12 /4 = 3 times
    {
    	for(int j = 0; j < 4; j++)
		{
			result[i*4 + j] = randValue[j] ^ result[i*4 + j];
            //mxLog("result=%X,  randV=%X, index=%d", result[i*4 + j],randValue[j], i*4 + j);
		}
	}

	//Step D
	memcpy(finalValue, randValue, 4);
	memcpy(finalValue+4, result, 12);

	//Step E
	for(int i = 0; i < 16; i++)
	{ 
		OneByteToTwoChar(finalValue[i], EncryptMAC);
		EncryptMAC += 2;
        //mxLog("finalValue[i]=0x%X, EncryptMAC=%c and %c", finalValue[i], *EncryptMAC, *(EncryptMAC+1));
	} 
	*EncryptMAC = '\0';

    return 0; 

}

/***********************************
*Name: OneByteToTwoChar
*Usage: 
*Input: 
*Output: 
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int  OneByteToTwoChar(unsigned char value, char *twoChar)
{
    if (NULL == twoChar)
    {
    	mxLog("towChar is NULL");
        return -1;
    }

    //char 1
    unsigned char ch =  (unsigned char)((value & 0xF0) >> 4 ) & 0x0F;
    if( 9 >= ch ) // 0~9
    {
        *(twoChar) = ch + '0';
    }
    if( 0xA <= ch && ch <= 0xF )  // a~f
    {
        *(twoChar) = ch + 'a' - 10;
    }

    //get  char 2
    ch = (unsigned char)(value & 0x0F);
    if( 9 >= ch ) // 0~9
	{
		*(twoChar+1) = ch + '0';
	}
	if( 0xA <= ch && ch <= 0xF )  // a~f
	{
		*(twoChar+1) = ch + 'a' - 10;
	}

	//get 2 char
    
    return 0;
}




/***********************************
*Name: GetCurrentVersion
*Usage: to get bapfeng net Version
*Input: void
*Output: mac string
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int AssembleJsonURL(const char* urlhead, const char *id, char *JsonUrl)
{
    if (NULL == JsonUrl || NULL == id)
    {
    	mxLog("jsonUrl or id is NULL");
        return -1;
    }
    memset(JsonUrl, 0, MAXURL*sizeof(char));
    strcpy(JsonUrl, urlhead);
    strcat(JsonUrl, "?id=");
    strcat(JsonUrl, id);
    return 0;
}


/***********************************
*Name: GetCurrentVersion
*Usage: to get bapfeng net Version
*Input: void
*Output: mac string
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int DownJson(const char *JsonUrlAddr, const char* jsonFileName)
{
    char command[MAX_CHAR_ONE_LINE] = {0};

    if (NULL == JsonUrlAddr)
    {
    	mxLog("jsonUrlAddr is NULL");
        return -1;
    }
    strcat(command, "wget ");
    strcat(command, JsonUrlAddr);
    strcat(command, " -O  ");
    strcat(command, jsonFileName);
    
    mxLog("command is: %s", command);

    if ( 0 != system(command)) 
    {
        mxLog("Download JSON Failed");
    }
    
    return 0;
}



/***********************************
*Name: GetCurrentVersion
*Usage: to get bapfeng net Version
*Input: void
*Output: mac string
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int GetVersion(const char *VERFile, char *versionNum)
{
    FILE * fp       = NULL;
    char * buf      = NULL;	
    char * p        = NULL;
    long   bufsize  = 0;

    if (NULL == VERFile || NULL == versionNum)
    {
    	mxLog("VERFile or versionNum is NULL");
            return -1;
    }

	bufsize = getFileSize(VERFile) +1;
    
    if(bufsize < 1)
    {
        mxLog("ERR! getFileSize");
        return -1;
    }
	buf = (char *)malloc(bufsize);
    memset(buf, 0, bufsize);
    
	if(0==bufsize || NULL ==buf)
	{
		mxLog("bufsize or buf is NULL");
		return -1;
	}

    if ((fp = fopen(VERFile, "r")) == NULL)
    {
        mxLog("Can not open macFile!");
		free(buf);
		mxLog("open VERFile failed");
        return -1;
    }

    fgets(buf, bufsize, fp);

	p = buf;
    while ('\0' != *p )
    {
        if (*p >= '0' && *p <= '9')
        {
            *versionNum++ = *p;
        }
        p++;
    }    
	
    if (fclose(fp) < 0)
    {
        mxLog( "ERROR: Close confile failed!");
		free(buf);
        return -1;
    }
	
	free(buf);
    return 0;
}


/***********************************
*Name: GetID
*Usage: to get bapfeng net Version
*Input: void
*Output: mac string
*Return:  0: success. -1: fail
*Comment: 
**************************************/
int GetID(const char *IDFile, char *id)
{
    FILE * fp       = NULL;
    char * buf      = NULL;
	long   bufsize  = 0;
    
    if (NULL == IDFile || NULL == id)
    {
    	mxLog("IDFile or id is NULL");
        return -1;
    }

	bufsize = getFileSize(IDFile) +1;
	buf = (char *)malloc(bufsize);
	memset(buf, 0, bufsize);
    
	if(0==bufsize || NULL ==buf)
	{
		mxLog("bufsize or buf is NULL");
		return -1;
	}

    if ((fp = fopen(IDFile, "r")) == NULL)
    {
		free(buf);
		mxLog("open IDFile failed");
        return -1;
    }

    fgets(buf, bufsize, fp);
    char *p = buf;
    while ('\0' != *p && *p != '\n')
    {
        *id++ = *p++;
    }
    mxLog("GetCurrentVersion : %s", buf);
	
    if (fclose(fp) < 0)
    {
        mxLog("ERROR: Close confile failed!");
		free(buf);
        return -1;
    }
	free(buf);
    return 0;


}

int DownloadNGXFile(const char *url, const char* md5, const char* pathname)
{
    char command[MAXURL] = {0};

    if (NULL == url || NULL == pathname || NULL == md5 )
    {
    	mxLog("url or pathname or MD5 is NULL");
        return -1;
    }

    strcat(command, "wget ");
    strcat(command, (const char *)url);
    strcat(command, " -O  ");
    strcat(command, (const char *)pathname);
	mxLog("DOWNLOAD command: %s", command);

	//download 
    if ( 0 != system(command)) 
    {
        mxLog("Download Nginx File Failed");
    }
	//verify
    return CheckMD5(pathname, md5);  
}

/***********************************
*Name: CheckUpdate
*Usage: 
*Input: bfUpdateJson: Json file in mem
*Output: N/A
*Return:  0: need update. -1: no update
*Comment: 
**************************************/
int DownloadResourceFile(const char *url, const char* pathname)
{
	char md5[33]           = {0};
    char command[MAXURL]   = {0};

    if (NULL == url || NULL == pathname)
    {
    	mxLog("url or pathname is NULL");
        return -1;
    }

    strcat(command, "wget ");
    strcat(command, (const char *)url);
    strcat(command, " -O  ");
    strcat(command, (const char *)pathname);
	mxLog("DOWNLOAD command: %s", command);

	//download 
    if ( 0 != system(command)) 
    {
        mxLog("Download Resource File Failed");
    }
	//verify
	GetMD5FromJsonPart(pathname, md5);
    return CheckMD5(pathname, md5);  

}


int CheckMD5(const char *namePath, const char *md5)
{
    char calcMD5[33] = {0};
    CalcFileMD5(namePath, calcMD5);
    return strcmp(md5, calcMD5);
}


void GetMD5FromFile(const char *file, char *outMD5)
{
    FILE *fp = NULL;
    
    if (NULL == file || NULL == outMD5)
    {
    	mxLog("file or outMD5 is NULL");
        return;
    }

    if ((fp = fopen(file, "r")) == NULL)
    {
	    mxLog("open file failed");
        return;
    }

    fgets(outMD5, 33, fp);
    if (fclose(fp) < 0)
    {
        mxLog("ERROR: Close confile failed!");
        return;
    }

}

int MoveFile(const char * srcNamePath,  const char * targetNamePath)
{
    char command[MAXURL]  = {0};
    char temppath[MAXURL] = {0};

    if (NULL == srcNamePath || NULL == targetNamePath )
    {
    	mxLog("srcNamePath or targetNamePath is NULL");
        return -1;
    }
    
    memset(temppath, 0, sizeof(temppath));
    strcpy(temppath, (const char *)targetNamePath);
 //   mxLog("temppath before: %s",temppath);
    for(int i=strlen(temppath); i>0;i--)
    {
        if(temppath[i-1] == '/')
        {
            temppath[i-1] = '\0';
            break;
        }
    }
//    mxLog("temppath after: %s",temppath);
    
    // prepare dir
    memset(command, 0, sizeof(command));
    strcat(command, "mkdir -p "); 
    strcat(command, temppath);
   
    mxLog("Mkdir command is: %s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Make Directory Failed");
    }
    //mkdir -p root/Folder/iso/bsldata/76/be/765564bb2f3787c6d00964697383e1be

    // move file
    memset(command, 0, sizeof(command));
    strcat(command, "mv -f "); 
    strcat(command, (const char *)srcNamePath); 
    strcat(command, " ");
    strcat(command, (const char *)targetNamePath);
    mxLog("MoveFile command: %s", command);
    if ( 0 != system(command)) 
    {
        mxLog("Move welcome.zip Failed");
    }
    
    return 0;
}
    

int GetHWInfo(const char *confile, char *info)
{
    FILE *fp   = NULL;

    if (NULL == confile || NULL == info)
    {
        return -1;
    }

    if ((fp = fopen(confile, "r")) == NULL)
    {
        mxLog("Can not open confile!");
        return -1;
    }
   
    fgets(info, MAXHWINFOLEN, fp);

    //clean up string
    for(unsigned int i = 0; i < strlen(info); i++)
    {
        if( '\n' == info[i])
        {
            info[i] = '\0';
        }
    }
          
    if (fclose(fp) < 0)
    {
        mxLog("ERROR: Close confile failed!");
        return -1;
    }
    return 0;
}


int DecryptionFile(const char *Encryptfile, const char *Decryptedfile)
{
    int ret = 0;
    unsigned char tmpByte;
    unsigned char keys[64] = {
        0xaf, 0xef, 0xff, 0x11, 0x23, 0xf6, 0x90, 0x0f,
        0xbf, 0xfe, 0xef, 0x22, 0x32, 0x89, 0xcc, 0xe1,
        0x19, 0x20, 0xf1, 0xe2, 0x33, 0x00, 0xe9, 0xe9,
        0xa9, 0xbb, 0xf3, 0x99, 0x11, 0x22, 0x33, 0x44,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xd2, 0xee,
        0xe9, 0xc9, 0xa9, 0xb9, 0xd9, 0xd1, 0x1d, 0xe1,
        0x1e, 0xf1, 0x1f, 0xdd, 0x93, 0x16, 0x64, 0x32 };
    FILE *encrfile = NULL;
    FILE *targfile = NULL;
    

    if (NULL == Encryptfile || NULL == Decryptedfile)
    {
        //error 
        return -1;
    }

    if (NULL == (encrfile = fopen(Encryptfile, "rb")))
    {
        mxLog("ERROR Can not open Encryptfile!");
        return -1;
    }
    if (NULL == (targfile = fopen(Decryptedfile, "wb")))
    {
        mxLog("ERROR Can not open fileName!");
        fclose(encrfile);
        return -1;
    }

    while(!feof(encrfile))
    {
        for (int count = 0; count < 64; count++)
        {
        	
            if(0 != fread(&tmpByte, 1, 1, encrfile))
            {
                tmpByte = tmpByte ^ keys[count];
                fwrite(&tmpByte, 1, 1, targfile);
            }
        }
    }

    if (fclose(encrfile) < 0)
    {
        mxLog("ERROR: Close Encryptfile failed!");
        ret = -1;
    }
    if (fclose(targfile) < 0)
    {
        mxLog("ERROR: Close Encryptfile failed!");
        ret = -1;
    }
    
    return ret;
}




int UpdateNginxVersion(char * versionFile)
{
    FILE *fp = NULL;

    if(NULL == versionFile)
    {
        mxLog("ERROR: open Nginx File Fail!");
        return -1;
    }
    if (NULL == (fp = fopen(versionFile, "w+")))
    {
        mxLog("ERROR Can not open fileName!");
        return -1;
    }
    fputs(pMXData->ngxData.jsonVersion, fp);
    
    if (fclose(fp) < 0)
    {
        mxLog("ERROR: Close NginxVersion File failed!");
    }
return 0;
    
}



int ReportDownloadResourceStart(mx_Data *pD)
{
	char json[] = "{\"type\":\"0x0002\"}\0";
    
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0002 to bf failed");
		return -1;
	}
	mxLog("Report type=0x0002 to bf success");
	return 0;
}


int ReportResourceDownloadFinish(mx_Data *pD)
{
	char json[] = "{\"type\":\"0x0003\"}\0";
    
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0003 to bf failed");
		return -1;
	}
	mxLog("Report type=0x0003 to bf success");
	return 0;
}


int ReportResourceDownloadFailed(mx_Data *pD)
{
	char json[] = "{\"type\":\"0x0004\"}\0";
    
	mxLog("Report to bf's json is:%s", json);
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0004 to bf failed");
		return -1;
	}
	mxLog("Report type=0x0004 to bf success");
	
	return 0;
}


int ReportStartUpdateResource(mx_Data *pD)
{
	char json[] = "{\"type\":\"0x0005\"}\0";
    
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0005 to bf failed");
		return -1;
	}
	mxLog("Report type=0x0005 to bf success");

	return 0;
}


int ReportStartUpdateFireware(mx_Data *pD)
{
	char *jsonFormat = "{\"type\":\"0x0006\",\"cfv\":\"%s\"}\0";
	
	char *json = (char *) malloc (strlen(jsonFormat) - 2 + strlen(pD->ngxData.localVersion) + 1);
	if(NULL == json)
	{
		return -1;
		mxLog("malloc failed");
	}
	
	sprintf(json, jsonFormat, pD->ngxData.localVersion);
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0006 to bf failed");
		return -1;
	}
	free(json);
	mxLog("Report type=0x0006 to bf success");
	
	return 0;
}


int ReportUpdateResourceSuccess(mx_Data *pD)
{
	char *jsonFormat = "{\"type\":\"0x0007\",\"pkgv\":\"%s\"}\0";
	
	char *json = (char *) malloc (strlen(jsonFormat) - 2 + strlen(pD->resData.localVersion) + 1);
	if(NULL == json)
	{
		return -1;
		mxLog("malloc failed");
	}
	
	sprintf(json, jsonFormat, pD->resData.JsonVersion);
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort),  pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0007 to bf failed");
		return -1;
	}
	free(json);
	mxLog("Report type=0x0007 to bf success");
	
	return 0;
}


int ReportUpdateResourceFailed(mx_Data *pD)
{
	char *jsonFormat = "{\"type\":\"0x0008\",\"cfv\":\"%s\",\"pkgv\":\"%s\"}\0";

	char *json = (char *) malloc (strlen(jsonFormat) - 4 + strlen(pD->ngxData.localVersion) + strlen(pD->resData.localVersion) + 1);
	if(NULL == json)
	{
		return -1;
		mxLog("malloc failed");
	}
	
	sprintf(json,jsonFormat, pD->ngxData.localVersion, pD->resData.localVersion);
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x0008 to bf failed");
		return -1;
	}
	free(json);
	mxLog("Report type=0x0008 to bf success");
	
	return 0;
}


int ReportUpdateFirewareSuccess(mx_Data * pD)
{
	char *jsonFormat = "{\"type\":\"0x000b\",\"cfv\":\"%s\"}\0";
	
	char *json = (char *) malloc (strlen(jsonFormat) - 2 + strlen(pD->ngxData.localVersion) + 1);
	if(NULL == json)
	{
		return -1;
		mxLog("malloc failed");
	}
	
	sprintf(json, jsonFormat, pD->ngxData.jsonVersion);
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x000b to bf failed");
		return -1;
	}
	free(json);
	mxLog("Report type=0x000b to bf success");
	
	return 0;
}


int ReportUpdateFirewareFailed(mx_Data * pD)
{
	char *jsonFormat = "{\"type\":\"0x000c\",\"cfv\":\"%s\"}\0";
	
	char *json = (char *) malloc (strlen(jsonFormat) - 2 + strlen(pD->ngxData.localVersion) + 1);
	if(NULL == json)
	{
		return -1;
		mxLog("malloc failed");
	}
	
	sprintf(json, jsonFormat, pD->ngxData.localVersion);
	mxLog("Report to bf's json is:%s", json);
	
	//send to bf
	if(0 != ReportJson(atoi(pD->reportPort), pD->reportDomain, pD->reportPath, pD->encryptMAC, json))
	{
		mxLog("Report type=0x000c to bf failed");
		return -1;
	}
	free(json);
	mxLog("Report type=0x000c to bf success");
	
	return 0;
}



#undef BFUPDATEAPI_C


