// create_llvm_profile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "profile_creator.h"

using namespace std;

int wmain(int argc, wchar_t* argv[])
{
  const wchar_t* g_szFilename;
  const wchar_t* g_szSampleFilename;

  g_szFilename = argv[1];
  g_szSampleFilename = argv[2];

  ProfileCreator creator;
  creator.CreateProfile(g_szSampleFilename, g_szFilename);

  return 0;
}