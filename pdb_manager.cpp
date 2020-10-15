#include "pdb_manager.h"
#include "Callback.h"
#include "source_info.h"
#include <atlbase.h>

#include <comutil.h>

#pragma comment(lib, "comsuppw.lib")

bool PdbManager::LoadPdb (
  const wchar_t* szFilename,
  IDiaDataSource** ppSource,
  IDiaSession** ppSession,
  IDiaSymbol** ppGlobal)
{
  wchar_t wszExt[MAX_PATH];
  const wchar_t* wszSearchPath = L"SRV**\\\\symbols\\symbols"; // Alternate path to search for debug data
  DWORD dwMachType = 0;

  HRESULT hr = CoInitialize(NULL);

  // Obtain access to the provider

  hr = CoCreateInstance(__uuidof(DiaSource),
    NULL,
    CLSCTX_INPROC_SERVER,
    __uuidof(IDiaDataSource),
    (void**)ppSource);

  if (FAILED(hr)) {
    wprintf(L"CoCreateInstance failed - HRESULT = %08X\n", hr);

    return false;
  }

  _wsplitpath_s(szFilename, NULL, 0, NULL, 0, NULL, 0, wszExt, MAX_PATH);
  wprintf(L"%s\n", wszExt);

  if (!_wcsicmp(wszExt, L".pdb")) {
    // Open and prepare a program database (.pdb) file as a debug data source

    hr = (*ppSource)->loadDataFromPdb(szFilename);

    if (FAILED(hr)) {
      wprintf(L"loadDataFromPdb failed - HRESULT = %08X\n", hr);

      return false;
    }
  }

  else {
    CCallback callback; // Receives callbacks from the DIA symbol locating procedure,
                        // thus enabling a user interface to report on the progress of
                        // the location attempt. The client application may optionally
                        // provide a reference to its own implementation of this
                        // virtual base class to the IDiaDataSource::loadDataForExe method.
    callback.AddRef();

    // Open and prepare the debug data associated with the executable
    hr = (*ppSource)->loadDataForExe(szFilename, wszSearchPath, &callback);

    if (FAILED(hr)) {
      wprintf(L"loadDataForExe failed - HRESULT = %08X\n", hr);

      return false;
    }
  }

  // Open a session for querying symbols

  hr = (*ppSource)->openSession(ppSession);

  if (FAILED(hr)) {
    wprintf(L"openSession failed - HRESULT = %08X\n", hr);

    return false;
  }

  // Retrieve a reference to the global scope

  hr = (*ppSession)->get_globalScope(ppGlobal);

  if (hr != S_OK) {
    wprintf(L"get_globalScope failed\n");

    return false;
  }

  // Set Machine type for getting correct register names

  if ((*ppGlobal)->get_machineType(&dwMachType) == S_OK) {
    switch (dwMachType) {
    case IMAGE_FILE_MACHINE_I386: g_dwMachineType = CV_CFL_80386; break;
    case IMAGE_FILE_MACHINE_IA64: g_dwMachineType = CV_CFL_IA64; break;
    case IMAGE_FILE_MACHINE_AMD64: g_dwMachineType = CV_CFL_AMD64; break;
    }
  }
}

bool PdbManager::getSymbolNamebyRVA(std::string& buffer, DWORD addr) {
  IDiaSymbol* pFunc;
  BSTR buf;
  BOOL is_func;
  DWORD dwSymTag;

  if (g_pDiaSession->findSymbolByRVA(addr, SymTagPublicSymbol, &pFunc) != S_OK)
    return false;

  if (pFunc->get_function(&is_func) == S_OK) {
    if (is_func) {
      pFunc->get_name(&buf);
      buffer = _com_util::ConvertBSTRToString(buf);
      pFunc->Release();
      return true;
    }
  }
  return false;
}

bool PdbManager::getFunctionNameByRVA(std::string& buffer, DWORD addr) {
  IDiaSymbol* pFunc;
  BSTR buf;
;
 
  if (g_pDiaSession->findSymbolByRVA(addr, SymTagFunction, &pFunc) == S_OK) {
    if (pFunc->get_name(&buf) == S_OK) {
      buffer = _com_util::ConvertBSTRToString(buf);
      return true;
    }
    pFunc->Release();
  }

 
  return false;
}

std::vector<DWORD> PdbManager::getLineNumbersByRVA(DWORD rva)
{
  std::vector<DWORD> lineNums = {};
  //IDiaSymbol* pFunc;
  //if (g_pDiaSession->findSymbolByRVA(rva, SymTagFunction, &pFunc) != S_OK) {
  //  return vector<DWORD>{0};
  //}

  IDiaEnumLineNumbers* pEnum = NULL;
  //DWORD dwrva;
  //ULONGLONG length;
  
  //if (pFunc->get_relativeVirtualAddress(&dwrva) == S_OK) {
    //pFunc->get_length(&length);
    if (g_pDiaSession->findLinesByRVA(rva, 0x100/*static_cast<DWORD>(length)*/, &pEnum) == S_OK) {

      IDiaLineNumber* pLine = NULL;
      DWORD celt;
      DWORD dwLinenum;
      while (SUCCEEDED(pEnum->Next(1, &pLine, &celt)) && (celt == 1)) {
        if (pLine->get_lineNumber(&dwLinenum) == S_OK) {
          lineNums.push_back(dwLinenum);
        }
        pLine->Release();
      }
      if (lineNums.empty())
        lineNums.push_back(0);
    }
  //}
  pEnum->Release();
  return lineNums;
}

bool PdbManager::getAllSymbols(SymMap& map)
{
  CComPtr<IDiaEnumSymbols> pEnumSymbols;
  CComPtr<IDiaSymbol> pSymbol;

  DWORD dwSymTag = 0;
  DWORD rva = 0;
  ULONGLONG length = 0;
  BSTR bstrName = SysAllocString(L"");
  ULONG celt = 0;
  std::vector<DWORD> lines = {};
  std::string symbolName;
  BOOL is_func;

  if (g_pGlobalSymbol->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols) != S_OK) {
    return false;
  }

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
    lines.clear();

    if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
      continue;
    }
    
    if (dwSymTag == SymTagPublicSymbol) {
      if (pSymbol->get_name(&bstrName) == S_OK) {
        
        //BOOL is_code;
        
        //if (FAILED(pSymbol->get_code(&is_code) && pSymbol->get_function(&is_func)))
        //  return false;

        pSymbol->get_function(&is_func);

        if(is_func) {
          if (pSymbol->get_relativeVirtualAddress(&rva) == S_OK) {
            if (pSymbol->get_length(&length) == S_OK) {
              CComPtr< IDiaEnumLineNumbers > pEnum;
              if (g_pDiaSession->findLinesByRVA(rva, static_cast<DWORD>(length), &pEnum) == S_OK) {

                CComPtr<IDiaLineNumber> pLine;
                DWORD celt;
                DWORD dwLinenum;
                while (SUCCEEDED(pEnum->Next(1, &pLine, &celt)) && (celt == 1)) {
                  if (pLine->get_lineNumber(&dwLinenum) == S_OK) {
                    lines.push_back(dwLinenum);
                  }
                  pLine = NULL;
                }
                pEnum = NULL;
              }
              else {
                lines.push_back(0);
              }
              symbolName = _com_util::ConvertBSTRToString(bstrName);
              map[symbolName] = new SourceInfo(rva, static_cast<DWORD>(length), lines);
            }
          }
        }
      }
    }
    pSymbol = NULL;
  }
  return true;
}