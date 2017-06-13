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

#define CONFIG_FILE "mwsilver.ini"
#define MAX_SECTION 32767
#define MAX_FILENAME 260
#define MAX_BATCHES 50

struct BatchData{
  char filename[MAX_FILENAME];
  int key;
  bool enabled;
};

struct CustomCommand{
  int key;
  bool enabled;
};

internal BatchData batches[MAX_BATCHES];
internal CustomCommand CommandToggle;

internal bool keys_active = true;
internal bool not_initialized = true;
internal int batches_count = 0;

//NOTE(adm244): overloaded version for CustomCommand structure
bool IsActivated(CustomCommand *cmd)
{
  return(IsActivated(cmd->key, &cmd->enabled));
}

//NOTE(adm244): loads a list of batch files and keys that activate them
// filename and associated keycode stored in a BatchData structure pointed to by 'batches'
//
// returns true if at least 1 bat file loaded successefully
// returns false if no bat files were loaded
bool InitBatchFiles(BatchData *batches, int *num)
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

internal bool ExecuteBatch(char *filename)
{
  bool result = false;

  FILE *src = NULL;
  fopen_s(&src, filename, "r");
  
  if( src ){
    char line[4096];
    
    while( fgets(line, sizeof(line), src) ){
      uint32 lineLen = strlen(line);
      
      if( lineLen > 1 ){
        if(line[lineLen - 1] == '\n'){
          line[lineLen - 1] = 0;
        }
        
        result = ExecuteScript(line);
        if( !result ){
          break;
        }
      }
    }

    fclose(src);
  }

  return result;
}

//NOTE(adm244): initializes plugin variables
// returns true if atleast 1 batch file wes successefully loaded
// returns false otherwise
internal bool Initilize()
{
  keys_active = true;
  InitBatchFiles(batches, &batches_count);
  
  CommandToggle.key = IniReadInt(CONFIG_FILE, "keys", "iKeyToggle", VK_HOME);
  CommandToggle.enabled = true;
  
  return(batches_count > 0);
}

#endif
