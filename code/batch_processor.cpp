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
#define MAX_DESCRIPTION 255
//#define MAX_BATCHES 50

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

#define MAIN_GRID_CELL_X_START -14
#define MAIN_GRID_CELL_X_END 20
#define MAIN_GRID_CELL_X_COUNT 35

#define SOLS_GRID_CELL_X_START -27
#define SOLS_GRID_CELL_X_END -17
#define SOLS_GRID_CELL_X_COUNT 11

struct GridPair {
  int min;
  int max;
};

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

internal GridPair mainGrid[MAIN_GRID_CELL_X_COUNT];
internal GridPair solstheimGrid[SOLS_GRID_CELL_X_COUNT];

internal BatchData batches[MAX_BATCHES];
internal CustomCommand CommandToggle;
internal CustomCommand CommandRandom;

internal bool keys_active = true;
internal bool not_initialized = true;
internal int batches_count = 0;

internal bool temp_teleportActive = false;

//NOTE(adm244): overloaded version for CustomCommand structure
internal bool IsActivated(CustomCommand *cmd)
{
  return(IsActivated(cmd->key, &cmd->enabled));
}

struct TeleportData {
  double mean;
  double deviation;
};

struct TeleportRegion {
  TeleportData x;
  TeleportData y;
};

internal bool IsOnSolstheim(TESCell *cell)
{
  bool result = false;

  if( IsCellInterior(cell) ) {
    //TODO(adm244): get list of solstheim interiors and check against them
    
    if( strstr(cell->name, "Solstheim") ) {
      result = true;
    } else if( strstr(cell->name, "Raven Rock") ) {
      result = true;
    } else if( strstr(cell->name, "Fort Frostmoth") ) {
      result = true;
    } else if( strstr(cell->name, "Skaal") ) {
      result = true;
    } else if( strstr(cell->name, "Thirsk") ) {
      result = true;
    } else if( strstr(cell->name, "Солстхейм") ) {
      result = true;
    }
  } else {
    //FIX(adm244): move region id into string array and compare in a loop
    //TODO(adm244): get list of solstheim regions and check agains them
    
    if( !strncmp(cell->region->id, "Hirstaang Forest Region", REGN_NAME_LENGTH) ) {
      result = true;
    } else if( !strncmp(cell->region->id, "Brodir Grove Region", REGN_NAME_LENGTH) ) {
      result = true;
    } else if( !strncmp(cell->region->id, "Isinfier Plains Region", REGN_NAME_LENGTH) ) {
      result = true;
    } else if( !strncmp(cell->region->id, "Moesring Mountains Region", REGN_NAME_LENGTH) ) {
      result = true;
    } else if( !strncmp(cell->region->id, "Felsaad Coast Region", REGN_NAME_LENGTH) ) {
      result = true;
    } else if( !strncmp(cell->region->id, "Thirsk Region", REGN_NAME_LENGTH) ) {
      result = true;
    }
  }
  
  return result;
}

