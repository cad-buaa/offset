#include "PrivateInterfaces/apiFinder.hxx"

// moduleName 的字符编码可能是 ANSI(CHAR) or UNICODE(WCHAR)
// procName 的字符编码仅为 ANSI(CHAR)
APIFINDER::APIFINDER(const TCHAR* moduleName): m_moduleName(nullptr), m_hModule(nullptr), m_STATE(MODULE_UNKNOWN) {
    if(_tcslen(moduleName) > 0) {
        m_moduleName = new TCHAR[_tcslen(moduleName) + 1];
        _tcscpy(m_moduleName, moduleName);
        m_hModule = GetModuleHandle(m_moduleName);
        if(!m_hModule) {
            m_STATE = MODULE_NOT_FIND;
        } else {
            m_STATE = MODULE_FIND;
        }
    }
}

void APIFINDER::Clear() {
    if(m_moduleName) {
        delete[] m_moduleName;
        m_moduleName = nullptr;
    }
    m_hModule = nullptr;
    m_STATE = MODULE_UNKNOWN;
}

APIFINDER::~APIFINDER() {
    Clear();
}

MODULESTATE APIFINDER::ResetModule(const TCHAR* moduleName) {
    Clear();
    if(_tcslen(moduleName) > 0) {
        m_moduleName = new TCHAR[_tcslen(moduleName) + 1];
        _tcscpy(m_moduleName, moduleName);
        m_hModule = GetModuleHandle(m_moduleName);
        if(!m_hModule) {
            m_STATE = MODULE_NOT_FIND;
        } else {
            m_STATE = MODULE_FIND;
        }
    }
    return m_STATE;
}

FARPROC APIFINDER::GetAddress(const CHAR* procName, PROCSTATE& state) {
    FARPROC ret = nullptr;
    if(m_STATE != MODULE_FIND) {
        state = PROC_NOT_FIND;
        return ret;
    }
    state = PROC_UNKNOWN;

    ret = GetProcAddress(m_hModule, procName);
    if(!ret) {
        state = PROC_NOT_FIND;
        return ret;
    }
    state = PROC_FIND;
    return ret;
}

// 定义一个全局的接口查找器
APIFINDER apiFinderACIS(TEXT("SPAAcisDs.dll"));
// 定义一个全局的接口查找状态值
PROCSTATE prostate = PROC_UNKNOWN;
