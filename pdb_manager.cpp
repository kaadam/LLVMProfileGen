#include "pdb_manager.h"
#include "Callback.h"
#include "source_info.h"

#include <comutil.h>
#pragma comment(lib, "comsuppw.lib")

#include <algorithm>

#define MAX_RVA_LINES_BYTES_RANGE 0x100

// Tags returned by Dia
const wchar_t* const rgTags[] =
{
  L"(SymTagNull)",                     // SymTagNull
  L"Executable (Global)",              // SymTagExe
  L"Compiland",                        // SymTagCompiland
  L"CompilandDetails",                 // SymTagCompilandDetails
  L"CompilandEnv",                     // SymTagCompilandEnv
  L"Function",                         // SymTagFunction
  L"Block",                            // SymTagBlock
  L"Data",                             // SymTagData
  L"Annotation",                       // SymTagAnnotation
  L"Label",                            // SymTagLabel
  L"PublicSymbol",                     // SymTagPublicSymbol
  L"UserDefinedType",                  // SymTagUDT
  L"Enum",                             // SymTagEnum
  L"FunctionType",                     // SymTagFunctionType
  L"PointerType",                      // SymTagPointerType
  L"ArrayType",                        // SymTagArrayType
  L"BaseType",                         // SymTagBaseType
  L"Typedef",                          // SymTagTypedef
  L"BaseClass",                        // SymTagBaseClass
  L"Friend",                           // SymTagFriend
  L"FunctionArgType",                  // SymTagFunctionArgType
  L"FuncDebugStart",                   // SymTagFuncDebugStart
  L"FuncDebugEnd",                     // SymTagFuncDebugEnd
  L"UsingNamespace",                   // SymTagUsingNamespace
  L"VTableShape",                      // SymTagVTableShape
  L"VTable",                           // SymTagVTable
  L"Custom",                           // SymTagCustom
  L"Thunk",                            // SymTagThunk
  L"CustomType",                       // SymTagCustomType
  L"ManagedType",                      // SymTagManagedType
  L"Dimension",                        // SymTagDimension
  L"CallSite",                         // SymTagCallSite
  L"InlineSite",                       // SymTagInlineSite
  L"BaseInterface",                    // SymTagBaseInterface
  L"VectorType",                       // SymTagVectorType
  L"MatrixType",                       // SymTagMatrixType
  L"HLSLType",                         // SymTagHLSLType
  L"Caller",                           // SymTagCaller,
  L"Callee",                           // SymTagCallee,
  L"Export",                           // SymTagExport,
  L"HeapAllocationSite",               // SymTagHeapAllocationSite
  L"CoffGroup",                        // SymTagCoffGroup
  L"Inlinee",                          // SymTagInlinee
};

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
  // Currently is not used; TODO
  if ((*ppGlobal)->get_machineType(&dwMachType) == S_OK) {
    switch (dwMachType) {
    case IMAGE_FILE_MACHINE_I386: g_dwMachineType = CV_CFL_80386; break;
    case IMAGE_FILE_MACHINE_IA64: g_dwMachineType = CV_CFL_IA64; break;
    case IMAGE_FILE_MACHINE_AMD64: g_dwMachineType = CV_CFL_AMD64; break;
    }
  }
  return true;
}

bool PdbManager::getSymbolNamebyRVA(std::string& buffer, DWORD addr) {
  CComPtr<IDiaSymbol> pFunc;
  BSTR buf;
  bool ret = false;
  BOOL is_code;

  if (g_pDiaSession->findSymbolByRVA(addr, SymTagPublicSymbol, &pFunc) == S_OK)
  {
    // Check whether Symbol is refers to a code address
    if (pFunc->get_code(&is_code) == S_OK && is_code)
    {
        pFunc->get_name(&buf);
        buffer = _com_util::ConvertBSTRToString(buf);
        SysFreeString(buf);
        return true;
    }
  }
  return ret;
}

