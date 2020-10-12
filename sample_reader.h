#ifndef SAMPLE_READER_H
#define SAMPLE_READER_H

#include "common.h"
#include <map>
#include <set>
#include <dia2.h>



class SampleReader {
public:
	SampleReader(const wchar_t *profile_file) :
		_profile_file(profile_file),
		total_count(0) {}
	bool Read();

	void Dump();
	set<DWORD> GetSampledAddresses() const;
  AddressCountMap getAddressCountMap() const;

	//uint64_t getSampledCountByAddress(uint64_t addr);
  ~SampleReader() { delete _profile_file; }

private:
	const wchar_t* _profile_file;
	AddressCountMap _address_count_map;
	uint64_t total_count;

};

#endif // !SAMPLE_READER_H

