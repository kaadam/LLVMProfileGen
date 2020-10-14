#pragma once
#ifndef PDB_MANAGER_H
#define PDB_MANAGER_H

#include <dia2.h>
#include <vector>

#include "common.h"
#include <map>
#include "source_info.h"

class PdbManager {
public:
	PdbManager(const wchar_t* pdb_filename) :
		g_szFilename(pdb_filename)
	{
		bool ret = LoadPdb(g_szFilename,
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
	std::vector<DWORD> getLineNumbersByRVA(DWORD);
  bool getAllSymbols(SymMap&);

private:
	bool LoadPdb(
		const wchar_t*,
		IDiaDataSource**,
		IDiaSession**,
		IDiaSymbol**);

	const wchar_t* g_szFilename;
	IDiaDataSource* g_pDiaDataSource;
	IDiaSession* g_pDiaSession;
	IDiaSymbol* g_pGlobalSymbol;
	DWORD g_dwMachineType = CV_CFL_80386;
};

#endif // !PDB_MANAGER_H
