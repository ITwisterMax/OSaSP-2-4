#include <windows.h>
#include <iostream>

#include "../Api/RegistryEditor.h"

/// <summary>
///		Create a new key in registry
/// </summary>
/// 
/// <param name="hKeyRoot">Hkey root path</param>
/// <param name="lpSubKey">Key path in hkey</param>
/// 
/// <returns>bool</returns>
bool CreateRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey)
{
	if (lpSubKey == NULL)
	{
		return false;
	}

	HKEY hKey;
	DWORD dwDisposition;

	// Create key
	LRESULT error = RegCreateKeyEx(hKeyRoot, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, &dwDisposition);

	CloseRegKey(hKey);

	return (error == ERROR_SUCCESS) && (dwDisposition == REG_CREATED_NEW_KEY);
}

/// <summary>
///		Open key
/// </summary>
/// 
/// <param name="hKeyRoot">Hkey root path</param>
/// <param name="lpSubKey">Path to key in hkey</param>
/// <param name="samDesired"></param>
/// <param name="phkResult"></param>
/// 
/// <returns>bool</returns>
bool OpenRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey, REGSAM samDesired, PHKEY phkResult)
{
	if (lpSubKey == NULL)
	{
		return false;
	}

	// Open key
	LRESULT error = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, samDesired, phkResult);
	return error == ERROR_SUCCESS;
}

/// <summary>
///		Close key
/// </summary>
/// 
/// <param name="hKey">Hkey root path</param>
/// 
/// <returns>bool</returns>
bool CloseRegKey(HKEY hKey)
{
	LRESULT error = RegCloseKey(hKey);

	return error == ERROR_SUCCESS;
}

/// <summary>
///		Set one of key parameters in registry
/// </summary>
/// 
/// <param name="hKeyRoot">Hkey root path</param>
/// <param name="lpSubKey">Key path in hkey</param>
/// <param name="lpParamName">Value name</param>
/// <param name="dwParamType">Value type</param>
/// <param name="lpData">Value</param>
/// <param name="cbData">Format Value</param>
/// 
/// <returns>bool</returns>
bool SetRegKey(HKEY hKeyRoot, LPCWSTR lpSubKey, LPCWSTR lpParamName, DWORD dwParamType, LPCVOID lpData, DWORD cbData)
{
	if ((lpParamName == NULL)|| (lpSubKey == NULL) || (lpData == NULL))
	{
		return false;
	}

	// Open key root in KEY_WRITE mode
	HKEY hKey;
	if (!OpenRegKey(hKeyRoot, L"", KEY_WRITE, &hKey))
	{
		return false;
	}

	// Set value of key parameter
	LRESULT error = RegSetKeyValue(hKey, lpSubKey, lpParamName, dwParamType, lpData, cbData);
	CloseRegKey(hKey);

	return error == ERROR_SUCCESS;
}

/// <summary>
///		Add elements to LPWSTR array
/// </summary>
/// 
/// <param name="lpsSourceArray">Input array</param>
/// <param name="dwSourceArrayCount">Array count</param>
/// <param name="lpsAdditionArray">New array part</param>
/// <param name="dwAdditionArrayCount">New array count</param>
/// 
/// <returns>LPWSTR*</returns>
LPWSTR* AddElementsToLPWSTRArray(LPWSTR* lpsSourceArray, DWORD dwSourceArrayCount, LPWSTR* lpsAdditionArray, DWORD dwAdditionArrayCount)
{
	if ((lpsAdditionArray == NULL) || (dwAdditionArrayCount == 0))
	{
		return lpsSourceArray;
	}

	if (lpsSourceArray == NULL)
	{
		return NULL;
	}

	LPWSTR* lpsResultArray = (LPWSTR*)realloc(lpsSourceArray, (dwSourceArrayCount + dwAdditionArrayCount) * sizeof(LPWSTR));
	if (lpsResultArray == NULL)
	{
		return NULL;
	}

	for (DWORD dwAdditionElementIndex = 0; dwAdditionElementIndex < dwAdditionArrayCount; ++dwAdditionElementIndex)
	{
		lpsResultArray[dwSourceArrayCount + dwAdditionElementIndex] = lpsAdditionArray[dwAdditionElementIndex];
	}

	return lpsResultArray;
}

