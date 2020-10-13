// create_llvm_profile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "profile_creator.h"

using namespace std;

void printHelpOptions()
{
  static const wchar_t* const helpString =
    L"usage: SampledProfileGenerator.exe <filename.csv> \
    <filename.pdb>\n"
    L"  -?                : print this help\n";

  wprintf(helpString);
}

int wmain(int argc, wchar_t* argv[])
{
  const wchar_t* g_szFilename;
  const wchar_t* g_szSampleFilename;
  FILE* fp;

  if (argc < 2) {
    printHelpOptions();
    return -1;
  }

  for (int i = 0; i < argc; ++i) {
    if (_wfopen_s(&fp, argv[i], L"r") || !fp) {
      // invalid file name or file does not exist

      printHelpOptions();
      return -1;
    }
    fclose(fp);
  }

  g_szFilename = argv[1];
  g_szSampleFilename = argv[2];

  ProfileCreator creator;
  creator.CreateProfile(g_szFilename, g_szSampleFilename);

  return 0;
}