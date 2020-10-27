#include "common.h"

#include "windows.h"
#include <vector>

#ifndef SOURCE_INFO_H
#define SOURCE_INFO_H

class SourceInfo {
public:
  SourceInfo(DWORD rva, DWORD length, std::vector<uint32_t> lines) :
    _rva(rva),
    _length(length),
    _lines(lines) {}

  uint32_t getStartLineNumber() {
    if (!_lines.empty())
      return _lines[0];
  }
  std::vector<uint32_t> getLineNumbers() {
    return _lines;
  }
  DWORD getRVA() {
    return _rva;
  }

private:
  uint32_t _rva;
  uint32_t _length;
  std::string func_name;
  std::vector<uint32_t> _lines;
//  InlineInfo* inlinees;
};

/*class InlineInfo
{

};
*/
typedef std::map<std::string, SourceInfo*> SymMap;
typedef std::map<uint32_t, uint32_t> FuncBody;
//typedef std::map < std::string, AddressCountMap> ProfileMap; // map<line,count>
typedef std::multimap < std::string, std::pair<uint32_t, uint32_t>> ProfileMap;

#endif // !SOURCE_INFO_H