/// <summary>
///		Create full path from lpsKeyPath and lpsSubKeyName
/// </summary>
/// 
/// <param name="lpsKeyPath">Hkey root path</param>
/// <param name="lpsSubKeyName">Key path in hkey</param>
/// 
/// <returns>LPWSTR</returns>
LPWSTR CreateFullName(LPWSTR lpsKeyPath, LPWSTR lpsSubKeyName)
{
	DWORD oldPathLength = lstrlen(lpsKeyPath);
	DWORD additionLength = lstrlen(lpsSubKeyName);
	DWORD newPathLength = oldPathLength + additionLength + 1;
	LPWSTR newKeyPath;

	// Merge two strings
	if (oldPathLength == 0)
	{
		newKeyPath = (LPWSTR)calloc(newPathLength + 1, sizeof(WCHAR));

		if (newKeyPath != NULL)
		{
			wcscpy_s(newKeyPath, newPathLength + 1, lpsSubKeyName);
		}
	}
	else
	{
		newKeyPath = (LPWSTR)calloc(newPathLength + 1, sizeof(WCHAR));

		if (newKeyPath != NULL)
		{
			wcscpy_s(newKeyPath, newPathLength + 1, lpsKeyPath);
			wcscat_s(newKeyPath, newPathLength + 1, L"\\");
			wcscat_s(newKeyPath, newPathLength + 1, lpsSubKeyName);
		}
	}
	
	return newKeyPath;
}

/// <summary>
///		Get list of names of keys on current level on nesting
/// </summary>
/// 
/// <param name="hKeyRoot">Hkey root path</param>
/// <param name="lpsKeyPath">Key path in hkey</param>
/// <param name="lpdwResultSize">Result count</param>
/// 
/// <returns>LPWSTR*</returns>
LPWSTR* SearchOneLevel(HKEY hKeyRoot, LPCWSTR lpsKeyPath, DWORD* lpdwResultSize)
{
	if ((lpsKeyPath == NULL) || (lpdwResultSize == 0))
	{
		return NULL;
	}

	// Enumerate keys in current folder
	HKEY hKey;
	if (!OpenRegKey(hKeyRoot, lpsKeyPath, KEY_ENUMERATE_SUB_KEYS, &hKey))
	{
		return NULL;
	}

	LPWSTR* lpsResultSubkeyNames = (LPWSTR*)calloc(0, sizeof(LPWSTR));
	LPWSTR lpsSubKeyName = (LPWSTR)calloc(MAX_KEY_NAME_LENGTH, sizeof(WCHAR));
	LRESULT error = ERROR_SUCCESS;
	DWORD dwNameSize;
	LPWSTR lpsFullName;
	DWORD dwNamesCount = 0;

	// Add all first level keys to list
	for (int dwIndex = 0; error != ERROR_NO_MORE_ITEMS; dwIndex++)
	{
		dwNameSize = MAX_KEY_NAME_LENGTH;
		error = RegEnumKeyEx(hKey, dwIndex, lpsSubKeyName, &dwNameSize, NULL, NULL, NULL, NULL);

		if (error == ERROR_SUCCESS)
		{
			lpsFullName = CreateFullName(const_cast<LPWSTR>(lpsKeyPath), lpsSubKeyName);

			if (lpsFullName != NULL)
			{
				LPWSTR* resultArray = AddElementsToLPWSTRArray(lpsResultSubkeyNames, dwNamesCount, &lpsFullName, 1);

				if (resultArray != NULL)
				{
					lpsResultSubkeyNames = resultArray;
					dwNamesCount++;
				}
				else
				{
					free(lpsFullName);
				}
			}
		}
	}

	free(lpsSubKeyName);
	CloseRegKey(hKey);
	*lpdwResultSize = dwNamesCount;

	return lpsResultSubkeyNames;
}

