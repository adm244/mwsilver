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

/*
  RE:
    CompileAndRun address: 0x0050E5A0
      method of object "[0x007C67DC] + 0x54"
      
      // globalState = eax = [[[0x007C67DC] + 0x2E4] + 0x20]
      // eax = [edx+20h] - object or variable
      // edx = [ecx+2E4h] - some object
      // ecx = 0x007C67DC - object
      
      // globalScriptState - Global Script state object
      // text - Script text
      // unk2 - some value (1 or 2)
      // unk3 - some object (can be null)
      // unk4 - some object | [0x007C67E0] + 0x3C [+ 0x8] | (can be null)
      // unk5 - some object (can be null)
      // unk6 - some object (can be null)
      void CompileAndRun(uint32 globalScriptState, char *text, int unk2, uint32 unk3, uint32 unk4, int unk5, int unk6);
    
    Object 007C67DC:
      [0x007C67DC] + 0x54 -- Script object(?)
*/

#include <cstdio>
#include <cstdarg>
#include "types.h"

internal uint32 globalStatePointer = 0x007C67DC;

//NOTE(adm244): unk0 should be 0x007C67DC
//typedef int (__cdecl *_ConsolePrint)(uint32 globalStatePointer, char *format, ...);
//extern const _ConsolePrint ConsolePrint;

//internal const _ConsolePrint ConsolePrint = (_ConsolePrint)0x0040F970;
internal const uint32 ConsolePrintAddress = 0x0040F970;
internal const uint32 CompileAndRunAddress = 0x0050E5A0;

internal uint32 GetGlobalState()
{
  return *(uint32 *)0x007C67DC;
}

internal uint32 GetGlobalScript(uint32 globalState)
{
  return *(uint32 *)(globalState + 0x54);
}

internal uint32 GetGlobalScriptState(uint32 globalState)
{
  uint32 unknownObject = *(uint32 *)(globalState + 0x02E4);
  return *(uint32 *)(unknownObject + 0x20);
}

internal void __cdecl ConsolePrint(char *format, ...)
{
  //NOTE(adm244): 260 bytes is used in original function
  char text[260];
  
  va_list list;
  va_start(list, format);
  
  vsprintf_s(text, format, list);
  
  va_end(list);
  
  __asm {
    //FIX(adm244): doesn't work (another pointer dereference issue?)
    push dword ptr text
    push globalStatePointer
    
    call ConsolePrintAddress
  }
}

internal void ExecuteScript(char *text)
{
  uint32 globalState = GetGlobalState();
  uint32 globalScriptState = GetGlobalScriptState(globalState);
  uint32 globalScript = GetGlobalScript(globalState);

  __asm {
    mov ecx, [globalScript]
  
    push 0
    push 0
    push 0
    push 0
    push 1
    push text
    push [globalScriptState]
    
    call CompileAndRunAddress
  }
}