bool PdbManager::getFunctionNameByRVA(std::string& buffer, DWORD addr) {
  CComPtr<IDiaSymbol> pFunc;
  BSTR buf;
 
  if (g_pDiaSession->findSymbolByRVA(addr, SymTagFunction, &pFunc) == S_OK) {
    if (pFunc->get_name(&buf) == S_OK) {
      buffer = _com_util::ConvertBSTRToString(buf);
      SysFreeString(buf);
    }
    pFunc = NULL;
    return true;
  }
  return false;
}

void PdbManager::getLineNumber(IDiaEnumLineNumbers* pEnum, std::vector<uint32_t>& lineNums, bool first_line) {
  CComPtr<IDiaLineNumber> pLine;
  DWORD celt;
  DWORD lineNum;
  while (SUCCEEDED(pEnum->Next(1, &pLine, &celt)) && (celt == 1)) {
    if (pLine->get_lineNumber(&lineNum) == S_OK) {
      lineNums.push_back(lineNum);
      if (first_line)
        break;
    }
    pLine = NULL;
  }
}

void PdbManager::getLineNumbersForSymbol(std::vector<uint32_t>& lineNums, DWORD rva, ULONGLONG length)
{
  CComPtr<IDiaEnumLineNumbers> pEnum;
  if (g_pDiaSession->findLinesByRVA(rva, static_cast<DWORD>(length), &pEnum) == S_OK) {
    getLineNumber(pEnum, lineNums);
    std::sort(lineNums.begin(), lineNums.end());
  }
  else {
    lineNums.push_back(0);
  }
}

void PdbManager::getLineNumberByRVA(std::vector<uint32_t>& lineNums, 
                                     DWORD rva)
{
  CComPtr<IDiaEnumLineNumbers> pEnum;
  if (g_pDiaSession->findLinesByRVA(rva, static_cast<DWORD>(MAX_RVA_LINES_BYTES_RANGE), &pEnum) == S_OK) {
    getLineNumber(pEnum, lineNums, true);
  }
  else {
    lineNums.push_back(0);
  }
}

bool PdbManager::findAllPublicSymbols(SymMap& map)
{
  CComPtr<IDiaEnumSymbols> pEnumSymbols;
  CComPtr<IDiaSymbol> pSymbol;

  BSTR bstrName = SysAllocString(L"");
  ULONG celt = 0;
  DWORD rva = 0;
  ULONGLONG length = 0;
  std::vector<uint32_t> lines = {};
  std::string symbolName;

  if (g_pGlobalSymbol->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols) != S_OK) {
    return false;
  }

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
    lines.clear();
    if (pSymbol->get_name(&bstrName) == S_OK) {
      if (pSymbol->get_relativeVirtualAddress(&rva) == S_OK) {
        if (pSymbol->get_length(&length) == S_OK) {
          getLineNumbersForSymbol(lines, rva, length);

          CComPtr<IDiaSymbol> func;
          DWORD dRVA;
          if (g_pDiaSession->findSymbolByRVA(rva, SymTagFunction, &func) == S_OK) {
            func->get_relativeVirtualAddress(&dRVA);
            BSTR function_name;
                func->get_name(&function_name);
                //getInlineSites(func);
                //wprintf(L"Function name: %s\n", function_name);
                SysFreeString(function_name);
                //getInlineSites(func);
          }
              //func = NULL;
              symbolName = _com_util::ConvertBSTRToString(bstrName);
              map[symbolName] = new SourceInfo(rva, static_cast<uint32_t>(length), lines);
        }
      }
    }
    pSymbol = NULL;
  }
  return true;
}

bool PdbManager::getInlineFileNumbers(IDiaSymbol* sym, std::vector<uint32_t> &lineNums) {
  CComPtr<IDiaEnumLineNumbers> lines;
  DWORD celt;
  std::vector<uint32_t> line_nums = {};
  if (sym->findInlineeLines(&lines) == S_OK) {
    CComPtr<IDiaLineNumber> line;
    DWORD num;
    while(SUCCEEDED(lines->Next(1, &line, &celt)) && (celt == 1 )) {
      line->get_lineNumber(&num);
      lineNums.push_back(num);
      line = NULL;
    }
    if (lineNums.empty())
      line_nums.push_back(0);
  }
  else {
    lineNums.push_back(0);
    return false;
  }
  return true;
}

