#include "sample_reader.h"
#include  <windows.h>

bool SampleReader::Read() {
	FILE* fp; 
	errno_t err;

	err = _wfopen_s(&fp, _profile_file, L"r");

	if (err != 0) {
		printf("Cannot open profile file to read");
		return false;
	}
	else {
		// Read the first line and skip it
		char buf[100];
		fgets(buf, 100, fp);


		// Read in AddressCountMap
		DWORD addr, count = 0;
		while (fscanf(fp, "%lx,%lu", &addr, &count) != EOF) {
			_address_count_map[addr] += count;
		}
		
		fclose(fp);
		return true;
	}
}

void SampleReader::Dump() {
	for (auto it = _address_count_map.cbegin(); it != _address_count_map.cend(); ++it) {
		printf("Map [%llx] : [%llu]\n", it->first, it->second);
	}
}

set<DWORD> SampleReader::GetSampledAddresses() const {
	set<DWORD> addrs;
	for (const auto& addr_count : _address_count_map) {
		addrs.insert(addr_count.first);
	}
	return addrs;
}

AddressCountMap SampleReader::getAddressCountMap() const {
  return _address_count_map;
}