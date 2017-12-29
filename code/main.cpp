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
    - Print ingame batch description taken from file (first line?) when random command is activated
    - Change random algorithm for @teleport command (normal distibution?)
    - Disable autosave at teleportation
    - @teleport: determine the current island and teleport within it only
    - Move config code into separate file
    - Move random code into separate file
    - Move teleport code into separate file
    - Create structure to store all config related variables
  TODO:
    - Get rid of globals
    - Move all platform-dependent code into separate file
    - Replace standard library file io with common/fileio
    - Move game independent stuff into statically linked library?
    
    - Rewrite batch file parsing code (get rid of streaming, read entire file and parse it)
    - Implement a set structure to replace current duplicates removal algorithm
    - Get rid of c++ string and stdio.h
    
    - Load batches from a "batches" folder
    - Save queue at game save and restore it later
    
    - Add dynamic memory allocation support through common/memlib
    - Automate hooking proccess (either patch on the call, or something more complicated)
    - Get rid of C Runtime Library
    - Code cleanup
  DROPPED:
    - Play sound at command activation (use PlaySound in your batch files)
    - Test random algorithm and decide if it should be changed (no need to change prng)
    - @teleport: modify random to be more uniform (switched to normal distribution)
    - @teleport_solstheim: add new command to teleport within solstheim (checking which island player is on)
    - Delete interactive save after it's loading (useless)
    - @teleport: re-roll if to close to player cell? (nah)
*/

#include <stdio.h>
#include <string>
#include <windows.h>

#include "common/types.h"
#include "common/utils.cpp"
#include "common/queue.cpp"

//FIX(adm244): hack!
#define MAX_SECTION 32767
#define MAX_FILENAME 260
#define MAX_DESCRIPTION 255
#define MAX_BATCHES 50

#include "random/functions.cpp"

#define AUTOSAVE_DISPLAY "AutoSave"
#define AUTOSAVE_FILENAME "autosave"

internal HMODULE mwsilver = 0;
internal HANDLE TimerQueue = 0;

internal bool IsInterior = false;
internal bool ActualGameplay = false;

#include "config.cpp"
#include "mw/functions.cpp"
#include "batch_processor.cpp"
#include "hooks.cpp"

internal HANDLE QueueHandle = 0;
internal DWORD QueueThreadID = 0;

internal Queue BatchQueue;
internal Queue InteriorPendingQueue;
internal Queue ExteriorPendingQueue;

internal uint8 IsTimedOut = 0;

internal inline void AutoSave()
{
  if( Settings.AutoSaveEnabled ) {
    SaveGame(AUTOSAVE_DISPLAY, AUTOSAVE_FILENAME);
  }
}

internal inline void MakePreSave()
{
  if( Settings.SavePreActivation ) {
    SaveGame("PreActivation", "pre");
  }
}

internal inline void MakePostSave()
{
  if( Settings.SavePostActivation ) {
    SaveGame("PostActivation", "post");
  }
}

internal VOID CALLBACK TimerQueueCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
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
    DisplaySuccessMessage(batch->description);
    
    if( isQueueEmpty ) MakePostSave();
  }
}

internal DWORD WINAPI QueueHandler(LPVOID data)
{
  for(;;) {
    if( IsActivated(&CommandToggle) ) {
      keys_active = !keys_active;
      
      //TODO(adm244): display it somehow on loading screen
      if( ActualGameplay ) {
        DisplayMessage(keys_active ? Strings.MessageOn : Strings.MessageOff);
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
        DisplayRandomSuccessMessage(batches[index].description);
      }
    }
  }
}

internal void GameLoop()
{
  if( IsTimedOut ) {
    IsTimedOut = 0;
    TimerCreate(TimerQueue, Settings.Timeout);
  }
  
  if( ActualGameplay ) {
    if( IsInterior != IsPlayerInInterior() ) {
      IsInterior = !IsInterior;
      
      if( IsInterior ) {
        ProcessQueue(&InteriorPendingQueue, false);
      } else {
        ProcessQueue(&ExteriorPendingQueue, false);
      }
      
      if( !temp_teleportActive ) {
        AutoSave();
      }
    }
    temp_teleportActive = false;
  
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
    TimerCreate(TimerQueue, Settings.Timeout);
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

internal BOOL Initialize()
{
  HookFirstLoading();
  HookReloading();
  HookMainLoop();
  
  SettingsInitialize(mwsilver);
  
  int batchesCount = InitilizeBatches(mwsilver);
  if( batchesCount <= 0 ) {
    MessageBox(0, "Batch files could not be located!", "Error", MB_OK | MB_ICONERROR);
  }
  
  QueueInitialize(&BatchQueue);
  QueueInitialize(&InteriorPendingQueue);
  QueueInitialize(&ExteriorPendingQueue);
  QueueHandle = CreateThread(0, 0, &QueueHandler, 0, 0, &QueueThreadID);
  
  RandomGeneratorInitialize(batchesCount);
  TimerQueue = CreateTimerQueue();
  
  InitializeMainGrid();
  InitializeSolstheimGrid();
  
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
