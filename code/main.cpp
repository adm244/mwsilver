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
  IMPLEMENTED:
    - Main game loop hook
    - Printing to console
    - Execute console command
  TODO:
    - Execute commands from *.txt file line by line (bat command)
    - Configuration file for batch files
  
  RE:
    Possible CompileAndRun address: 0x0050E5A0
      method of object "[0x007C67DC] + 0x54"
      
      // unk0 = eax = [[[0x007C67DC] + 0x2E4] + 0x20]
      // eax = [edx+20h] - object or variable
      // edx = [ecx+2E4h] - some object
      // ecx = 0x007C67DC - object
      
      // globalState - Global Script state object
      // text - Script text
      // unk2 - some value (1 or 2)
      // unk3 - some object (can be null)
      // unk4 - some object | 0x007C67E0 + 0x3C [+ 0x8] | (can be null)
      // unk5 - some object (can be null)
      // unk6 - some object (can be null)
      void __cdecl CompileAndRun(uint32 globalState, char *text, int unk2, uint32 unk3, uint32 unk4, int unk5, int unk6);
    
    Object 007C67DC:
      0x007C67DC + 0x54 -- Script object(?)
*/

#include <windows.h>

#define internal static

typedef unsigned char uint8;
typedef unsigned long uint32;

//NOTE(adm244): addresses for hooks (morrowind 1.6.1820)
internal const uint32 mainloop_hook_patch_address = 0x00417227;
internal const uint32 mainloop_hook_return_address = 0x0041722D;

internal bool isKeyHomeEnabled = true;

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

// test
//NOTE(adm244): unk0 should be 0x007C67DC
//  (it's pointer to a 32-bit value, probably a pointer to some object)
typedef int (__cdecl *_PrintToConsole)(uint32 unk0, char *format, ...);
extern const _PrintToConsole PrintToConsole;

internal const _PrintToConsole PrintToConsole = (_PrintToConsole)0x0040F970;
// ###

internal void GameLoop()
{
  if( IsActivated(VK_HOME, &isKeyHomeEnabled) ){
    char *text = "set gamehour to 0";
    
    uint32 masterImage = *(uint32 *)0x007C67DC;
    uint32 obj1 = *(uint32 *)(masterImage + 0x2E4);
    uint32 addr = *(uint32 *)(obj1 + 0x20);
    
    uint32 thisPointer = *(uint32 *)(masterImage + 0x54);
    
    int const funcCompileAndRun = 0x0050E5A0;
  
    __asm {
      mov ecx, [thisPointer]
    
      push 0
      push 0
      push 0
      push 0
      push 1
      push text
      push [addr]
      
      call funcCompileAndRun
    }
  
    //CompileAndRun(*addr, "set gamehour to 0", 1, 0, 0, 0, 0);
    //PrintToConsole(0x007C67DC, "This is a test!");
    //MessageBox(NULL, "Home key is pressed!", "Yey!", MB_OK);
  }
}

// ----
internal void SafeWrite8(uint32 addr, uint32 data)
{
  uint32 oldProtect;

  VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
  *((uint8 *)addr) = data;
  VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

internal void SafeWrite32(uint32 addr, uint32 data)
{
  uint32 oldProtect;

  VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
  *((uint32 *)addr) = data;
  VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
}

internal void WriteRelJump(uint32 jumpSrc, uint32 jumpTgt)
{
  // jmp rel32
  SafeWrite8(jumpSrc, 0xE9);
  SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}
// ----

internal void __declspec(naked) GameLoop_Hook()
{
  __asm {
    pushad
    call GameLoop
    popad
    
    //NOTE(adm244): original instructions
    mov ecx,dword ptr ds:[0x007C6CDC]

    jmp [mainloop_hook_return_address]
  }
}

internal BOOL WINAPI DllMain(HANDLE procHandle, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH) {
    MessageBox(NULL, "MWSilver is loaded!", "Yey!", MB_OK);
    
    //NOTE(adm244): patching in the morrowind main loop
    WriteRelJump(mainloop_hook_patch_address, (uint32)&GameLoop_Hook);
    SafeWrite8(mainloop_hook_patch_address + 5, 0x90);
  }

  return TRUE;
}
