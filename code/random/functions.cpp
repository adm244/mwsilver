#ifndef RANDOM_FUNCTIONS_CPP
#define RANDOM_FUNCTIONS_CPP

#include "random/randomlib.c"

internal int randomGenerated = 0;
internal uint8 randomCounters[MAX_BATCHES];

internal void RandomClearCounters()
{
  randomGenerated = 0;
  for( int i = 0; i < MAX_BATCHES; ++i ) {
    randomCounters[i] = 0;
  }
}

internal int GetNextBatchIndex(int batchesCount)
{
  if( randomGenerated >= batchesCount ) {
    RandomClearCounters();
  }
  
  int value;
  for( ;; ) {
    value = RandomInt(0, batchesCount - 1);
    if( randomCounters[value] == 0 ) break;
  }
  
  ++randomGenerated;
  randomCounters[value] = 1;
  
  return value;
}

internal void RandomGeneratorInitialize(int batchesCount)
{
  int ticksPassed = GetTickCount();
  
  int ij = ticksPassed % 31328;
  int kj = ticksPassed % 30081;
  
  RandomInitialize(ij, kj);
  RandomClearCounters();
}

#endif
