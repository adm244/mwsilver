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

#ifndef MW_TYPES_H
#define MW_TYPES_H

#define REGN_NAME_LENGTH 32

//TODO(adm244): assert structure sizes

#pragma pack(push, 1)
struct TESRegion {
  void *vtable; // 0x00
  char signature[4]; // 0x04
  uint32 unk08;
  uint32 unk0C;
  char name[32]; // 0x10
  char description[32]; // 0x30
  uint8 clearChance; // 0x50
  uint8 cloudyChance; // 0x51
  uint8 foggyChance; // 0x52
  uint8 overcastChance; // 0x53
  uint8 rainChance; // 0x54
  uint8 thunderChance; // 0x55
  uint8 ashChance; // 0x56
  uint8 blightChance; // 0x57
  uint8 snowChance; // 0x58
  uint8 blizzardChance; // 0x59
  uint8 gap5A[2];
  void *ptrLEVC; // 0x5C
  uint32 unk60;
  uint32 unk64; // sounds count?
  void *unk68;
  void *unk6C;
  uint32 unk70;
  uint32 unk74;
  uint32 unk78;
  uint32 unk7C;
}; // 128 bytes = 0x80
#pragma pack(pop)

#pragma pack(push, 1)
struct TESCell {
  void *vtable;
  char signature[4];
  uint32 unk08;
  uint32 unk0C;
  char *name;
  uint32 unk14;
  uint32 flags;
  
  union {
    uint32 unk1C;
    uint32 ambient; // 0x1C
  };
  union {
    void *land; // 0x20
    uint32 sunlight;
  };
  union {
    uint32 x; // 0x24
    uint32 fog;
  };
  union {
    uint32 y; // 0x28
    float density;
  };
  
  uint32 unk2C;
  uint32 unk30;
  void *ptrToRef01;
  void *ptrToRef02;
  TESCell *ptrToCell01;
  uint32 unk40;
  void *ptrToRef03;
  void *ptrToRef04;
  TESCell *ptrToCell02;
  uint32 unk50;
  uint32 unk54;
  uint32 unk58;
  uint32 unk5C;
  uint32 unk60;
  TESCell *ptrToCell03;
  void *unk68;
  uint32 unk6C;
  void *unk70;
  void *unk74;
  void *unk78;
  uint32 unk7C;
  uint32 unk80;
  uint32 unk84;
  uint32 unk88;
  void *unk8C;
  TESRegion *region;
  uint32 unk94;
  uint32 unk98;
  uint32 unk9C;
}; // 160 bytes = 0xA0
#pragma pack(pop)

#endif
