#include <string.h>
#include "weak.h"

static const int INIT_CASTLE_EVENT_COUNT = 100;

// Append CastleEvent to specified CastleEvent slice.
void
AppendCastleEvent(CastleEventSlice *slice, CastleEvent event)
{
  CastleEvent *buffer;

  // TODO: Avoid duplication :-)

  if(slice->Len > slice->Cap) {
    panic("CastleEventSlice->Len %d > CastleEventSlice->Cap %d - Impossible!", slice->Len,
          slice->Cap);
  }

  // Expand.
  if(slice->Len == slice->Cap) {
    slice->Cap *= 2;
    buffer = (CastleEvent*)allocate(sizeof(CastleEvent), slice->Cap);
    memcpy(buffer, slice->Vals, slice->Len);
    release(slice->Vals);
    slice->Vals = buffer;
  }

  slice->Vals[slice->Len] = event;
  slice->Len++;
}

CastleEventSlice
NewCastleEventSlice()
{
  CastleEventSlice ret;

  ret.Len = 0;
  ret.Cap = INIT_CASTLE_EVENT_COUNT;
  ret.Vals = (CastleEvent*)allocate(sizeof(CastleEvent), INIT_CASTLE_EVENT_COUNT);

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