/// <summary>
///		Get list of names of keys recursive on all levels of nesting
/// </summary>
/// 
/// <param name="hKeyRoot">Hkey root path</param>
/// <param name="lpsKeyPath">Key path in hkey</param>
/// <param name="lpdwResultCount">Result count</param>
/// 
/// <returns>LPWSTR*</returns>
LPWSTR* SearchRecursive(HKEY hKeyRoot, LPCWSTR lpsKeyPath, DWORD* lpdwResultCount)
{
	LPWSTR* lpsSubresult;
	LPWSTR* lpsGeneralSubresult = (LPWSTR*)calloc(0, sizeof(LPWSTR));

	DWORD dwKeyNamesCount, dwSubresultCount, dwGeneralSubresultCount = 0;

	// Format elements in first level
	LPWSTR* lpsKeyNamesList = SearchOneLevel(hKeyRoot, lpsKeyPath, &dwKeyNamesCount);
	LPWSTR* lpsBuffer;

	if (lpsKeyNamesList == NULL)
	{
		return NULL;
	}

	// Recursive call
	for (DWORD dwElemIndex = 0; dwElemIndex < dwKeyNamesCount; dwElemIndex++)
	{
		lpsSubresult = SearchRecursive(hKeyRoot, lpsKeyNamesList[dwElemIndex], &dwSubresultCount);

		if (lpsSubresult != NULL)
		{
			lpsBuffer = AddElementsToLPWSTRArray(lpsGeneralSubresult, 
				dwGeneralSubresultCount, 
				lpsSubresult, 
				dwSubresultCount);

			if (lpsBuffer != NULL)
			{
				lpsGeneralSubresult = lpsBuffer;
				dwGeneralSubresultCount += dwSubresultCount;
			}

			free(lpsSubresult);
		}
	}

	lpsBuffer = AddElementsToLPWSTRArray(lpsKeyNamesList, dwKeyNamesCount, lpsGeneralSubresult, dwGeneralSubresultCount);

	if (lpsBuffer != NULL)
	{
		*lpdwResultCount = dwKeyNamesCount + dwGeneralSubresultCount;
		return lpsBuffer;
	}
	else
	{
		*lpdwResultCount = dwKeyNamesCount;
		return lpsBuffer;
	}
}

/// <summary>
///		Find necessary key in list
/// </summary>
/// 
/// <param name="lpsKeyNamesList">Input list</param>
/// <param name="dwKeyNamesCount">Names count</param>
/// <param name="lpsSearchedKey">Searched key</param>
/// <param name="lpdwFoundKeysCount">Keys count</param>
/// 
/// <returns>LPWSTR*</returns>
LPWSTR* SearchKeyInList(LPWSTR* lpsKeyNamesList, DWORD dwKeyNamesCount, LPWSTR lpsSearchedKey, DWORD* lpdwFoundKeysCount)
{
	if ((lpsKeyNamesList == NULL) || (lpsSearchedKey == NULL))
	{
		return NULL;
	}

	DWORD dwResultCount = 0;
	LPWSTR* lpsResult = (LPWSTR*)calloc(dwResultCount, sizeof(LPWSTR)), *lpsBuffer;
	WCHAR* lpsTemp;

	// Find necessary element
	if (lpsResult != NULL)
	{
		for (DWORD dwKeyIndex = 0; dwKeyIndex < dwKeyNamesCount; dwKeyIndex++)
		{
			DWORD dwKeyNameLength = lstrlen(lpsSearchedKey);

			if (((lpsTemp = wcsstr(lpsKeyNamesList[dwKeyIndex], lpsSearchedKey)) != NULL) && ((lpsTemp[dwKeyNameLength] == L'\0') || (lpsTemp[dwKeyNameLength] == L'\\')))
			{
				lpsBuffer = AddElementsToLPWSTRArray(lpsResult, dwResultCount, &(lpsKeyNamesList[dwKeyIndex]), 1);

				if (lpsBuffer != NULL)
				{
					lpsResult = lpsBuffer;
					dwResultCount++;
				}
			}
		}
	}

	*lpdwFoundKeysCount = dwResultCount;
	return lpsResult;
}

/// <summary>
///		Search key function
/// </summary>
/// 
/// <param name="hKey">Hkey root path</param>
/// <param name="lpsSearchedKey">Searched key path</param>
/// <param name="lpdwFoundKeysCount">Found keys count</param>
/// 
/// <returns>LPWSTR*</returns>
LPWSTR* SearchKey(HKEY hKey, LPCWSTR lpsSearchedKey, DWORD* lpdwFoundKeysCount)
{
	if ((lpsSearchedKey == NULL) || (lstrlen(lpsSearchedKey) == 0) || (lpdwFoundKeysCount == NULL))
	{
		return NULL;
	}
	
	// Reformat keys to list
	DWORD dwSearchResultCount = 0;
	LPWSTR* lpsAllKeyNamesList = SearchRecursive(hKey, L"", &dwSearchResultCount);
	
	if (lpsAllKeyNamesList == NULL)
	{
		return NULL;
	}
	
	// Find necessary key in list
	LPWSTR* lpsFoundKeyNamesList = SearchKeyInList(lpsAllKeyNamesList, 
		dwSearchResultCount, 
		const_cast<LPWSTR>(lpsSearchedKey), 
		&dwSearchResultCount);

	*lpdwFoundKeysCount = dwSearchResultCount;

	return lpsFoundKeyNamesList;
}

