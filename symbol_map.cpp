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
  uint32_t offset = line - start_line;
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
  if (!pdb->getAllSymbols(_symbolNameLineMap))
    return false;
  return true;
}

void SymbolMap::BuildProfileMap(AddressCountMap &count_map)
{
  std::string mangledName;
  DWORD line = 0;
  DWORD offset = 0;
  for (const auto& item : count_map)
  {
      if (pdb->getSymbolNamebyRVA(mangledName, item.first))
      {

        line = pdb->getLineNumbersByRVA(item.first)[0];
        //_pmap[mangledName] = make_pair(line, item.second);
        if (line > 0) {
          _pmap.insert(make_pair(mangledName, make_pair(line, item.second)));
        }
      }
  }
  
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
