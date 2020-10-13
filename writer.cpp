#include "writer.h"

void Writer::Dump(std::vector<std::pair<std::string, Symbol*>> profileMap)
{
  FILE* fp = fopen(out_filename, "w");
  if (!fp) {
    printf("File opening failed to write");
  }
  for (auto& item : profileMap) {
    fprintf(fp, "%s:%d:%d\n", item.first.c_str(), item.second->getTotalCount(),
                            item.second->getHeadCount());
    for (auto& func_body : item.second->getFunctionBody()) {
      fprintf(fp, " %d: %d\n", func_body.first, func_body.second);
    }
  }
  fclose(fp);
}
