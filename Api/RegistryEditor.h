#pragma once

#include <windows.h>
#include <iostream>

const DWORD MAX_KEY_NAME_LENGTH = 4096;
const DWORD KEY_FLAGS_COUNT = 3;

typedef struct _KEYFLAG {
	LPSTR lpsFlagName;
	LPSTR lpsFlagValue;
} KEYFLAG;

bool OpenRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey, REGSAM samDesired, PHKEY phkResult);
bool CreateRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey);
bool CloseRegKey(HKEY hKey);
bool SetRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey, LPCWSTR lpParamName, DWORD dwParamType, LPCVOID lpData, DWORD cbData);
bool GetRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey, LPCWSTR lpParamName, DWORD* dwParamType, BYTE* lpData, DWORD* cbData);
LPWSTR* AddElementsToLPWSTRArray(LPWSTR* lpsSourceArray, DWORD dwSourceArrayCount, LPWSTR* lpsAdditionArray, DWORD dwAdditionArrayCount);
LPWSTR CreateFullName(LPWSTR lpsKeyPath, LPWSTR lpsSubKeyName);
LPWSTR* SearchOneLevel(HKEY hKeyRoot, LPCWSTR lpsKeyPath, DWORD* lpdwResultSize);
LPWSTR* SearchKeyInList(LPWSTR* lpsKeyNamesList, DWORD dwKeyNamesCount, LPWSTR lpsSearchedKey, DWORD* lpdwFoundKeysCount);
LPWSTR* SearchKey(HKEY hKey, LPCWSTR lpsSearchedKey, DWORD* lpdwFoundKeysCount);
LPSTR ExecuteRegExe(WCHAR* lpsCommand);
KEYFLAG* GetInitializedFlags(DWORD* dwFlagsCount);
bool ParseRegExeOutput(LPSTR lpsCommandOutput, KEYFLAG* kfFlags, DWORD dwKeyCount);
LPWSTR CreateFlagsQuery(LPCWSTR lpsKeyRoot, LPCWSTR lpsSubkeyPath);
bool NotifyChange(HKEY hKeyRoot, LPCWSTR lpsSubkeyPath, bool bWatchSubtree);