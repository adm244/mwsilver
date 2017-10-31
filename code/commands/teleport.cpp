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

#ifndef TELEPORT_CPP
#define TELEPORT_CPP

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

internal GridPair mainGrid[MAIN_GRID_CELL_X_COUNT];
internal GridPair solstheimGrid[SOLS_GRID_CELL_X_COUNT];

internal bool temp_teleportActive = false;

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

#endif
