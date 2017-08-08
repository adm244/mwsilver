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
    - Command to activate random batch file
    
    - Get message strings from config file
  TODO:
    - Move game independent stuff into statically linked library?
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
#define CONFIG_MESSAGE_SECTION "message"
#define CONFIG_KEYS_SECTION "keys"

#define CONFIG_PRESAVE "bSaveGamePreActivation"
#define CONFIG_POSTSAVE "bSaveGamePostActivation"
#define CONFIG_SHOWMESSAGES "bShowMessages"
#define CONFIG_SHOWMESSAGES_RANDOM "bShowMessagesRandom"
#define CONFIG_SAVEFILE "sSaveFile"
#define CONFIG_SAVENAME "sSaveName"
#define CONFIG_MESSAGE "sMessage"
#define CONFIG_MESSAGE_RANDOM "sMessageRandom"
#define CONFIG_MESSAGE_TOGGLE_ON "sMessageToggleOn"
#define CONFIG_MESSAGE_TOGGLE_OFF "sMessageToggleOff"

#define CONFIG_DEFAULT_SAVEFILE "mwsilver_save"
#define CONFIG_DEFAULT_SAVENAME "MWSilver Save"
#define CONFIG_DEFAULT_MESSAGE "%s activated"
#define CONFIG_DEFAULT_MESSAGE_RANDOM "%s activated"
#define CONFIG_DEFAULT_MESSAGE_TOGGLE_ON "Commands are ON"
#define CONFIG_DEFAULT_MESSAGE_TOGGLE_OFF "Commands are OFF"

#define CELL_X_START -17
#define CELL_X_END 22
#define CELL_X_COUNT 40

struct GridPair {
  int min;
  int max;
};

internal GridPair grid[CELL_X_COUNT];

internal HMODULE mwsilver = 0;
internal uint8 IsInterior = 0;

#define STRING_SIZE 256

internal char SaveFileName[STRING_SIZE];
internal char SaveDisplayName[STRING_SIZE];
internal char Message[STRING_SIZE];
internal char MessageRandom[STRING_SIZE];
internal char MessageOn[STRING_SIZE];
internal char MessageOff[STRING_SIZE];

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
internal bool ShowMessagesRandom = true;
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
  ShowGameMessage(message, 0, 1);
}

internal void DisplaySuccessMessage(char *batchName)
{
  char statusString[260];
  sprintf(statusString, Message, batchName);
  
  ConsolePrint(globalStatePointer, statusString);
  
  if( ShowMessages ) {
    DisplayMessage(statusString);
  }
}

