/*
 * mxCPPAPI.h
 *
 *  Created on: Aug 12, 2015
 *      Author: zwh
 */

#ifndef MXCPPAPI_H_
#define MXCPPAPI_H_


extern void mxClearHashMap(void);
extern void AddCStrToHashMap(const char * cstr, int id);
extern int  LookUpCStr(const char * cstr);
extern int ReadValueFromConf(const char* fileName, const char *pConfName, char *pConfValue);
extern int FindSubStrInString(const char* str, const char *subStr);


#endif /* MXCPPAPI_H_ */
