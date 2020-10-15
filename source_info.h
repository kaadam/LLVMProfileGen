#pragma once
#include "common.h"
#include "windows.h"
#include <vector>

#ifndef SOURCE_INFO_H
#define SOURCE_INFO_H

class SourceInfo {
public:
  SourceInfo(DWORD rva, DWORD length, std::vector<DWORD> lines) :
    _rva(rva),
    _length(length),
    _lines(lines) {}

  DWORD getStartLineNumber() {
    if (!_lines.empty())
      return _lines[0];
  }
  std::vector<DWORD> getLineNumbers() {
    return _lines;
  }
  DWORD getRVA() {
    return _rva;
  }

private:
  DWORD _rva;
  DWORD _length;
  std::vector<DWORD> _lines;
};

typedef std::map<std::string, SourceInfo*> SymMap;
typedef std::map<DWORD, DWORD> FuncBody;
typedef std::multimap < std::string, std::pair<DWORD, DWORD>> ProfileMap;

#endif // !SOURCE_INFO_H
