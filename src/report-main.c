#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "bfupdateapi.h"
#include "mxlib.h"
#include "mxcppapi.h"



Report_Data * reportData;
mx_Data     * pMXData;


int InitialReportMain(Report_Data * reportData);
int HeartReport(Report_Data * reportData);
int FaultReport(Report_Data * reportData);


int main(void) 
{

    reportData = (Report_Data *)malloc(sizeof(Report_Data));
    if (NULL == reportData)
    {
        mxLog("No Mem For reportData. Fatal Error!!");
        return -1;
    }
    memset(reportData, 0, sizeof(Report_Data));
    
    
    if (0 != InitialReportMain(reportData))
    {
        mxLog("InitialConfig Failed!");
        free(reportData);
        return -1;
    }

    if (0 != HeartReport(reportData))
    {
        mxLog("Heart Report Fail!");
    }
    else
        {
            mxLog("Heart Report over.");
        }
/*

    if (0 != FaultReport(reportData))
    {
        mxLog("Fault Report Failed!");
    }
    else
        {
            mxLog("Fault Report over.");
        }

    */
    free(reportData);

    mxLog("All Report END.");
    return 0;
}





/******************************
 * Name : HeartReport
 * Return: Success: 0
 *          Failed: not zero
 * Description:
 ***********************************/
int HeartReport(Report_Data * reportData)
{
    int              ccc, cpuNumber;
    unsigned long    mem[5];
    char             json[200];
    float            sysload[3];
    float *          cpu = NULL;


    if (NULL == reportData)
    {
        mxLog("Heart Report error!");
        return -1;
    }
    
    /*setp1: get the data*/
    ConnectNum(&ccc);
    mxLog("Connect Num:%d", ccc);
    
    SystemLoad(sysload);    
    mxLog("Sysload:%4.2f   %4.2f   %4.2f", sysload[0], sysload[1], sysload[2]);

    MemoryUsage(mem);
    mxLog("Mem:%lu   %lu   %lu    %lu    %lu", mem[0], mem[1], mem[2], mem[3], mem[4]);


    if (0 != GetCPUNumber(&cpuNumber))
    {
        mxLog("Get cpu number failed! Default setting:1.");
        cpuNumber = 1;
    }    
    mxLog("CPU number(s):%d", cpuNumber);
    
    cpu = (float *) malloc(cpuNumber * sizeof(float));
    if (NULL == cpu)
    {
        mxLog("No Mem For cpu info. Error!!");
        return -1;
    }
    memset(cpu, 0, cpuNumber * sizeof(float));
    CpuUsage(reportData->HWInfo, cpu, cpuNumber);
    

    /*setp2: Assember Json*/
    if (0 != CreateHeartJson(json, ccc, mem, cpu, cpuNumber, sysload))
    {
        mxLog("Create Heart Json Failed!");
        free(cpu);
        return -1;
    }
	mxLog("Heart JSON:%s", json);

    /*setp3: Report json*/
    int port = atoi(reportData->port);
    if (0 == ReportJson(port, reportData->reportDomain, reportData->reportPath, reportData->encryptMac, json))
    {
        mxLog("Heart Report Success. END");  
    }    
    else
        {
            mxLog("Heart Report Failed! END");
        }

    free(cpu);
    return 0;
}



