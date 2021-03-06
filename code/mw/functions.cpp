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

#ifndef MW_FUNCTIONS_CPP
#define MW_FUNCTIONS_CPP

#include "mw/native_functions.cpp"

internal void DisplayMessage(char *message)
{
  ShowGameMessage(message, 0, 1);
}

internal void DisplaySuccessMessage(char *batchName)
{
  char statusString[260];
  sprintf(statusString, Strings.Message, batchName);
  
  ConsolePrint(globalStatePointer, statusString);
  
  if( Settings.ShowMessages ) {
    DisplayMessage(statusString);
  }
}

internal void DisplayRandomSuccessMessage(char *batchName)
{
  char statusString[260];
  sprintf(statusString, Strings.MessageRandom, batchName);
  
  ConsolePrint(globalStatePointer, statusString);
  
  if( Settings.ShowMessagesRandom ) {
    DisplayMessage(statusString);
  }
}

#endif
