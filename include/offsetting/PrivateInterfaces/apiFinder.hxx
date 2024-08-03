#pragma once
#define _AMD64_
#include <synchapi.h>
#include <tchar.h>
#include <windows.h>

// 模块加载情况
enum MODULESTATE { MODULE_UNKNOWN, MODULE_NOT_FIND, MODULE_FIND };
// API 查询情况
enum PROCSTATE { PROC_UNKNOWN, PROC_NOT_FIND, PROC_FIND };

class APIFINDER {
    MODULESTATE m_STATE;
    TCHAR* m_moduleName;
    HMODULE m_hModule;
    void Clear();

  public:
    // 构造时即查询模块加载情况
    APIFINDER(const TCHAR* moduleName = TEXT(""));
    ~APIFINDER();
    MODULESTATE ResetModule(const TCHAR* moduleName);
    FARPROC GetAddress(const CHAR* procName, PROCSTATE& state);
};
