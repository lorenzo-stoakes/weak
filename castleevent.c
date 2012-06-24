#include <string.h>
#include "weak.h"

static const int INIT_CASTLE_EVENT_COUNT = 4;

// Append CastleEvent to specified CastleEvent slice.
CastleEventSlice
AppendCastleEvent(CastleEventSlice slice, CastleEvent event)
{
  // TODO: Avoid duplication :-)  

  CastleEventSlice ret;

  if(slice.Len > slice.Cap) {
    panic("CastleEventSlice.Len %d > CastleEventSlice.Cap %d - Impossible!", slice.Len,
          slice.Cap);
  }

  // Expand.
  if(slice.Len == slice.Cap) {
    ret.Cap = slice.Cap*2;
    ret.Vals = (CastleEvent*)allocate(sizeof(CastleEvent)*ret.Cap);
    memcpy(ret.Vals, slice.Vals, slice.Len);
    release(slice.Vals);
  } else {
    ret.Vals = slice.Vals;
    ret.Cap = slice.Cap;
  }

  ret.Vals[slice.Len] = event;
  ret.Len = slice.Len+1;

  return ret;
}

CastleEventSlice
NewCastleEventSlice()
{
  CastleEventSlice ret;

  ret.Len = 0;
  ret.Cap = INIT_CASTLE_EVENT_COUNT;
  ret.Vals = (CastleEvent*)allocate(sizeof(CastleEvent)*INIT_CASTLE_EVENT_COUNT);

  return ret;
}

CastleEvent
PopCastleEvent(CastleEventSlice *slice)
{
  CastleEvent ret;

  if(slice->Len <= 0) {
    panic("Invaild slice length %d on PopCastleEvent().", slice->Len);    
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}
