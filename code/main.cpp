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
*/

#include <windows.h>

#include "mw_functions.cpp"

//NOTE(adm244): addresses for hooks (morrowind 1.6.1820)
internal const uint32 mainloop_hook_patch_address = 0x00417227;
internal const uint32 mainloop_hook_return_address = 0x0041722D;

internal bool isKeyHomeEnabled = true;
internal bool isKeyEndEnabled = true;

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

internal void GameLoop()
{
  if( IsActivated(VK_HOME, &isKeyHomeEnabled) ){
    ExecuteScript("set gamehour to 0");
  
    //CompileAndRun(*addr, "set gamehour to 0", 1, 0, 0, 0, 0);
    //PrintToConsole(0x007C67DC, "This is a test!");
    //MessageBox(NULL, "Home key is pressed!", "Yey!", MB_OK);
  }
  
  if( IsActivated(VK_END, &isKeyEndEnabled) ){
    //ConsolePrint("This %s a test %d!", "is", 123);
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
