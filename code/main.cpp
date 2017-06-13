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
    - Execute commands from *.txt file line by line (bat command)
    - Configuration file for batch files
  TODO:
    - Printing in-game message
    - Play sound at command activation
    - Get rid of c++ string and stdio.h
    - Get rid of CRL
    - Code cleanup
*/

#include <stdio.h>
#include <string>
#include <windows.h>

#include "types.h"

internal HMODULE mwsilver = 0;

#include "utils.cpp"
#include "mw_functions.cpp"
#include "batch_processor.cpp"

//NOTE(adm244): addresses for hooks (morrowind 1.6.1820)
internal const uint32 mainloop_hook_patch_address = 0x00417227;
internal const uint32 mainloop_hook_return_address = 0x0041722D;

internal bool isKeyHomeEnabled = true;
internal bool isKeyEndEnabled = true;

internal void GameLoop()
{
  if( IsActivated(&CommandToggle) ) {
    keys_active = !keys_active;
    ConsolePrint(globalStatePointer, "[INFO] Commands are %s", keys_active ? "on" : "off");
  }
  
  if( keys_active ){
    for( int i = 0; i < batches_count; ++i ){
      if( IsActivated(batches[i].key, &batches[i].enabled) ){
        ExecuteBatch(batches[i].filename);
        ConsolePrint(globalStatePointer, "[STATUS] %s was successeful", batches[i].filename);
      }
    }
  }
}

internal void __declspec(naked) GameLoop_Hook()
{
  __asm {
    pushad
    call GameLoop
    popad
    
    //NOTE(adm244): original instructions
    mov ecx, dword ptr ds:[0x007C6CDC]

    jmp [mainloop_hook_return_address]
  }
}

internal BOOL WINAPI DllMain(HMODULE instance, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH) {
    mwsilver = instance;
    //MessageBox(0, "MWSilver is loaded!", "Yey!", MB_OK);
    
    //NOTE(adm244): patching in the morrowind main loop
    WriteRelJump(mainloop_hook_patch_address, (uint32)&GameLoop_Hook);
    SafeWrite8(mainloop_hook_patch_address + 5, 0x90);
    
    if( !Initilize() ) {
      MessageBox(0, "Batch files could not be located!", "Yey!", MB_OK | MB_ICONERROR);
    }
  }

  return TRUE;
}