bool PdbManager::findInlinedFunctions(IDiaSymbol* sym, std::vector<std::string> &inlineStack) {
  DWORD rva;
  ULONGLONG lenght;
  sym->get_relativeVirtualAddress(&rva);
  sym->get_length(&lenght);

  CComPtr<IDiaEnumSymbols> inlineFrame;
  if (sym->findInlineFramesByRVA(rva, &inlineFrame) == S_OK) {
    CComPtr<IDiaSymbol> frame;
    DWORD celt;
    BSTR name;
    while (SUCCEEDED(inlineFrame->Next(1, &frame, &celt)) && (celt == 1)) {
      frame->get_name(&name);
      char* temp = _com_util::ConvertBSTRToString(name);
      inlineStack.push_back(temp);
      frame = NULL;
    }
  }
  else {
    inlineStack.push_back("");
    return false;
  }
  return true;
}


IDiaSymbol* PdbManager::findFunctionSymbol(uint32_t rva) {
  IDiaSymbol* pSymbol = NULL;

  CComPtr<IDiaEnumSymbolsByAddr> pEnumByRVA;
  if (g_pDiaSession->getSymbolsByAddr(&pEnumByRVA) == S_OK)
  {
    CComPtr<IDiaSymbol> pSymbol;
    if (pEnumByRVA->symbolByRVA(rva, &pSymbol) == S_OK)
    {
      do
      {
        IDiaSymbol* pSymbolParent = NULL;
        DWORD tag = SymTagNull;
        pSymbol->get_symTag(&tag);

        if (tag == SymTagFunction)
        {
          break;
        }
        pSymbol->get_lexicalParent(&pSymbolParent);
        pSymbol = pSymbolParent;
      } while (NULL != pSymbol);
    }
  }
  return pSymbol;
}

IDiaSymbol* PdbManager::findPublicSymbol(uint32_t rva) {
  IDiaSymbol* pSymbol = NULL;

  CComPtr<IDiaEnumSymbolsByAddr> pEnumByRVA;
  if (g_pDiaSession->getSymbolsByAddr(&pEnumByRVA) == S_OK)
  {
    CComPtr<IDiaSymbol> pSymbol;
    if (pEnumByRVA->symbolByRVA(rva, &pSymbol) == S_OK)
    {
      do
      {
        IDiaSymbol* pSymbolParent = NULL;
        DWORD tag = SymTagNull;
        pSymbol->get_symTag(&tag);

        if (tag == SymTagPublicSymbol)
        {
          break;
        }
        pSymbol->get_lexicalParent(&pSymbolParent);
        pSymbol = pSymbolParent;
      } while (NULL != pSymbol);
    }
  }
  return pSymbol;
}
/*
void PdbManager::processInlineSite(IDiaSymbol* symbol)
{
  DWORD celt;
  CComPtr<IDiaEnumSymbols> enumSym;
  std::vector<std::string> istack;
  std::vector<uint32_t> nums;
  BSTR s;
  if (g_pDiaSession->findChildren(symbol, SymTagInlineSite, NULL, nsNone, &enumSym) == S_OK)
  {
    CComPtr<IDiaSymbol> func;
    while (SUCCEEDED(enumSym->Next(1, &func, &celt)) && (celt == 1))
    {
      if (func->get_name(&s) == S_OK)
        wprintf(L"INlineSite %s\n");
      if (getInlinedFunctionStack(func, istack))
      {
        wprintf(L"Inline stack: \n");
        for (int i = 0; i < istack.size(); ++i)
        {
          wprintf(L"  %s\n", istack[i]);
        }
      }

      if (getInlineFileNumbers(func, nums)) {
        wprintf(L"Inline file numbers: \n");
        for (int i = 0; i < istack.size(); ++i) {
          wprintf(L"  %d\n", nums[i]);
        }
      }
      func = NULL;
    }
  }
}
*/
