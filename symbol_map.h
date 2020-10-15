#ifndef SYMBOL_MAP_H
#define SYMBOL_MAP_H

#include "common.h"
#include "pdb_manager.h"
#include "source_info.h"
#include <dia2.h>

#include <map>
#include <vector>
#include <set>
#include <string>

// Map function's line offsets to count
//typedef std::map<DWORD, DWORD> FuncBody;


class Symbol {
public:
  Symbol() :
    total_count(0),
    head_count(0),
    start_line(0),
    mangledName(),
    body() {}

  Symbol(std::vector<DWORD> lines) :
    total_count(0),
    head_count(0),
    start_line(0),
    mangledName(),
    body()
  {
    if (lines.size() > 0) {
      uint16_t offset = 0;
      start_line = lines[0];
      for (int i = 1; i < lines.size(); ++i) {
        offset = lines[i] - start_line;
        if (offset > 0)
          body.insert(std::make_pair(offset, 0));
        //printf("lines[i] - strart_line = offset: %u - %u = %u\n", lines[i], start_line, offset);
      }
    }
  }

  void addTotalCount(DWORD tcount);
  void addHeadCount(DWORD hcount);

  DWORD getTotalCount();
  DWORD getHeadCount();
  FuncBody getFunctionBody();

  void updateFuncBody(DWORD line, DWORD count);

private:
	DWORD total_count;
	DWORD head_count;
	DWORD start_line;
  BSTR mangledName;
	// Map line offset to count
	FuncBody body;
};

class SymbolMap {
public:
  SymbolMap(const wchar_t* pdb_filename) :
    _symbolNameLineMap(),
    _pmap(),
    pdb(new PdbManager(pdb_filename)) {};

  bool BuildSymbolMap();
  void BuildProfileMap(AddressCountMap &count_map);
  set<std::string> getSampledFunctions();
  ProfileMap getProfileMap();
  
  SymMap getSymbolMap();
  
private:
  SymMap _symbolNameLineMap;
  ProfileMap _pmap;
  PdbManager *pdb;

};

#endif // SYMBOL_MAP_H