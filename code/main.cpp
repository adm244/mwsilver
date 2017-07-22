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
    - Determine loading screen and main menu
    - Queue commands that were activated at loading or main menu
    - Save game, Auto Save
    - Check if player is in interior or exterior
    - Optional ingame messages
    - Queue batches that cannot be executed right away
    - Specify which commands should execute in interior\exterior
    
    - Load save filename and display name from config file
  TODO:
    - Command to activate random batch file
    - Automate hooking proccess (either patch on the call, or something more complicated)
    - Message with progress bar (on game load and game save)
    - Get rid of c++ string and stdio.h
    - Get rid of CRT
    - Code cleanup
  DROPPED:
    - Play sound at command activation (use PlaySound in your batch files)
*/

#include <stdio.h>
#include <string>
#include <windows.h>

#include "types.h"

#define CONFIG_FILE "mwsilver.ini"
#define CONFIG_SETTINGS_SECTION "settings"
#define CONFIG_PRESAVE "bSaveGamePreActivation"
#define CONFIG_POSTSAVE "bSaveGamePostActivation"
#define CONFIG_SHOWMESSAGES "bShowMessages"
#define CONFIG_SAVEFILE "sSaveFile"
#define CONFIG_SAVENAME "sSaveName"

#define CONFIG_DEFAULT_SAVEFILE "mwsilver_save"
#define CONFIG_DEFAULT_SAVENAME "MWSilver Save"

internal HMODULE mwsilver = 0;
internal uint8 IsInterior = 0;

#define STRING_SIZE 256

internal char SaveFileName[STRING_SIZE];
internal char SaveDisplayName[STRING_SIZE];

#include "randomlib.c"
#include "utils.cpp"
#include "mw_functions.cpp"
#include "batch_processor.cpp"
#include "queue.cpp"
#include "hooks.cpp"

internal HANDLE QueueHandle = 0;
internal DWORD QueueThreadID = 0;

internal Queue BatchQueue;
internal Queue InteriorPendingQueue;
internal Queue ExteriorPendingQueue;

internal bool ShowMessages = false;
internal bool ActualGameplay = false;
internal bool SavePreActivation = false;
internal bool SavePostActivation = false;

/*internal void DrawText(char *text)
{
  HWND mwWindow = FindWindow("Morrowind", 0);
  HDC mwDC = GetDC(mwWindow);
  
  RECT mwWindowRect;
  GetClientRect(mwWindow, &mwWindowRect);
  
  DrawText(mwDC, text, -1, &mwWindowRect, DT_TOP | DT_CENTER);
}*/

internal void DisplayMessage(char *message)
{
  if( ShowMessages ) {
    ShowGameMessage(message, 0, 1);
  }
}

internal void DisplaySuccessMessage(char *batchName)
{
  char statusString[260];
  sprintf(statusString, "[STATUS] %s was successeful", batchName);
  
  ConsolePrint(globalStatePointer, statusString);
  DisplayMessage(statusString);
}

internal void MakePreSave()
{
  if( SavePreActivation ) {
    SaveGame("PreActivation", "pre");
  }
}

internal void MakePostSave()
{
  if( SavePostActivation ) {
    SaveGame("PostActivation", "post");
  }
}

internal void ProcessQueue(Queue *queue, bool checkExecState)
{
  pointer dataPointer;
  
  while( dataPointer = QueueGet(queue) ) {
    BatchData *batch = (BatchData *)dataPointer;
    bool isQueueEmpty = QueueIsEmpty(queue);
    
    if( checkExecState ) {
      uint8 executionState = GetBatchExecState(batch->filename);
    
      bool executionStateValid = ((executionState == EXEC_EXTERIOR_ONLY) && !IsInterior)
        || ((executionState == EXEC_INTERIOR_ONLY) && IsInterior)
        || (executionState == EXEC_DEFAULT);
      
      if( !executionStateValid ) {
        if( executionState == EXEC_EXTERIOR_ONLY ) {
          QueuePut(&ExteriorPendingQueue, dataPointer);
        } else {
          QueuePut(&InteriorPendingQueue, dataPointer);
        }
        
        return;
      }
    }
    
    if (isQueueEmpty) MakePreSave();
  
    ExecuteBatch(batch->filename);
    DisplaySuccessMessage(batch->filename);
    
    if (isQueueEmpty) MakePostSave();
  }
}

internal DWORD WINAPI QueueHandler(LPVOID data)
{
  for(;;) {
    //DrawText("Test message string. Is it working?");
    
    if( IsActivated(&CommandToggle) ) {
      keys_active = !keys_active;
      
      //TODO(adm244): display it somehow on loading screen
      if( ActualGameplay ) {
        char infoString[260];
        sprintf(infoString, "[INFO] Commands are %s", keys_active ? "on" : "off");
        DisplayMessage(infoString);
      }
    }
  
    if( keys_active ){
      for( int i = 0; i < batches_count; ++i ){
        if( IsActivated(batches[i].key, &batches[i].enabled) ){
          QueuePut(&BatchQueue, (pointer)&batches[i]);
        }
      }
      
      if( IsActivated(&CommandRandom) ) {
        int index = RandomInt(0, batches_count - 1);
        QueuePut(&BatchQueue, (pointer)&batches[index]);
      }
    }
  }
}

internal void GameLoop()
{
  if( ActualGameplay ) {
    if( IsInterior != IsPlayerInInterior() ) {
      IsInterior = !IsInterior;
      
      if( IsInterior ) {
        ProcessQueue(&InteriorPendingQueue, false);
      } else {
        ProcessQueue(&ExteriorPendingQueue, false);
      }
    }
  
    ProcessQueue(&BatchQueue, true);
  }
}

internal void FirstLoadStart()
{
  ActualGameplay = false;
}

internal void FirstLoadEnd()
{
  ActualGameplay = true;
}

internal void ReloadStart()
{
  ActualGameplay = false;
}

internal void ReloadEnd()
{
  ActualGameplay = true;
}

internal void SettingsInitialize()
{
  SavePreActivation = IniReadBool(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_PRESAVE, false);
  SavePostActivation = IniReadBool(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_POSTSAVE, true);
  ShowMessages = IniReadBool(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SHOWMESSAGES, true);
  
  IniReadString(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SAVEFILE,
    CONFIG_DEFAULT_SAVEFILE, SaveFileName, STRING_SIZE);
  IniReadString(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SAVENAME,
    CONFIG_DEFAULT_SAVENAME, SaveDisplayName, STRING_SIZE);
}

internal BOOL Initialize()
{
  HookFirstLoading();
  HookReloading();
  HookMainLoop();
  
  SettingsInitialize();
  
  QueueInitialize(&BatchQueue);
  QueueInitialize(&InteriorPendingQueue);
  QueueInitialize(&ExteriorPendingQueue);
  
  QueueHandle = CreateThread(0, 0, &QueueHandler, 0, 0, &QueueThreadID);
  
  if( !Initilize() ) {
    MessageBox(0, "Batch files could not be located!", "Error", MB_OK | MB_ICONERROR);
  }
  
  return TRUE;
}

internal BOOL WINAPI DllMain(HMODULE instance, DWORD reason, LPVOID reserved)
{
  BOOL status = TRUE;

  if(reason == DLL_PROCESS_ATTACH) {
    mwsilver = instance;
    
    status = Initialize();
  }

  return status;
}
