#include "symbol_map.h"

#include <comutil.h>

#pragma comment(lib, "comsuppw.lib")


void Symbol::addHeadCount(DWORD count) {
  head_count += count;
}

DWORD Symbol::getTotalCount()
{
  return this->total_count;
}

DWORD Symbol::getHeadCount()
{
  return this->head_count;
}

FuncBody Symbol::getFunctionBody()
{
  return this->body;
}

void Symbol::addTotalCount(DWORD count) {
  total_count += count;
}

void Symbol::updateFuncBody(DWORD line, DWORD count) {
  uint16_t offset = line - start_line;
  total_count += count;
  if (offset > 0) {
    //body.insert(std::pair<DWORD, DWORD>(offset, count));
    body[offset] += count;
  }
  else {
    head_count += count;
  }
}

bool SymbolMap::BuildSymbolMap()
{
  if (!pdb->findAllPublicSymbols(_symbolNameLineMap))
    return false;
  return true;
}

void SymbolMap::BuildProfileMap(AddressCountMap &count_map)
{
  std::string mangledName;
  DWORD offset = 0;
  std::vector<std::uint32_t> nums;
  static int a = 0;
  for (const auto& item : count_map)
  {   // Here we only search for public symbol, but not all functions have linkage name.
      if (pdb->getSymbolNamebyRVA(mangledName, item.first))
      {
        pdb->getLineNumberByRVA(nums, item.first);
        _pmap.insert(make_pair(mangledName, make_pair(nums[0], item.second)));
        a++;
      }
      else {
        
        //TODO , find symbols in Global scope (SymTagPublicSymbol, SymTagFunction)
      }
  }
  wprintf(L"all: %d", a);
}

set<std::string> SymbolMap::getSampledFunctions()
{
  set<std::string> names;
  for (const auto &name : _pmap) {
    names.insert(name.first);
  }
  return names;
}

ProfileMap SymbolMap::getProfileMap()
{
  return _pmap;
}


SymMap SymbolMap::getSymbolMap()
{
  return _symbolNameLineMap;
}
