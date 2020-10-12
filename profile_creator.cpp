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

bool ProfileCreator::CreateProfile(const wchar_t *input_prof_file,
                                   const wchar_t *pdb_filename) {
  reader = new SampleReader(input_prof_file);
  //pdb = new PdbManager(pdb_filename);

  reader->Read();

  SymbolMap syms(pdb_filename);
  syms.BuildSymbolMap();
  AddressCountMap count_map = reader->getAddressCountMap();
  syms.BuildProfileMap(count_map);
  
  SymMap map = syms.getSymbolMap();
  std::map<std::string, Symbol*> profile;
  for (auto& item : syms.getSampledFunctions()) {
    if (map.count(item)) {
      profile.insert(std::make_pair(item, new Symbol(map[item]->getLineNumbers())));
    }
  }
  
  ProfileMap pmap = syms.getProfileMap();

  for (auto& item : pmap) {
    profile[item.first]->updateFuncBody(item.second.first, item.second.second);
  }

  // Sort
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
