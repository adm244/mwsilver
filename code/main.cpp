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
    - Printing in-game message
    - Determine loading screen and main menu (probably through hooks on load and new game)
  TODO:
    - Automate hooking proccess (either patch on the call, or something more complicated)
    - Save game, Auto Save
    - Message with progress bar (on game load and game save)
    - Get rid of c++ string and stdio.h
    - Get rid of CRL
    - Code cleanup
  DROPPED:
    - Play sound at command activation (use PlaySound in your batch files)
*/

#include <stdio.h>
#include <string>
#include <windows.h>

#include "types.h"

internal HMODULE mwsilver = 0;

#include "utils.cpp"
#include "mw_functions.cpp"
#include "batch_processor.cpp"
#include "hooks.cpp"

internal bool isKeyHomeEnabled = true;
internal bool isKeyEndEnabled = true;

internal void GameLoop()
{
  if( IsActivated(&CommandToggle) ) {
    keys_active = !keys_active;
    ConsolePrint(globalStatePointer, "[INFO] Commands are %s", keys_active ? "on" : "off");
    
    char str[260];
    sprintf(str, "[INFO] Commands are %s", keys_active ? "on" : "off");
    ShowGameMessage(str, 0, 1);
    
    //NOTE(adm244): forgot about "this" pointer
    /*char buffer[1024];
    GetRandomSplashPath(buffer, 3);
    ShowGameMessage(buffer, 0, 1);*/
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

internal void FirstLoadStart()
{
  MessageBox(0, "Loading started...", "Yey!", MB_OK);
}

internal void FirstLoadEnd()
{
  MessageBox(0, "Loading ended...", "Yey!", MB_OK);
}

internal void ReloadStart()
{
  MessageBox(0, "Reloading started...", "Yey!", MB_OK);
}

internal void ReloadEnd()
{
  MessageBox(0, "Reloading ended...", "Yey!", MB_OK);
}

internal BOOL WINAPI DllMain(HMODULE instance, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH) {
    mwsilver = instance;
    
    HookFirstLoading();
    HookReloading();
    HookMainLoop();
    
    if( !Initilize() ) {
      MessageBox(0, "Batch files could not be located!", "Error", MB_OK | MB_ICONERROR);
    }
  }

  return TRUE;
}