internal void DisplayRandomSuccessMessage(char *batchName)
{
  char statusString[260];
  sprintf(statusString, MessageRandom, batchName);
  
  ConsolePrint(globalStatePointer, statusString);
  
  if( ShowMessagesRandom ) {
    DisplayMessage(statusString);
  }
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
    
    if( isQueueEmpty ) MakePreSave();
  
    ExecuteBatch(batch->filename);
    DisplaySuccessMessage(batch->filename);
    
    if( isQueueEmpty ) MakePostSave();
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
        DisplayMessage(keys_active ? MessageOn : MessageOff);
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
        
        DisplayRandomSuccessMessage(batches[index].filename);
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
    
    if ( IsActivated(&CommandTeleport) ) {
      int cell_x = RandomInt(CELL_X_START, CELL_X_END);
      
      int index = cell_x + 17;
      int cell_y = RandomInt(grid[index].min, grid[index].max);
      
      char buffer[MW_SCRIPT_LINE];
      sprintf(buffer, "coe, %d, %d", cell_x, cell_y);
      ExecuteScript(buffer);
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
  ShowMessagesRandom = IniReadBool(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SHOWMESSAGES_RANDOM, true);
  
  IniReadString(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SAVEFILE,
    CONFIG_DEFAULT_SAVEFILE, SaveFileName, STRING_SIZE);
  IniReadString(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_SAVENAME,
    CONFIG_DEFAULT_SAVENAME, SaveDisplayName, STRING_SIZE);
    
  IniReadString(CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE,
    CONFIG_DEFAULT_MESSAGE, Message, STRING_SIZE);
  IniReadString(CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE_RANDOM,
    CONFIG_DEFAULT_MESSAGE_RANDOM, MessageRandom, STRING_SIZE);
  IniReadString(CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE_TOGGLE_ON,
    CONFIG_DEFAULT_MESSAGE_TOGGLE_ON, MessageOn, STRING_SIZE);
  IniReadString(CONFIG_FILE, CONFIG_MESSAGE_SECTION, CONFIG_MESSAGE_TOGGLE_OFF,
    CONFIG_DEFAULT_MESSAGE_TOGGLE_OFF, MessageOff, STRING_SIZE);
}

internal void RandomGeneratorInitialize()
{
  int ticksPassed = GetTickCount();
  
  int ij = ticksPassed % 31328;
  int kj = ticksPassed % 30081;
  
  RandomInitialize(ij, kj);
}

internal void InitializeGrid()
{
  grid[0].min = 11; grid[0].max = 15; // -17
  grid[1].min = 10; grid[1].max = 16; // -16
  grid[2].min = 10; grid[2].max = 16; // -15
  grid[3].min = 8; grid[3].max = 16; // -14
  grid[4].min = 7; grid[4].max = 17; // -13
  grid[5].min = 6; grid[5].max = 18; // -12
  grid[6].min = 3; grid[6].max = 19; // -11
  grid[7].min = -4; grid[7].max = 19; // -10
  grid[8].min = -6; grid[8].max = 19; // -9
  grid[9].min = -7; grid[9].max = 19; // -8
  grid[10].min = -8; grid[10].max = 21; // -7
  grid[11].min = -10; grid[11].max = 23; // -6
  grid[12].min = -10; grid[12].max = 26; // -5
  grid[13].min = -11; grid[13].max = 26; // -4
  grid[14].min = -11; grid[14].max = 26; // -3
  grid[15].min = -11; grid[15].max = 26; // -2
  grid[16].min = -12; grid[16].max = 26; // -1
  grid[17].min = -15; grid[17].max = 26; // 0
  grid[18].min = -15; grid[18].max = 26; // 1
  grid[19].min = -15; grid[19].max = 26; // 2
  grid[20].min = -15; grid[20].max = 26; // 3
  grid[21].min = -15; grid[21].max = 26; // 4
  grid[22].min = -14; grid[22].max = 26; // 5
  grid[23].min = -15; grid[23].max = 26; // 6
  grid[24].min = -15; grid[24].max = 26; // 7
  grid[25].min = -15; grid[25].max = 26; // 8
  grid[26].min = -15; grid[26].max = 25; // 9
  grid[27].min = -14; grid[27].max = 23; // 10
  grid[28].min = -14; grid[28].max = 22; // 11
  grid[29].min = -14; grid[29].max = 22; // 12
  grid[30].min = -15; grid[30].max = 21; // 13
  grid[31].min = -15; grid[31].max = 18; // 14
  grid[32].min = -16; grid[32].max = 17; // 15
  grid[33].min = -15; grid[33].max = 13; // 16
  grid[34].min = -15; grid[34].max = 12; // 17
  grid[35].min = -11; grid[35].max = 11; // 18
  grid[36].min = -11; grid[36].max = 10; // 19
  grid[37].min = -11; grid[37].max = 8; // 20
  grid[38].min = -11; grid[38].max = 8; // 21
  grid[39].min = -10; grid[39].max = 5; // 22
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
  
  RandomGeneratorInitialize();
  
  BOOL result = InitilizeBatches();
  if( !result ) {
    MessageBox(0, "Batch files could not be located!", "Error", MB_OK | MB_ICONERROR);
  }
  
  InitializeGrid();
  
  return result;
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
