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

#ifndef HOOKS_H
#define HOOKS_H

//NOTE(adm244): addresses for hooks (morrowind 1.6.1820)
internal const uint32 mainloop_hook_patch_address = 0x00417227;
internal const uint32 mainloop_hook_return_address = 0x0041722D;

internal const uint32 firstloadstart_hook_patch_address = 0x004C4EB0;
internal const uint32 firstloadstart_hook_return_address = 0x004C4EB6;

internal const uint32 firstloadend_hook_patch_address = 0x004C5481;
internal const uint32 firstloadend_hook_return_address = 0x004C5486;

internal const uint32 reloadstart_hook_patch_address = 0x004C4800;
internal const uint32 reloadstart_hook_return_address = 0x004C4806;

internal const uint32 reloadend_hook_patch_address = 0x004C4E7A;
internal const uint32 reloadend_hook_return_address = 0x004C4E7F;

//NOTE(adm244): hooked functions prototypes
internal void GameLoop();

internal void FirstLoadStart();
internal void FirstLoadEnd();

internal void ReloadStart();
internal void ReloadEnd();

//NOTE(adm244): hook functions
internal void __declspec(naked) GameLoop_Hook()
{
  __asm {
    pushad
    call GameLoop
    popad
    
    //NOTE(adm244): original instructions
    mov ecx, dword ptr ds:[0x007C6CDC]

    jmp [mainloop_hook_return_address]
  }
}

internal void __declspec(naked) FirstLoadStart_Hook()
{
  __asm {
    pushad
    call FirstLoadStart
    popad
    
    //NOTE(adm244): original instructions
    mov eax, dword ptr fs:[0]
    
    jmp [firstloadstart_hook_return_address]
  }
}

internal void __declspec(naked) FirstLoadEnd_Hook()
{
  __asm {
    pushad
    call FirstLoadEnd
    popad
    
    //NOTE(adm244): original instructions
    mov ecx, dword ptr ss:[esp+0x6C]
    pop edi
    
    jmp [firstloadend_hook_return_address]
  }
}

internal void __declspec(naked) ReloadStart_Hook()
{
  __asm {
    pushad
    call ReloadStart
    popad
    
    //NOTE(adm244): original instructions
    mov eax, dword ptr fs:[0]
    
    jmp [reloadstart_hook_return_address]
  }
}

internal void __declspec(naked) ReloadEnd_Hook()
{
  __asm {
    pushad
    call ReloadEnd
    popad
    
    //NOTE(adm244): original instructions
    mov ecx, dword ptr ss:[esp+0x6C]
    pop edi
    
    jmp [reloadend_hook_return_address]
  }
}

internal void HookFirstLoading()
{
  WriteRelJump(firstloadstart_hook_patch_address, (uint32)&FirstLoadStart_Hook);
  SafeWrite8(firstloadstart_hook_patch_address + 5, 0x90);

  WriteRelJump(firstloadend_hook_patch_address, (uint32)&FirstLoadEnd_Hook);
}

internal void HookReloading()
{
  WriteRelJump(reloadstart_hook_patch_address, (uint32)&ReloadStart_Hook);
  SafeWrite8(reloadstart_hook_patch_address + 5, 0x90);
  
  WriteRelJump(reloadend_hook_patch_address, (uint32)&ReloadEnd_Hook);
}

//NOTE(adm244): patching in the morrowind main loop
internal void HookMainLoop()
{
  WriteRelJump(mainloop_hook_patch_address, (uint32)&GameLoop_Hook);
  SafeWrite8(mainloop_hook_patch_address + 5, 0x90);
}

#endif
