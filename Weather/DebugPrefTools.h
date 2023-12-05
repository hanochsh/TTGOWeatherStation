#pragma once

#ifdef  __cplusplus
#define C_Prefix "C"
#else
#define C_Prefix
#endif

extern C_Prefix int startPrefAction(char* tokenName);
extern C_Prefix int endPrefAction(int idx);
extern C_Prefix int getAvgPrefTime(int idx);

#ifdef DEBUGPREF
#define DEBUGLINE
#else
#define DEBUGLINE //
#endif