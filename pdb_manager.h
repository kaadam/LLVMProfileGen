
#ifndef PDB_MANAGER_H
#define PDB_MANAGER_H

#include <dia2.h>
#include <vector>

#include "common.h"
#include <map>
#include "source_info.h"
#include <atlbase.h>

class PdbManager {
public:
	PdbManager(const wchar_t* pdb_filename) :
    g_input_pdb_file(pdb_filename)
	{
		bool ret = LoadPdb(g_input_pdb_file,
			&g_pDiaDataSource,
			&g_pDiaSession,
			&g_pGlobalSymbol
			);
		if (!ret) printf("Faild to load pdb file..");
	}
		~PdbManager() {
//		delete g_szFilename;
//		delete g_pDiaDataSource;
//		delete g_pDiaSession;
//		delete g_pGlobalSymbol;
	}
	bool getSymbolNamebyRVA(std::string&, DWORD);
	bool getFunctionNameByRVA(std::string&, DWORD);
  void getLineNumbersForSymbol(std::vector<uint32_t>& lineNums, DWORD rva, ULONGLONG length);
	void getLineNumberByRVA(std::vector<uint32_t>& lineNums, DWORD rva);
  bool findAllPublicSymbols(SymMap&);
  IDiaSymbol* findFunctionSymbol(uint32_t rva);
  IDiaSymbol* findPublicSymbol(uint32_t rva);

private:

  void getLineNumber(IDiaEnumLineNumbers* pEnum, std::vector<uint32_t>& linNums, bool first_line = false);
  bool findInlinedFunctions(IDiaSymbol*, std::vector<std::string>&);
  bool getInlineFileNumbers(IDiaSymbol*, std::vector<uint32_t>&);
  void processInlineSite(IDiaSymbol* symbol);
  bool LoadPdb(
    const wchar_t*,
    IDiaDataSource**,
    IDiaSession**,
    IDiaSymbol**);

  const wchar_t* g_input_pdb_file;
  CComPtr<IDiaDataSource> g_pDiaDataSource;
  CComPtr<IDiaSession> g_pDiaSession;
  CComPtr<IDiaSymbol> g_pGlobalSymbol;
  // Todo: remove this later
  DWORD g_dwMachineType = CV_CFL_80386;
};

#endif // !PDB_MANAGER_H
