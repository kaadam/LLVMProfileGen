#include "profile_creator.h"
#include "sample_reader.h"
#include "pdb_manager.h"
#include <string>
#include "writer.h"
#include <algorithm>

bool sortByVal(const pair<std::string, Symbol*>& a,
  const pair<std::string, Symbol*>& b) {
  return (a.second->getTotalCount() > b.second->getTotalCount());
}

bool ProfileCreator::CreateProfile(const wchar_t *input_profile,
                                   const wchar_t *input_pdb_file) {
  reader = new SampleReader(input_profile);
  reader->Read();
  AddressCountMap count_map = reader->getAddressCountMap();

  SymbolMap syms(input_pdb_file, count_map);
  
  SymMap map = syms.getSymbolMap();
  std::map<std::string, Symbol*> profile;
  for (auto& item : syms.getSampledFunctions()) {
    if (map.count(item)) {
      //printf("function: %s : rva: %lx \n", item.c_str(), map[item]->getRVA());
      profile.insert(std::make_pair(item, new Symbol(map[item]->getLineNumbers())));
    }
  }
  
  ProfileMap pmap = syms.getProfileMap();

  for (auto& item : pmap) {
    profile[item.first]->updateFuncBody(item.second.first, item.second.second);
  }

  // Sort
  // reserve prealloc size of pmap
  vector<pair<std::string, Symbol*>> vec;
  std::map<std::string, Symbol*> :: iterator it;
  for (it = profile.begin(); it != profile.end(); ++it) {
    vec.push_back(std::make_pair(it->first, it->second));
  }
  sort(vec.begin(), vec.end(), sortByVal);

  Writer w;
  w.Dump(vec);

  return true;
}

SampleReader& ProfileCreator::getSampleReader()
{
  return *reader;
}

SymbolMap& ProfileCreator::getSymbolMap()
{
  return *sym_map;
}
