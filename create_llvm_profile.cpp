// SampleProfileGenerator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <dia2.h>

#include "sample_reader.h"
#include "pdb_manager.h"
#include "profile_creator.h"

using namespace std;

const wchar_t* g_szFilename;
const wchar_t* g_szSampleFilename;
IDiaDataSource* g_pDiaDataSource;
IDiaSession* g_pDiaSession;
IDiaSymbol* g_pGlobalSymbol;
DWORD g_dwMachineType = CV_CFL_80386;

/*
bool LoadDataFromPdb(
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
*/
void PrintPublicSymbol(IDiaSymbol* pSymbol)
{
    DWORD dwSymTag;
    DWORD dwRVA;
    DWORD dwSeg;
    DWORD dwOff;
    BSTR bstrName;

    if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
        return;
    }

    if (pSymbol->get_relativeVirtualAddress(&dwRVA) != S_OK) {
        dwRVA = 0xFFFFFFFF;
    }

    pSymbol->get_addressSection(&dwSeg);
    pSymbol->get_addressOffset(&dwOff);

    wprintf(L"%d: [%08X][%04X:%08X] ", dwSymTag, dwRVA, dwSeg, dwOff);

    if (dwSymTag == SymTagThunk) {
        if (pSymbol->get_name(&bstrName) == S_OK) {
            wprintf(L"%s\n", bstrName);

            SysFreeString(bstrName);
        }

        else {
            if (pSymbol->get_targetRelativeVirtualAddress(&dwRVA) != S_OK) {
                dwRVA = 0xFFFFFFFF;
            }

            pSymbol->get_targetSection(&dwSeg);
            pSymbol->get_targetOffset(&dwOff);

            wprintf(L"target -> [%08X][%04X:%08X]\n", dwRVA, dwSeg, dwOff);
        }
    }

    else {
        // must be a function or a data symbol

        BSTR bstrUndname;

        if (pSymbol->get_name(&bstrName) == S_OK) {
            if (pSymbol->get_undecoratedName(&bstrUndname) == S_OK) {
                wprintf(L"%s(%s)\n", bstrName, bstrUndname);

                SysFreeString(bstrUndname);
            }

            else {
                wprintf(L"%s\n", bstrName);
            }

            SysFreeString(bstrName);
        }
    }
}

bool DumpAllPublics(IDiaSymbol* pGlobal)
{
    wprintf(L"\n\n*** PUBLICS\n\n");

    // Retrieve all the public symbols

    IDiaEnumSymbols* pEnumSymbols;

    if (FAILED(pGlobal->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols))) {
        return false;
    }

    IDiaSymbol* pSymbol;
    ULONG celt = 0;

    while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
        PrintPublicSymbol(pSymbol);

        pSymbol->Release();
    }

    pEnumSymbols->Release();

    putwchar(L'\n');

    return true;
}



int wmain(int argc, wchar_t* argv[])
{
    std::map<string, int> SampledData;
    
    g_szFilename = argv[1];
    g_szSampleFilename = argv[2];

    ProfileCreator creator;
    creator.CreateProfile(g_szSampleFilename, g_szFilename);

 /*   SampleReader reader(g_szSampleFilename);
    reader.Read();
    //reader.Dump();
    PdbManager pdb(g_szFilename);
    BSTR buff;
    BSTR mangled;
    pdb.getSymbolNamebyRVA(&buff, 0x0000000000006E0A);
    pdb.getFunctionNameByRVA(&mangled, 0x0000000000006E0A);
    wprintf(L"Szimi: %s", buff);
    wprintf(L"Mangled: %s", mangled);
    SysFreeString(buff);
    SysFreeString(mangled);
    */
    /*
    if (!LoadDataFromPdb(g_szFilename, &g_pDiaDataSource, &g_pDiaSession, &g_pGlobalSymbol)) {
        return -1;
    }
*/
    //DumpAllPublics(g_pGlobalSymbol);


    return 0;
}