/// <summary>
///		Execute query
/// </summary>
/// 
/// <param name="lpsCommand">Get flags query</param>
/// 
/// <returns>LPSTR</returns>
LPSTR ExecuteRegExe(WCHAR* lpsCommand)
{
	HANDLE hReadPipe, hWritePipe;

	// Set the bInheritHandle flag so pipe handles are inherited.
	SECURITY_ATTRIBUTES saAttributes;
	saAttributes.bInheritHandle = TRUE;
	saAttributes.lpSecurityDescriptor = NULL;
	saAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

	LPSTR lpsResult = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (CreatePipe(&hReadPipe, &hWritePipe, &saAttributes, 0))
	{
		// Create a child process that uses the previously created pipes for STDIN and STDOUT.
		STARTUPINFO siConsole;
		PROCESS_INFORMATION piInfo;

		// Set up members of the PROCESS_INFORMATION structure. 
		ZeroMemory(&piInfo, sizeof(PROCESS_INFORMATION));

		// Set up members of the STARTUPINFO structure. 
		// This structure specifies the STDIN and STDOUT handles for redirection.
		ZeroMemory(&siConsole, sizeof(STARTUPINFO));
		siConsole.cb = sizeof(STARTUPINFO);
		siConsole.hStdOutput = hWritePipe;
		siConsole.hStdError = hWritePipe;
		siConsole.hStdInput = hReadPipe;
		siConsole.dwFlags |= STARTF_USESTDHANDLES;
		
		// Create the child process. 
		if (CreateProcess(NULL, lpsCommand, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siConsole, &piInfo))
		{
			const DWORD dwBufLength = 4096;
			LPSTR lpsBuffer = (LPSTR)calloc(dwBufLength, sizeof(CHAR));
			DWORD dwReadCount;

			WaitForSingleObject(piInfo.hProcess, INFINITE);

			if (ReadFile(hReadPipe, lpsBuffer, (dwBufLength - 1) * sizeof(CHAR), &dwReadCount, NULL))
			{
				lpsBuffer[dwReadCount / sizeof(CHAR)] = '\0';
				lpsResult = lpsBuffer;
			}
			else
			{
				free(lpsBuffer);
			}

			// Close handles to the child process and its primary thread.
			// Some applications might keep these handles to monitor the status
			// of the child process, for example. 
			CloseHandle(piInfo.hThread);
			CloseHandle(piInfo.hProcess);
		}

		// Close handles to the stdin and stdout pipes no longer needed by the child process.
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
	}

	return lpsResult;
}

/// <summary>
///		Create flags query
/// </summary>
/// 
/// <param name="lpsKeyRoot">Hkey root path</param>
/// <param name="lpsSubkeyPath">Key path in hkey</param>
/// 
/// <returns></returns>
LPWSTR CreateFlagsQuery(LPCWSTR lpsKeyRoot, LPCWSTR lpsSubkeyPath)
{
	DWORD dwKeyRootLength = lstrlen(lpsKeyRoot);
	DWORD dwSubkeyPathLength = lstrlen(lpsSubkeyPath);
	DWORD dwNewPathLength = dwKeyRootLength + dwSubkeyPathLength + 1;

	LPWSTR lpsKey = (LPWSTR)calloc(dwNewPathLength + 1, sizeof(WCHAR));

	if (lpsKey != NULL)
	{
		wcscpy_s(lpsKey, dwNewPathLength + 1, lpsKeyRoot);
		wcscat_s(lpsKey, dwNewPathLength + 1, L"\\");
		wcscat_s(lpsKey, dwNewPathLength + 1, lpsSubkeyPath);
	}

	DWORD dwQueryLength = lstrlen(lpsKey) + lstrlen(L"REG FLAGS  QUERY");
	LPWSTR lpsResult = (LPWSTR)calloc(dwQueryLength + 1, sizeof(WCHAR));
	swprintf(lpsResult, dwQueryLength + 1, L"REG FLAGS %s QUERY", lpsKey);
	
	return lpsResult;
}

