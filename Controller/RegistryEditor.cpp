#include "../Api/RegistryEditor.h"

const char FAIL_MESSAGE[] = "Error!\0";
const char SUCCESS_MESSAGE[] = "Ok!\0";

/// <summary>
///		Convert const char* to const wchar_t*
/// </summary>
/// 
/// <param name="c">const char*</param>
/// 
/// <returns>const wchar_t*</returns>
const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	size_t outSize;
	mbstowcs_s(&outSize, wc, cSize, c, cSize - 1);

	return wc;
}

/// <summary>
///		Get hkey root path
/// </summary>
/// 
/// <param name="lpsKey">Search key</param>
/// 
/// <returns>HKEY</returns>
HKEY GetHkeyRoot(LPSTR lpsKey)
{
	if (strcmp(lpsKey, "HKEY_CLASSES_ROOT") == 0)
	{
		return HKEY_CLASSES_ROOT;
	}
	if (strcmp(lpsKey, "HKEY_CURRENT_USER") == 0)
	{
		return HKEY_CURRENT_USER;
	}
	if (strcmp(lpsKey, "HKEY_LOCAL_MACHINE") == 0)
	{
		return HKEY_LOCAL_MACHINE;
	}
	if (strcmp(lpsKey, "HKEY_USERS") == 0)
	{
		return HKEY_USERS;
	}
	if (strcmp(lpsKey, "HKEY_CURRENT_CONFIG") == 0)
	{
		return HKEY_CURRENT_CONFIG;
	}

	return NULL;
}

/// <summary>
///		Get value type
/// </summary>
/// 
/// <param name="lpsType">Type in string format</param>
/// 
/// <returns>DWORD</returns>
DWORD GetParamType(LPSTR lpsType)
{
	if (strcmp(lpsType, "REG_SZ") == 0)
	{
		return REG_SZ;
	}
	if (strcmp(lpsType, "REG_BINARY") == 0)
	{
		return REG_BINARY;
	}
	if (strcmp(lpsType, "REG_DWORD") == 0)
	{
		return REG_DWORD;
	}
	if (strcmp(lpsType, "REG_LINK") == 0)
	{
		return REG_LINK;
	}
	return NULL;
}

/// <summary>
///		Convert value of registry type to LPVOID
/// </summary>
/// <param name="dwParamType"></param>
/// <param name="lpsValue"></param>
/// <param name="dwValueSize"></param>
/// <returns></returns>
LPCVOID ConvertValueToLPVOID(DWORD dwParamType, LPSTR lpsValue, DWORD* dwValueSize)
{
	switch (dwParamType)
	{
		case REG_SZ:
		{
			LPVOID lpBuffer = (LPVOID)GetWC(lpsValue);
			*dwValueSize = lstrlen(GetWC(lpsValue)) * sizeof(WCHAR);
			return lpBuffer;
		}
		case REG_DWORD:
		{
			DWORD* lpdwValue = (DWORD*)calloc(1, sizeof(DWORD));
			*lpdwValue = atoi(lpsValue);
			*dwValueSize = sizeof(*lpdwValue);
			return (LPVOID)lpdwValue;
		}
		case REG_BINARY:
		{
			LPVOID lpBuffer = (LPVOID)lpsValue;
			*dwValueSize = strlen(lpsValue) * sizeof(char);
			return lpBuffer;
		}
		case REG_LINK:
		{
			LPVOID lpBuffer = (LPVOID)GetWC(lpsValue);
			*dwValueSize = lstrlen(GetWC(lpsValue)) * sizeof(WCHAR);
			return lpBuffer;
		}
	}

	return NULL;
}