internal TeleportRegion GetRandomTeleportRegion()
{
  TeleportRegion region;
  
  //FIX(adm244): temporary solution!
  //FIX(adm244): move these into TeleportRegion array
  int rnd = RandomInt(0, 7);
  switch( rnd ) {
    case 0: {
      region.x.mean = 4;
      region.x.deviation = 4;
      region.y.mean = 21;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 1: {
      region.x.mean = -8;
      region.x.deviation = 5;
      region.y.mean = 11;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 2: {
      region.x.mean = 10;
      region.x.deviation = 5;
      region.y.mean = 10;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 3: {
      region.x.mean = 15;
      region.x.deviation = 4;
      region.y.mean = 2;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 4: {
      region.x.mean = 13;
      region.x.deviation = 6;
      region.y.mean = -6;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 5: {
      region.x.mean = 3;
      region.x.deviation = 4;
      region.y.mean = -9;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 6: {
      region.x.mean = -2;
      region.x.deviation = 5;
      region.y.mean = 2;
      region.y.deviation = region.x.deviation;
    } break;
    
    case 7: {
      region.x.mean = 6;
      region.x.deviation = 4;
      region.y.mean = 7;
      region.y.deviation = region.x.deviation;
    } break;
  }
  
  return region;
}

internal void GetRandomSolstheimCell(int *cellX, int *cellY)
{
  //TODO(adm244): change to normal distribution
  int x = RandomInt(SOLS_GRID_CELL_X_START, SOLS_GRID_CELL_X_END);
  
  int index = x + Absolute(SOLS_GRID_CELL_X_START);
  int minY = solstheimGrid[index].min;
  int maxY = solstheimGrid[index].max;
  int y = RandomInt(minY, maxY);
  
  *cellX = x;
  *cellY = y;
}

internal void GetRandomMainLandCell(int *cellX, int *cellY)
{
  double mean;
  double deviation;
  
  TeleportRegion region = GetRandomTeleportRegion();
  
  int x = (int)RandomGaussian(region.x.mean, region.x.deviation);
  x = Clamp(x, MAIN_GRID_CELL_X_START, MAIN_GRID_CELL_X_END);
  
  int index = x + Absolute(MAIN_GRID_CELL_X_START);
  mean = (mainGrid[index].min + mainGrid[index].max) / 2;
  deviation = Absolute(mainGrid[index].max - mean) / 3;
  
  int y = (int)RandomGaussian(region.y.mean, region.y.deviation);
  y = Clamp(y, mainGrid[index].min, mainGrid[index].max);
  
  *cellX = x;
  *cellY = y;
}

internal void Teleport()
{
  temp_teleportActive = true;
  
  int cell_x;
  int cell_y;
  
  TESCell *currentCell = GetCurrentCell(GetTESObject());
  if( IsOnSolstheim(currentCell) ) {
    GetRandomSolstheimCell(&cell_x, &cell_y);
  } else {
    GetRandomMainLandCell(&cell_x, &cell_y);
  }
  
  char buffer[MW_SCRIPT_LINE];
  sprintf(buffer, "coe, %d, %d", cell_x, cell_y);
  ExecuteScript(buffer);
}

internal void InitializeMainGrid()
{
  mainGrid[0].min = 8;    mainGrid[0].max = 16; // -14
  mainGrid[1].min = 7;    mainGrid[1].max = 17; // -13
  mainGrid[2].min = 6;    mainGrid[2].max = 18; // -12
  mainGrid[3].min = 3;    mainGrid[3].max = 19; // -11
  mainGrid[4].min = -4;   mainGrid[4].max = 19; // -10
  mainGrid[5].min = -6;   mainGrid[5].max = 19; // -9
  mainGrid[6].min = -7;   mainGrid[6].max = 19; // -8
  mainGrid[7].min = -8;   mainGrid[7].max = 21; // -7
  mainGrid[8].min = -10;  mainGrid[8].max = 23; // -6
  mainGrid[9].min = -10;  mainGrid[9].max = 23; // -5
  mainGrid[10].min = -11; mainGrid[10].max = 23; // -4
  mainGrid[11].min = -11; mainGrid[11].max = 23; // -3
  mainGrid[12].min = -11; mainGrid[12].max = 23; // -2
  mainGrid[13].min = -12; mainGrid[13].max = 23; // -1
  mainGrid[14].min = -15; mainGrid[14].max = 23; // 0
  mainGrid[15].min = -15; mainGrid[15].max = 23; // 1
  mainGrid[16].min = -15; mainGrid[16].max = 23; // 2
  mainGrid[17].min = -15; mainGrid[17].max = 23; // 3
  mainGrid[18].min = -15; mainGrid[18].max = 23; // 4
  mainGrid[19].min = -14; mainGrid[19].max = 23; // 5
  mainGrid[20].min = -15; mainGrid[20].max = 23; // 6
  mainGrid[21].min = -15; mainGrid[21].max = 23; // 7
  mainGrid[22].min = -15; mainGrid[22].max = 23; // 8
  mainGrid[23].min = -15; mainGrid[23].max = 23; // 9
  mainGrid[24].min = -14; mainGrid[24].max = 23; // 10
  mainGrid[25].min = -14; mainGrid[25].max = 22; // 11
  mainGrid[26].min = -14; mainGrid[26].max = 22; // 12
  mainGrid[27].min = -15; mainGrid[27].max = 21; // 13
  mainGrid[28].min = -15; mainGrid[28].max = 18; // 14
  mainGrid[29].min = -15; mainGrid[29].max = 17; // 15
  mainGrid[30].min = -15; mainGrid[30].max = 13; // 16
  mainGrid[31].min = -15; mainGrid[31].max = 12; // 17
  mainGrid[32].min = -11; mainGrid[32].max = 11; // 18
  mainGrid[33].min = -11; mainGrid[33].max = 10; // 19
  mainGrid[34].min = -11; mainGrid[34].max = 8; // 20
}

internal void InitializeSolstheimGrid()
{
  solstheimGrid[0].min = 19; solstheimGrid[0].max = 27; // -27
  solstheimGrid[1].min = 17; solstheimGrid[1].max = 27; // -26
  solstheimGrid[2].min = 16; solstheimGrid[2].max = 27; // -25
  solstheimGrid[3].min = 15; solstheimGrid[3].max = 27; // -24
  solstheimGrid[4].min = 15; solstheimGrid[4].max = 27; // -23
  solstheimGrid[5].min = 16; solstheimGrid[5].max = 27; // -22
  solstheimGrid[6].min = 17; solstheimGrid[6].max = 27; // -21
  solstheimGrid[7].min = 17; solstheimGrid[7].max = 27; // -20
  solstheimGrid[8].min = 17; solstheimGrid[8].max = 26; // -19
  solstheimGrid[9].min = 17; solstheimGrid[9].max = 26; // -18
  solstheimGrid[10].min = 24; solstheimGrid[10].max = 25; // -17
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
