#ifndef PROFILE_CREATOR_H
#define PROFILE_CREATOR_H

#include <vector>

#include "common.h"
#include "sample_reader.h"
#include "symbol_map.h"
#include "pdb_manager.h"

class ProfileCreator {
public:
	ProfileCreator() : reader(nullptr),
                     sym_map(nullptr) {};

  bool CreateProfile(const wchar_t* input_prof_file, const wchar_t* pdb_filename);

  SampleReader& getSampleReader();
  SymbolMap& getSymbolMap();

  //bool ProfileWriter(const wchar_t& output);

private:
	SampleReader* reader;
  SymbolMap* sym_map;
//  wchar_t* pdb_filnename;
//  wchar_t* csv_filename;
};

#endif // !PROFILE_CREATOR_H
