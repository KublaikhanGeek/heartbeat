#ifndef _MXJSON_H_
#define _MXJSON_H_
#ifdef __cplusplus
extern "C"
{
#endif

/*
* cjsonfile
* IN: char *xmlpath, path of json file
* OUT: const char *savepath, save folder, MUSH have ending '/'
*/
extern int cjsonfile(const char *xmlpath);

#ifdef __cplusplus
}
#endif
#endif 
