#pragma once
#ifndef WRITER_H
#define WRITER_H

#include <map>
#include "symbol_map.h"

class Writer {
public:
  Writer() : out_filename("text-profile.txt") {}
  Writer(const char* output_file) : out_filename(output_file) {}

  void Dump(std::map<std::string, Symbol*>);

private:
  const char* out_filename;
};


#endif // !WRITER_H