/// <summary>
///		Add key
/// </summary>
/// 
/// <param name="arguments">Arguments values</param>
/// <param name="argumentsCount">Arguments count</param>
/// 
/// <returns>LPCSTR</returns>
LPCSTR AddKeyCommand(LPSTR* arguments, DWORD argumentsCount)
{
	if (argumentsCount < 2)
	{
		return FAIL_MESSAGE;
	}

	// Convert to necessary format
	HKEY hKeyRoot = GetHkeyRoot(arguments[0]);
	LPCWSTR lpsSubkeyPath = GetWC(arguments[1]);
	if ((hKeyRoot == NULL) || (lpsSubkeyPath == NULL))
	{
		return FAIL_MESSAGE;
	}

	// Create a new key in registry
	if (CreateRegKey(hKeyRoot, lpsSubkeyPath))
	{
		return SUCCESS_MESSAGE;
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

/// <summary>
///		Add value to key
/// </summary>
/// 
/// <param name="lpsArguments">Arguments values</param>
/// <param name="dwArgumentsCount">Arguments count</param>
/// 
/// <returns>LPCSTR</returns>
LPCSTR AddValueCommand(LPSTR* lpsArguments, DWORD dwArgumentsCount)
{
	if (dwArgumentsCount < 5)
	{
		return FAIL_MESSAGE;
	}

	// Convert to necessary format
	DWORD dwArgumentSize;
	LPCVOID lpBuffer = ConvertValueToLPVOID(GetParamType(lpsArguments[3]), lpsArguments[4], &dwArgumentSize);
	if (lpBuffer == NULL)
	{
		return FAIL_MESSAGE;
	}

	// Set new value
	if (SetRegKey(
		GetHkeyRoot(lpsArguments[0]), 
		GetWC(lpsArguments[1]), 
		GetWC(lpsArguments[2]), 
		GetParamType(lpsArguments[3]), 
		lpBuffer,
		dwArgumentSize))
	{
		return SUCCESS_MESSAGE;
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

/// <summary>
///		Search key
/// </summary>
/// 
/// <param name="arguments">Arguments values</param>
/// <param name="argumentsCount">Arguments count</param>
/// 
/// <returns>LPCSTR</returns>
LPCSTR SearchKeyCommand(LPSTR* arguments, DWORD argumentsCount)
{
	if (argumentsCount < 3)
	{
		return FAIL_MESSAGE;
	}

	// Open an existing key in registry
	HKEY hKey;
	if (!OpenRegKey(GetHkeyRoot(arguments[0]), GetWC(arguments[1]), KEY_READ, &hKey))
	{
		return FAIL_MESSAGE;
	}

	// Search necessary key
	DWORD dwFoundKeysCount = 0;
	LPWSTR* lpsFoundKeys = SearchKey(hKey, GetWC(arguments[2]), &dwFoundKeysCount);
	if (lpsFoundKeys == NULL)
	{
		return "No keys found!\n";
	}

	// Output result
	printf("Search result in %s\\%s\\:\n", arguments[0], arguments[1]);
	for (DWORD dwIndex = 0; dwIndex < dwFoundKeysCount; dwIndex++)
	{
		wprintf(L"%d. %s\n", dwIndex, lpsFoundKeys[dwIndex]);
	}

	return SUCCESS_MESSAGE;
}

/// <summary>
///		Get flags
/// </summary>
/// 
/// <param name="arguments">Arguments values</param>
/// <param name="argumentsCount">Arguments count</param>
/// 
/// <returns>LPCSTR</returns>
LPCSTR ViewFlagsCommand(LPSTR* arguments, DWORD argumentsCount)
{
	if (argumentsCount < 2)
	{
		return FAIL_MESSAGE;
	}

	// Create query like L"REG FLAGS HKLM\\SOFTWARE\\Test_key QUERY"
	WCHAR* lpsCommand = CreateFlagsQuery(GetWC(arguments[0]), GetWC(arguments[1]));
	if (lpsCommand == NULL)
	{
		return FAIL_MESSAGE;
	}
	
	// Get flags in string format
	LPSTR lpsRegExeOutput = ExecuteRegExe(lpsCommand);
	if (lpsRegExeOutput == NULL)
	{
		return FAIL_MESSAGE;
	}

	KEYFLAG* kfFlags = NULL;
	DWORD dwFlagsCount;

	// Initialize flags values
	kfFlags = GetInitializedFlags(&dwFlagsCount);
	if (kfFlags == NULL)
	{
		printf("Initialization key flags failure!\n");
	}
	else
	{
		// GEt final result
		if (!ParseRegExeOutput(lpsRegExeOutput, kfFlags, dwFlagsCount))
		{
			return FAIL_MESSAGE;
		}
	}

	// Output result
	printf("Key flags:\n");
	for (DWORD dwIndex = 0; dwIndex < dwFlagsCount; dwIndex++)
	{
		printf("%d. Flag name: %s  Flag value: %s\n", dwIndex, kfFlags[dwIndex].lpsFlagName, kfFlags[dwIndex].lpsFlagValue);
	}

	return SUCCESS_MESSAGE;
}

/// <summary>
///		Observe registry changes
/// </summary>
/// 
/// <param name="arguments">Argumets values</param>
/// <param name="argumentsCount">Arguments count</param>
/// 
/// <returns>LPCSTR</returns>
LPCSTR NotifyCommand(LPSTR* arguments, DWORD argumentsCount)
{
	if (argumentsCount < 2)
	{
		return FAIL_MESSAGE;
	}

	// Get hkey name
	HKEY hKeyRoot = GetHkeyRoot(arguments[0]);
	if (hKeyRoot == NULL)
	{
		return FAIL_MESSAGE;
	}

	// Start notify
	LPCWSTR lpsSubkeyPath = GetWC(arguments[1]);
	if (NotifyChange(hKeyRoot, lpsSubkeyPath, TRUE))
	{
		return SUCCESS_MESSAGE;
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

/// <summary>
///		Find necessary command
/// </summary>
/// 
/// <param name="argc">Arguments count</param>
/// <param name="argv">Argumets values</param>
/// 
/// <returns>LPCSTR</returns>
LPCSTR CommandProcessor(char** argv, int argc)
{
	if (argc < 2)
	{
		return FAIL_MESSAGE;
	}

	if (strcmp(argv[1], "ADD_KEY") == 0)
	{
		return AddKeyCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "ADD_VALUE") == 0)
	{
		return AddValueCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "VIEW_FLAGS") == 0)
	{
		return ViewFlagsCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "SEARCH_KEY") == 0)
	{
		return SearchKeyCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "NOTIFY") == 0)
	{
		return NotifyCommand(argv + 2, argc - 2);
	}

	return NULL;
}

/// <summary>
///		Entry point
/// </summary>
/// 
/// <param name="argc">Arguments count</param>
/// <param name="argv">Argumets values</param>
/// 
/// <returns>int</returns>
int main(int argc, char** argv)
{
	LPCSTR cmdResult = CommandProcessor(argv, argc);

	if (cmdResult == NULL)
	{
		printf("%s\n", FAIL_MESSAGE);
	}
	else
	{
		printf("%s\n", cmdResult);
	}

	getchar();

	return 0;
}

///	ADD_KEY HKEY_LOCAL_MACHINE SOFTWARE\TEST
/// ADD_VALUE HKEY_LOCAL_MACHINE SOFTWARE\TEST TEST REG_SZ TEST
/// VIEW_FLAGS HKEY_LOCAL_MACHINE SOFTWARE\TEST
/// SEARCH_KEY HKEY_LOCAL_MACHINE SOFTWARE TEST
/// NOTIFY HKEY_LOCAL_MACHINE SOFTWARE