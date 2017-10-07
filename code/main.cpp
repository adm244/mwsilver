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

//IMPORTANT(adm244): CODE'S GETTING MESSY, CLEAN IT!!

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
    - Change random algorithm a bit
    - Set new seed every N minutes
    - Message with progress bar (on game load and game save)
    - Remove duplications (regenerate value if result equls to previous one)
    
    - Autosave location change
  TODO:
    - Print ingame batch description taken from file (first line?) when random command is activated
    - @teleport: modify random to be more uniform
    - Change random algorithm for @teleport command (normal distibution?)
    - @teleport: re-roll if to close to player cell?
    - @teleport_solstheim: add new command to teleport within solstheim
    - Save queue at game save and restore it later
    
    - Move config code into separate file
    - Move random code into random.c
    - Implement a set structure to replace current duplicates removal algorithm
    - Replace standard library file io with common/fileio
    - Add dynamic memory allocation support through common/memlib
    - Rewrite batch file parsing code (get rid of streaming, read entire file and parse it)
    - Move all platform-dependent code into separate file
    - Move game independent stuff into statically linked library?
    - Automate hooking proccess (either patch on the call, or something more complicated)
    - Get rid of c++ string and stdio.h
    - Get rid of C Runtime Library
    - Code cleanup
  DROPPED:
    - Play sound at command activation (use PlaySound in your batch files)
    - Test random algorithm and decide if it should be changed (no need to change prng)
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
#define CONFIG_TIMER "iTimeout"
#define CONFIG_AUTOSAVE "bAutoSave"

#define CONFIG_DEFAULT_SAVEFILE "mwsilver_save"
#define CONFIG_DEFAULT_SAVENAME "MWSilver Save"
#define CONFIG_DEFAULT_MESSAGE "%s activated"
#define CONFIG_DEFAULT_MESSAGE_RANDOM "%s activated"
#define CONFIG_DEFAULT_MESSAGE_TOGGLE_ON "Commands are ON"
#define CONFIG_DEFAULT_MESSAGE_TOGGLE_OFF "Commands are OFF"
#define CONFIG_DEFAULT_TIMER (15 * 60 * 1000)

#define AUTOSAVE_DISPLAY "AutoSave"
#define AUTOSAVE_FILENAME "autosave"

internal HMODULE mwsilver = 0;
internal HANDLE TimerQueue = 0;
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
internal bool AutoSaveEnabled = true;

internal uint8 IsTimedOut = 0;
internal uint Timeout = 0;

internal int randomGenerated = 0;
internal uint8 randomCounters[MAX_BATCHES];

internal void RandomClearCounters()
{
  randomGenerated = 0;
  for( int i = 0; i < MAX_BATCHES; ++i ) {
    randomCounters[i] = 0;
  }
}

internal int GetNextBatchIndex(int batchesCount)
{
  if( randomGenerated >= batchesCount ) {
    RandomClearCounters();
  }
  
  int value;
  for( ;; ) {
    value = RandomInt(0, batchesCount - 1);
    if( randomCounters[value] == 0 ) break;
  }
  
  ++randomGenerated;
  randomCounters[value] = 1;
  
  return value;
}

internal void RandomGeneratorInitialize(int batchesCount)
{
  int ticksPassed = GetTickCount();
  
  int ij = ticksPassed % 31328;
  int kj = ticksPassed % 30081;
  
  RandomInitialize(ij, kj);
  RandomClearCounters();
}

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

internal inline void AutoSave()
{
  if( AutoSaveEnabled ) {
    SaveGame(AUTOSAVE_DISPLAY, AUTOSAVE_FILENAME);
  }
}

internal inline void MakePreSave()
{
  if( SavePreActivation ) {
    SaveGame("PreActivation", "pre");
  }
}

internal inline void MakePostSave()
{
  if( SavePostActivation ) {
    SaveGame("PostActivation", "post");
  }
}

internal VOID CALLBACK TimerQueueCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
  //RandomGeneratorInitialize();
  RandomClearCounters();
  
  IsTimedOut = 1;
}

internal void TimerCreate(HANDLE timerQueue, uint timeout)
{
  HANDLE newTimerQueue;
  if( !CreateTimerQueueTimer(&newTimerQueue, TimerQueue,
                            (WAITORTIMERCALLBACK)TimerQueueCallback,
                            0, timeout, 0, 0) ) {
    DisplayMessage("Ooops... Timer is dead :'(");
    DisplayMessage("We're sad pandas now :(");
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
        //int index = RandomInt(0, batches_count - 1);
        int index = GetNextBatchIndex(batches_count);
        QueuePut(&BatchQueue, (pointer)&batches[index]);
        DisplayRandomSuccessMessage(batches[index].filename);
      }
    }
  }
}

internal void GameLoop()
{
  if( IsTimedOut ) {
    IsTimedOut = 0;
    TimerCreate(TimerQueue, Timeout);
  }
  
  if( ActualGameplay ) {
    if( IsInterior != IsPlayerInInterior() ) {
      IsInterior = !IsInterior;
      
      if( IsInterior ) {
        ProcessQueue(&InteriorPendingQueue, false);
      } else {
        ProcessQueue(&ExteriorPendingQueue, false);
      }
      
      AutoSave();
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
  
  if( TimerQueue ) {
    TimerCreate(TimerQueue, Timeout);
  }
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
  AutoSaveEnabled = IniReadBool(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_AUTOSAVE, true);
  
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
  
  Timeout = IniReadInt(CONFIG_FILE, CONFIG_SETTINGS_SECTION, CONFIG_TIMER, CONFIG_DEFAULT_TIMER);
}

internal BOOL Initialize()
{
  HookFirstLoading();
  HookReloading();
  HookMainLoop();
  
  SettingsInitialize();
  
  int batchesCount = InitilizeBatches();
  if( batchesCount <= 0 ) {
    MessageBox(0, "Batch files could not be located!", "Error", MB_OK | MB_ICONERROR);
  }
  
  QueueInitialize(&BatchQueue);
  QueueInitialize(&InteriorPendingQueue);
  QueueInitialize(&ExteriorPendingQueue);
  QueueHandle = CreateThread(0, 0, &QueueHandler, 0, 0, &QueueThreadID);
  
  RandomGeneratorInitialize(batchesCount);
  TimerQueue = CreateTimerQueue();
  InitializeGrid();
  
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