int FaultReport(Report_Data * reportData)
{
    char    json[200]      = {0};       
    int     lastFault      = -1;
    FILE *  faultFile      = NULL;
    
    if (NULL == reportData)
    {
        mxLog("Fault Report error!");
        return -1;
    }
    
    /* Setp1: Check the equipment failure*/
    int nowFault = NginxServiceCheck(reportData);
    mxLog("NowFault:%d", nowFault);
    
    if (nowFault < 0)
    {
        mxLog("Check error!");
        return 0;
    }

    /*Setp2: Get the last time report type*/
    faultFile = fopen(reportData->faultReportFile, "r");
    if ( 1 != fscanf(faultFile, "%d", &lastFault) )
    {
        fclose(faultFile);   
        return -1;
    }    
    fclose(faultFile); 
    mxLog("Last fault flag:%d", lastFault);    

    /*Setp3: Whether need to report*/
    if (nowFault == lastFault)
    {
        mxLog("This event has been reported.");
        return 0;
    }

    /*Setp4: Assember Json*/
    if (1 == nowFault)      //fault report
    {
        if (0 != CreateFaultJson(9, json))
        {
            mxLog("Create fault json failed!");
        }
        mxLog("Fault JSON:%s", json);
    }
    if (0 == nowFault)      //fault ok report
    {
        if (0 != CreateFaultJson(10, json))
        {
            mxLog("Create fault Ok json failed!");
        }
        mxLog("Fault OK JSON:%s", json);
    }
    
    /*Setp5: Report Json*/
    int port = atoi(reportData->port);
    if (0 == ReportJson(port, reportData->reportDomain, reportData->reportPath, reportData->encryptMac, json))
    {        
        if (0 == nowFault)
        {
            mxLog("Fault OK report Success. END");
            if (0 != OverWriteStringToFile(reportData->faultReportFile, "0"))
            {
                mxLog("Record events failure!");
            }
        }
        if (1 == nowFault)
        {
            mxLog("Fault report Success. END");
            if (0 != OverWriteStringToFile(reportData->faultReportFile, "1"))
            {
                mxLog("Record events failure!");
            }
        }
    }
    else
        {
            mxLog("Fault report Failed! END");
        }
    
    
    return 0;
}



#define READCONFITEM(FILENAME, NAME, VALUE) do{\
    int ret;                                               \
    if(0!=(ret = ReadValueFromConf(FILENAME, NAME, VALUE))) \
    {                                                      \
        mxLog("ReadValueFromConf  ERR code=%d", ret);      \
        mxLog("ReadConf PASS KEY=%s, Value=%s", NAME, VALUE);\
        return -1;                                         \
    }                                                      \
    else{mxLog("ReadConf PASS KEY=%s, Value=%s", NAME, VALUE);}\
}while(0)


int InitialReportMain(Report_Data * reportData)
{

    if (NULL == reportData)
    {
        mxLog("Error!");
        return -1;
    }
    if (0 != (access("./etc/conf", F_OK)))
    {
        mxLog("No finded config file!");
        return -1;
    }

    /* Setp1: Read config */
    READCONFITEM("./etc/conf",    "macFile",          reportData->macFile);
    READCONFITEM("./etc/conf",    "FaultReportFile",  reportData->faultReportFile);
	READCONFITEM("./etc/conf",    "ReportDomain",     reportData->reportDomain);
	READCONFITEM("./etc/conf",    "ReportPath",       reportData->reportPath);
	READCONFITEM("./etc/conf",    "ReportPort",       reportData->port);
	READCONFITEM("./etc/conf",    "targetPath",       reportData->targetPath);

    /* Setp2: Check fault report record file */
    if (0 != (access(reportData->faultReportFile, F_OK))) 
    {
        char command[64];
        memset(command, 0, sizeof(command));
        strcat(command, "echo 0 > ");
        strcat(command, reportData->faultReportFile);        
        mxLog("Create faultReportFile command:%s", command);
        
        if (0 != system(command)) 
        {
            mxLog("Touch File Failed!");
        }
    }
    
    /* Setp3: Get HWInfo */
    if (0 != GetHWInfo("./etc/HWInfo.conf", reportData->HWInfo))
    {
        mxLog("Get HWInfo failed!");
        return -1;        
    }
    mxLog("HWInfo is:%s", reportData->HWInfo);

    /* Setp4: Get report MAC */
    if (0 != GetReportMAC(reportData->macFile, reportData->mac))
    {
        mxLog("Get mac failed!");
        return -1;
    }
    mxLog("MAC is:%s", reportData->mac);

    /*Setp5: Encrypt report MAC */
    if (0 != EncryptMAC(reportData->mac, reportData->encryptMac))
    {
        mxLog("Encrypt MAC failed!");
        return -1;        
    }
    mxLog("Encrypt MAC is:%s", reportData->encryptMac);
    
    return 0;
}

