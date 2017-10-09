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

#ifndef UTILS_CPP
#define UTILS_CPP

internal int Clamp(int value, int min, int max)
{
  if( value < min ) return min;
  if( value > max ) return max;
  
  return value;
}

internal int Absolute(int value)
{
  return (value < 0) ? value * -1 : value;
}

internal double Absolute(double value)
{
  return (value < 0) ? value * -1.0f : value;
}

//NOTE(adm244): retrieves a folder path from full path
internal std::string GetDirectoryFromPath(std::string path)
{
  return path.substr(0, path.rfind("\\") + 1);
}

//NOTE(adm244): retrieves an integer value from specified section and value of ini file
internal int IniReadInt(char *inifile, char *section, char *param, int def)
{
  char curdir[MAX_PATH];
  GetModuleFileNameA(mwsilver, curdir, sizeof(curdir));
  std::string fname = GetDirectoryFromPath(curdir) + inifile;
  
  return GetPrivateProfileIntA(section, param, def, fname.c_str());
}

//NOTE(adm244): retrieves a bool value from specified section and value of ini file
internal bool IniReadBool(char *inifile, char *section, char *param, bool def)
{
  int value = IniReadInt(inifile, section, param, def ? 1 : 0);
  return(value != 0 ? true : false);
}

//NOTE(adm244): retrieves a string value from specified section and value of ini file and stores it in buffer
internal int IniReadString(char *inifile, char *section, char *param, char *default, char *output, int size)
{
  char buffer[MAX_PATH];
  GetModuleFileNameA(mwsilver, buffer, sizeof(buffer));
  std::string fname = GetDirectoryFromPath(buffer) + inifile;

  return GetPrivateProfileStringA(section, param, default, output, size, fname.c_str());
}

//NOTE(adm244): retrieves all key-value pairs from specified section of ini file and stores it in buffer
internal DWORD IniReadSection(char *inifile, char *section, char *buffer, DWORD bufsize)
{
  char curdir[MAX_PATH];
  GetModuleFileNameA(mwsilver, curdir, sizeof(curdir));
  std::string fname = GetDirectoryFromPath(curdir) + inifile;
  
  return GetPrivateProfileSectionA(section, buffer, bufsize, fname.c_str());
}

//NOTE(adm244): returns whenever key is pressed or not
internal int GetKeyPressed(byte key)
{
  short keystate = (short)GetAsyncKeyState(key);
  return( (keystate & 0x8000) > 0 );
}

//NOTE(adm244): checks if key was pressed and locks its state
// returns true if key wasn't pressed in previous frame but it is in this one
// returns false if key is not pressed or is hold down
internal bool IsActivated(byte key, bool *enabled)
{
  if( GetKeyPressed(key) ){
    if( *enabled ){
      *enabled = false;
      return(true);
    }
  } else{
    *enabled = true;
  }
  return(false);
}

//NOTE(adm244): writes byte data at specified address
internal void SafeWrite8(uint32 addr, uint8 data)
{
  uint32 oldProtect;

  VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
  *((uint8 *)addr) = data;
  VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

//NOTE(adm244): writes 4-byte data at specified address
internal void SafeWrite32(uint32 addr, uint32 data)
{
  uint32 oldProtect;

  VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
  *((uint32 *)addr) = data;
  VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

//NOTE(adm244): writes a relative jump to specified address
internal void WriteRelJump(uint32 jumpSrc, uint32 jumpTgt)
{
  SafeWrite8(jumpSrc, 0xE9);
  SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}

#endif