/// <summary>
///		Initialize flags structure
/// </summary>
/// 
/// <param name="dwFlagsCount">Flags count</param>
/// 
/// <returns>KEYFLAG*</returns>
KEYFLAG* GetInitializedFlags(DWORD* dwFlagsCount)
{
	KEYFLAG* kfFlags = (KEYFLAG*)calloc(KEY_FLAGS_COUNT, sizeof(KEYFLAG));

	if (kfFlags == NULL)
	{
		return NULL;
	}

	kfFlags[0].lpsFlagName = const_cast<LPSTR>("REG_KEY_DONT_VIRTUALIZE");
	kfFlags[1].lpsFlagName = const_cast<LPSTR>("REG_KEY_DONT_SILENT_FAIL");
	kfFlags[2].lpsFlagName = const_cast<LPSTR>("REG_KEY_RECURSE_FLAG");

	*dwFlagsCount = KEY_FLAGS_COUNT;
	return kfFlags;
}

/// <summary>
///		Parse string with flags
/// </summary>
/// 
/// <param name="lpsCommandOutput">Command output string value</param>
/// <param name="kfFlags">Flags</param>
/// <param name="dwKeyCount">Key count</param>
/// 
/// <returns>bool</returns>
bool ParseRegExeOutput(LPSTR lpsCommandOutput, KEYFLAG* kfFlags, DWORD dwKeyCount)
{
	if ((lpsCommandOutput == NULL) || (kfFlags == NULL))
	{
		return false;
	}

	LPSTR lpsKeyPos, lpsKeyValuePos, lpsBuffer;
	LPCSTR lpsDelimiter = ": \r\t\n";

	DWORD dwCommandOutputLength = strlen(lpsCommandOutput);
	LPSTR lpsCommandOutputCopy = (LPSTR)calloc(dwCommandOutputLength + 1, sizeof(char));

	DWORD dwValueLength;

	for (DWORD dwkeyIndex = 0; dwkeyIndex < dwKeyCount; ++dwkeyIndex)
	{
		strcpy_s(lpsCommandOutputCopy, (dwCommandOutputLength + 1) * sizeof(CHAR), lpsCommandOutput);
		lpsKeyPos = strstr(lpsCommandOutputCopy, kfFlags[dwkeyIndex].lpsFlagName);

		if (lpsKeyPos == NULL)
		{
			return false;
		}
		else
		{
			// Find necessary substring and write it in kfFlags
			lpsKeyValuePos = strtok_s(lpsKeyPos + strlen(kfFlags[dwkeyIndex].lpsFlagName), lpsDelimiter, &lpsBuffer);

			if (lpsKeyValuePos == NULL)
			{
				return false;
			}
			else
			{
				dwValueLength = strlen(lpsKeyValuePos);
				lpsBuffer = (LPSTR)calloc(dwValueLength + 1, sizeof(char));

				if (lpsBuffer == NULL)
				{
					return false;
				}
				else
				{
					strcpy_s(lpsBuffer, dwValueLength + 1, lpsKeyValuePos);
					kfFlags[dwkeyIndex].lpsFlagValue = lpsBuffer;
				}
			}
		}
	}

	free(lpsCommandOutputCopy);
	return true;
}

/// <summary>
///		Observe registry changes
/// </summary>
/// 
/// <param name="hKeyRoot">Hkey root path</param>
/// <param name="lpsSubkeyPath">Path to key in hkey</param>
/// <param name="bWatchSubtree"></param>
/// 
/// <returns></returns>
bool NotifyChange(HKEY hKeyRoot, LPCWSTR lpsSubkeyPath, bool bWatchSubtree)
{
	HKEY hKey;

	// Open key
	if (!OpenRegKey(hKeyRoot, lpsSubkeyPath, KEY_NOTIFY, &hKey))
	{
		return false;
	}

	// Observe registry changes
	bool bResult = RegNotifyChangeKeyValue(hKeyRoot, bWatchSubtree, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
		NULL, FALSE) == ERROR_SUCCESS;

	// Close key
	CloseRegKey(hKey);

	return bResult;
}