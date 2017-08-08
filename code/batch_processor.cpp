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

#define MAX_SECTION 32767
#define MAX_FILENAME 260
#define MAX_BATCHES 50

#define BATCH_DEFAULT "@default"
#define BATCH_INTERIOR "@interior"
#define BATCH_EXTERIOR "@exterior"
#define BATCH_INTERIOR_ONLY "@interioronly"
#define BATCH_EXTERIOR_ONLY "@exterioronly"
#define BATCH_SAVEGAME "@savegame"
#define BATCH_TELEPORT "@teleport"

#define EXEC_DEFAULT 0
#define EXEC_INTERIOR 1
#define EXEC_EXTERIOR 2
#define EXEC_INTERIOR_ONLY 3
#define EXEC_EXTERIOR_ONLY 4

#define CELL_X_START -14
#define CELL_X_END 20
#define CELL_X_COUNT 35

struct GridPair {
  int min;
  int max;
};

struct BatchData{
  char filename[MAX_FILENAME];
  int key;
  bool enabled;
};

struct CustomCommand{
  int key;
  bool enabled;
};

internal GridPair grid[CELL_X_COUNT];
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

internal void Teleport()
{
  int cell_x = RandomInt(CELL_X_START, CELL_X_END);

  int index = cell_x + Absolute(CELL_X_START);
  int cell_y = RandomInt(grid[index].min, grid[index].max);
  
  char buffer[MW_SCRIPT_LINE];
  sprintf(buffer, "coe, %d, %d", cell_x, cell_y);
  ExecuteScript(buffer);
}

internal void InitializeGrid()
{
  //grid[0].min = 11; grid[0].max = 15; // -17
  //grid[1].min = 10; grid[1].max = 16; // -16
  //grid[2].min = 10; grid[2].max = 16; // -15
  grid[0].min = 8;    grid[0].max = 16; // -14
  grid[1].min = 7;    grid[1].max = 17; // -13
  grid[2].min = 6;    grid[2].max = 18; // -12
  grid[3].min = 3;    grid[3].max = 19; // -11
  grid[4].min = -4;   grid[4].max = 19; // -10
  grid[5].min = -6;   grid[5].max = 19; // -9
  grid[6].min = -7;   grid[6].max = 19; // -8
  grid[7].min = -8;   grid[7].max = 21; // -7
  grid[8].min = -10;  grid[8].max = 23; // -6
  grid[9].min = -10;  grid[9].max = 23; // -5
  grid[10].min = -11; grid[10].max = 23; // -4
  grid[11].min = -11; grid[11].max = 23; // -3
  grid[12].min = -11; grid[12].max = 23; // -2
  grid[13].min = -12; grid[13].max = 23; // -1
  grid[14].min = -15; grid[14].max = 23; // 0
  grid[15].min = -15; grid[15].max = 23; // 1
  grid[16].min = -15; grid[16].max = 23; // 2
  grid[17].min = -15; grid[17].max = 23; // 3
  grid[18].min = -15; grid[18].max = 23; // 4
  grid[19].min = -14; grid[19].max = 23; // 5
  grid[20].min = -15; grid[20].max = 23; // 6
  grid[21].min = -15; grid[21].max = 23; // 7
  grid[22].min = -15; grid[22].max = 23; // 8
  grid[23].min = -15; grid[23].max = 23; // 9
  grid[24].min = -14; grid[24].max = 23; // 10
  grid[25].min = -14; grid[25].max = 22; // 11
  grid[26].min = -14; grid[26].max = 22; // 12
  grid[27].min = -15; grid[27].max = 21; // 13
  grid[28].min = -15; grid[28].max = 18; // 14
  grid[29].min = -15; grid[29].max = 17; // 15
  grid[30].min = -15; grid[30].max = 13; // 16
  grid[31].min = -15; grid[31].max = 12; // 17
  grid[32].min = -11; grid[32].max = 11; // 18
  grid[33].min = -11; grid[33].max = 10; // 19
  grid[34].min = -11; grid[34].max = 8; // 20
  //grid[38].min = -11; grid[38].max = 8; // 21
  //grid[39].min = -10; grid[39].max = 5; // 22
}

//NOTE(adm244): loads a list of batch files and keys that activate them
// filename and associated keycode stored in a BatchData structure pointed to by 'batches'
//
// returns true if at least 1 bat file loaded successefully
// returns false if no bat files were loaded
internal bool InitBatchFiles(BatchData *batches, int *num)
{
  char buf[MAX_SECTION];
  char *str = buf;
  int index = 0;
  
  IniReadSection(CONFIG_FILE, "batch", buf, MAX_SECTION);
  
  while( true ){
    char *p = strrchr(str, '=');
    
    if( p && (index < MAX_BATCHES) ){
      char *endptr;
      *p++ = '\0';
      
      strcpy(batches[index].filename, str);
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

//IMPORTANT(adm244): get rid off duplicated file reading!
internal uint8 GetBatchExecState(char *filename)
{
  uint8 result = EXEC_DEFAULT;

  FILE *src = NULL;
  fopen_s(&src, filename, "r");
  
  if( src ){
    char line[4096];
    
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
        SaveGame(SaveDisplayName, SaveFileName);
      } else if( strcmp(line, BATCH_TELEPORT) == 0 ) {
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
internal bool InitilizeBatches()
{
  keys_active = true;
  InitBatchFiles(batches, &batches_count);
  
  CommandToggle.key = IniReadInt(CONFIG_FILE, CONFIG_KEYS_SECTION, "iKeyToggle", VK_HOME);
  CommandToggle.enabled = true;
  
  CommandRandom.key = IniReadInt(CONFIG_FILE, CONFIG_KEYS_SECTION, "iKeyRandomBatch", VK_ADD);
  CommandRandom.enabled = true;
  
  return(batches_count > 0);
}

#endif
