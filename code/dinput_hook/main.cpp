/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

//IMPORTANT(adm244): SCRATCH VERSION JUST TO GET IT UP WORKING

#include <string>
#include <windows.h>

#define internal static
#define CONFIG_FILE "dinput8.ini"
#define CONFIG_SECTION "proxy"
#define CONFIG_KEY "proxy_library"

typedef HRESULT __stdcall DInput8CreateFunc(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);

internal HMODULE mwsilver = 0;
internal HMODULE proxylib = 0;

extern "C" HRESULT __stdcall FakeDirectInput8Create(
  HINSTANCE hinst,
  DWORD dwVersion,
  REFIID riidltf,
  LPVOID * ppvOut,
  LPUNKNOWN punkOuter
)
{
  if( !proxylib ) {
    proxylib = LoadLibrary("c:\\windows\\system32\\dinput8.dll");
  }
  
  DInput8CreateFunc *DInput8Create = (DInput8CreateFunc *)GetProcAddress(proxylib, "DirectInput8Create");
  return DInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

std::string GetDirectoryFromPath(std::string path)
{
  return path.substr(0, path.rfind("\\") + 1);
}

int IniReadString(char *inifile, char *section, char *param, char *default, char *output, int size)
{
  char buffer[MAX_PATH];
  GetModuleFileNameA(mwsilver, buffer, sizeof(buffer));
  std::string fname = GetDirectoryFromPath(buffer) + inifile;

  return GetPrivateProfileStringA(section, param, default, output, size, fname.c_str());
}

internal void LoadProxyLibrary()
{
  char proxyname[MAX_PATH];
  int result = IniReadString(CONFIG_FILE, CONFIG_SECTION, CONFIG_KEY, 0, proxyname, sizeof(proxyname));
  
  if( result ) {
    proxylib = LoadLibrary(proxyname);
    //MessageBox(0, "Proxy library is loaded!", "Yey!", MB_OK);
  }
}

internal BOOL WINAPI DllMain(HANDLE procHandle, DWORD reason, LPVOID reserved)
{
  if( reason == DLL_PROCESS_ATTACH )
  {
    //MessageBox(0, "Hook library is loaded!", "Yey!", MB_OK);
    LoadProxyLibrary();
    
    mwsilver = LoadLibrary("mwsilver.dll");
    if( !mwsilver ) {
      MessageBox(0, "Plugin mwsilver.dll was not found!", "Plugin dll not found", MB_OK | MB_ICONERROR);
      return FALSE;
    }
  }

  return TRUE;
}
