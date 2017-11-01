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

#ifndef BATCH_PROCESSOR_H
#define BATCH_PROCESSOR_H

#include "commands/teleport.cpp"

#define BATCH_DEFAULT "@default"
#define BATCH_INTERIOR "@interior"
#define BATCH_EXTERIOR "@exterior"
#define BATCH_INTERIOR_ONLY "@interioronly"
#define BATCH_EXTERIOR_ONLY "@exterioronly"

//TODO(adm244): move outside
#define BATCH_SAVEGAME "@savegame"
#define BATCH_TELEPORT "@teleport"

#define EXEC_DEFAULT 0
#define EXEC_INTERIOR 1
#define EXEC_EXTERIOR 2
#define EXEC_INTERIOR_ONLY 3
#define EXEC_EXTERIOR_ONLY 4

struct BatchData{
  char filename[MAX_FILENAME];
  char description[MAX_DESCRIPTION];
  int key;
  bool enabled;
};

struct CustomCommand{
  int key;
  bool enabled;
};

internal BatchData batches[MAX_BATCHES];
internal CustomCommand CommandToggle;
internal CustomCommand CommandRandom;

internal bool keys_active = true;
internal bool not_initialized = true;
internal int batches_count = 0;

//NOTE(adm244): overloaded version for CustomCommand structure
internal bool IsActivated(CustomCommand *cmd)
{
  return(IsActivated(cmd->key, &cmd->enabled));
}

//NOTE(adm244): loads a list of batch files and keys that activate them
// filename and associated keycode stored in a BatchData structure pointed to by 'batches'
//
// returns true if at least 1 bat file loaded successefully
// returns false if no bat files were loaded
internal bool InitBatchFiles(HMODULE module, BatchData *batches, int *num)
{
  char buf[MAX_SECTION];
  char *str = buf;
  int index = 0;
  
  IniReadSection(module, CONFIG_FILE, "batch", buf, MAX_SECTION);
  
  while( true ){
    char *p = strrchr(str, '=');
    
    if( p && (index < MAX_BATCHES) ){
      char *endptr;
      *p++ = '\0';
      
      strcpy(batches[index].filename, str);
      strcpy(batches[index].description, str);
      batches[index].key = (int)strtol(p, &endptr, 0);
      batches[index].enabled = true;
      
      str = strchr(p, '\0');
      str++;
      
      index++;
    } else{
      break;
    }
  }
  
  *num = index;
  return(index > 0);
}

internal void ReadBatchDescriptions(BatchData *batches)
{
  FILE *file = NULL;
  char line[MAX_DESCRIPTION];
  
  for( int i = 0; i < batches_count; ++i ) {
    fopen_s(&file, batches[i].filename, "r");
    
    if( file ) {
      if( fgets(line, sizeof(line), file) ) {
        strcpy(batches[i].description, line);
      }
    }
  }
}

//IMPORTANT(adm244): get rid off duplicated file reading!
internal uint8 GetBatchExecState(char *filename)
{
  uint8 result = EXEC_DEFAULT;

  FILE *src = NULL;
  fopen_s(&src, filename, "r");
  
  if( src ){
    char line[4096];
    
    //FIX(adm244): hack
    fgets(line, sizeof(line), src);
    
    while( fgets(line, sizeof(line), src) ){
      uint32 lineLen = strlen(line);
      
      if(line[lineLen - 1] == '\n'){
        line[lineLen - 1] = 0;
      }
    
      if( strcmp(line, BATCH_EXTERIOR_ONLY) == 0 ) {
        result = EXEC_EXTERIOR_ONLY;
        break;
      } else if( strcmp(line, BATCH_INTERIOR_ONLY) == 0 ) {
        result = EXEC_INTERIOR_ONLY;
        break;
      }
    }

    fclose(src);
  }

  return result;
}

internal bool ExecuteBatch(char *filename)
{
  bool result = false;
  
  FILE *src = NULL;
  fopen_s(&src, filename, "r");

  if( src ){
    char line[4096];
    uint8 executionState = EXEC_DEFAULT;
    
    //FIX(adm244): hack
    fgets(line, sizeof(line), src);
    
    while( fgets(line, sizeof(line), src) ){
      uint32 lineLen = strlen(line);
      
      if(line[lineLen - 1] == '\n'){
        line[lineLen - 1] = 0;
      }
      
      //FIX(adm244): @interioronly and @exterioronly are executed by morrowind
      
      if( strcmp(line, BATCH_EXTERIOR) == 0 ) {
        executionState = EXEC_EXTERIOR;
      } else if( strcmp(line, BATCH_INTERIOR) == 0 ) {
        executionState = EXEC_INTERIOR;
      } else if( strcmp(line, BATCH_DEFAULT) == 0 ) {
        executionState = EXEC_DEFAULT;
      } else if( strcmp(line, BATCH_SAVEGAME) == 0 ) {
        SaveGame(Strings.SaveDisplayName, Strings.SaveFileName);
      } else if( strcmp(line, BATCH_TELEPORT) == 0 ) {
        //TODO(adm244): rewrite so it could use custom commands (search by table?)
        Teleport();
      } else {
        if( (IsInterior && (executionState == EXEC_INTERIOR))
         || (!IsInterior && (executionState == EXEC_EXTERIOR))
         || (executionState == EXEC_DEFAULT) ) {
          result = ExecuteScript(line);
          if( !result ) {
            break;
          }
        }
      }
    }
    
    fclose(src);
  }

  return result;
}

//NOTE(adm244): initializes plugin variables
// returns true if atleast 1 batch file was successefully loaded
// returns false otherwise
internal int InitilizeBatches(HMODULE module)
{
  keys_active = true;
  InitBatchFiles(module, batches, &batches_count);
  ReadBatchDescriptions(batches);
  
  //TODO(adm244): rewrite IniRead* functions so they accept full path to config file folder
  CommandToggle.key = IniReadInt(module, CONFIG_FILE, CONFIG_KEYS_SECTION, "iKeyToggle", VK_HOME);
  CommandToggle.enabled = true;
  
  CommandRandom.key = IniReadInt(module, CONFIG_FILE, CONFIG_KEYS_SECTION, "iKeyRandomBatch", VK_ADD);
  CommandRandom.enabled = true;
  
  return(batches_count);
}

#endif
