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
  TODO:
    - search for "Done Initalizing Main" (main menu ?)
    
  RE:
    IsNPCEssential address: 0x004D5EE0
      [ecx + 0x34] -- flag, bit 2 - essential
    
    possible ShowProgressMessage address: 0x005DED20
    
    ShowGameMessage address: 0x005F90C0
      // message - message text
      // unk1 - ? (can be null)
      // unk2 - some value (1 as it seems)
      void ShowGameMessage(char *message, int unk1, int unk2);
  
    CompileAndRun address: 0x0050E5A0
      method of object "[0x007C67DC] + 0x54"
      
      // globalState = eax = [[[0x007C67DC] + 0x2E4] + 0x20]
      // eax = [edx+20h] - object or variable
      // edx = [ecx+2E4h] - some object
      // ecx = 0x007C67DC - object
      
      // globalScriptState - Global Script state object
      // text - Script text
      // unk2 - some value (1 or 2)
      // unk3 - some object (can be null)
      // unk4 - some object | [0x007C67E0] + 0x3C [+ 0x8] | (can be null)
      // unk5 - some object (can be null)
      // unk6 - some object (can be null)
      void CompileAndRun(uint32 globalScriptState, char *text, int unk2, uint32 unk3, uint32 unk4, int unk5, int unk6);
    
    Object 007C67DC as GlobalState:
      method GetMessageString
      
      GlobalState + 0x2C -- (float?) some value
      GlobalState + 0x34 -- some object pointer
      GlobalState + 0x4C -- some object pointer
      GlobalState + 0x54 -- Script object(?)
      GlobalState + 0x50 -- some object
      GlobalState + 0x5C -- some object
      GlobalState + 0x6C -- Quest object(?)
      GlobalState + 0x90 -- (float) message background transparency (0.0 - 1.0)
      GlobalState + 0xC0 -- some object pointer
      GlobalState + 0xD6 -- (boolean?) some flag, multiple uses
        useDefaultTransparency, if 0 - use specified message background transparency
      GlobalState + 0xDB -- (byte) exit flag (if 0 continue)
      GlobalState + 0x0108 -- some object pointer
      GlobalState + 0x018C -- some object pointer
      GlobalState + 0x01A8 -- some object pointer
      GlobalState + 0x032C -- (char *) starting cell (or cell to load?)
    
    Object ??? as Actor:
      method void SayDialogueVoice(int): 0x00528F80
      
      Actor + 0x14 -- some object pointer
    
    Object [Actor + 0x14] as UnkObj04:
      UnkObj04 + 0x28 -- some object pointer
    
    Object [UnkObj04 + 0x28] as UnkObj05:
      method bool IsNPCEssential(): UnkObj05 + 0xBC
    
    Object [0x007C67DC] + 0x34 as UnkObj01:
      UnkObj01 + 0x290 -- pointer to byte value
    
    Object [[0x007C67DC] + 0xC0] as UnkObj03:
      UnkObj03 + 0x34 -- (float?) 
    
    Object 007C67E0 as TES:
      method ReloadGame
      ???
    
    Object [0x007C6CDC] as TESGame:
      method MorrowindGameLoop
      TESGame + 0x0108 -- some object
    
    Object [[0x007C6CDC] + 0x0108]:
      method GetRandomSplashPath
    
    Object [[0x007C67DC] + 0x5C] as UnkObj02:
      UnkObj02 + 0x28 -- ProjectileManager object
*/

#ifndef MW_FUNCTIONS_H
#define MW_FUNCTIONS_H

internal uint32 globalStatePointer = 0x007C67DC;

typedef int (__cdecl *_ConsolePrint)(uint32 globalStatePointer, char *format, ...);
typedef void (__cdecl *_ShowGameMessage)(char *message, int unk1, int unk2);
//typedef void (__stdcall *_GetRandomSplashPath)(char *buffer, char flag);

internal const _ConsolePrint ConsolePrint = (_ConsolePrint)0x0040F970;
internal const _ShowGameMessage ShowGameMessage = (_ShowGameMessage)0x005F90C0;
//internal const _GetRandomSplashPath GetRandomSplashPath = (_GetRandomSplashPath)0x00459830;

internal const uint32 CompileAndRunAddress = 0x0050E5A0;

internal uint32 GetGlobalState()
{
  return *(uint32 *)globalStatePointer;
}

internal uint32 GetGlobalScript(uint32 globalState)
{
  return *(uint32 *)(globalState + 0x54);
}

internal uint32 GetGlobalScriptState(uint32 globalState)
{
  uint32 unknownObject = *(uint32 *)(globalState + 0x02E4);
  return *(uint32 *)(unknownObject + 0x20);
}

internal bool ExecuteScript(char *text)
{
  uint32 globalState = GetGlobalState();
  uint32 globalScriptState = GetGlobalScriptState(globalState);
  uint32 globalScript = GetGlobalScript(globalState);

  __asm {
    mov ecx, [globalScript]
  
    push 0
    push 0
    push 0
    push 0
    push 1
    push text
    push [globalScriptState]
    
    call CompileAndRunAddress
  }
  
  return true;
}

#endif
