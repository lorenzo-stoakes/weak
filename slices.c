#include <string.h>
#include "weak.h"

static const int INIT_CASTLE_EVENT_COUNT = 100;
static const int INIT_ENPASSANT_COUNT    = 100;
static const int INIT_PIECE_COUNT        = 32;

static void     expandCastleEventSlice(CastleEventSlice *slice);
static void     expandEnPassantSlice(EnPassantSlice*);

void
AppendCastleEvent(CastleEventSlice *slice, CastleEvent event)
{
  // TODO: Avoid duplication :-)

  if(slice->Len > slice->Cap) {
    panic("CastleEventSlice->Len %d > CastleEventSlice->Cap %d - Impossible!", slice->Len,
          slice->Cap);
  }

  if(slice->Len == slice->Cap) {
    expandCastleEventSlice(slice);
  }

  slice->Vals[slice->Len] = event;
  slice->Len++;
}

void
AppendEnpassantSquare(EnPassantSlice *slice, Position pos)
{
  // TODO: Avoid duplication :-)

  if(slice->Len > slice->Cap) {
    panic("EnPassantSlice->Len %d > EnPassantSlice->Cap %d - Impossible!", slice->Len,
          slice->Cap);
  }

  if(slice->Len == slice->Cap) {
    expandEnPassantSlice(slice);
  }

  slice->Vals[slice->Len] = pos;
  slice->Len++;
}

void
AppendPiece(PieceSlice *slice, Piece piece)
{
  // Never need to expand slice, as there can never be more than 32 pieces on the board.
  if(slice->Len >= slice->Cap) {
    panic("Impossible condition, have %d pieces.", slice->Len);
  }

  slice->Vals[slice->Len] = piece;
  slice->Len++;
}

int
LenMoves(MoveSlice *slice)
{
  return slice->Curr - slice->Vals;
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

EnPassantSlice
NewEnPassantSlice()
{
  EnPassantSlice ret;

  ret.Len = 0;
  ret.Cap = INIT_ENPASSANT_COUNT;
  ret.Vals = (Position*)allocate(sizeof(Position), INIT_ENPASSANT_COUNT);

  return ret;
}

MoveSlice
NewMoveSlice(Move *buffer)
{
  MoveSlice ret;

  ret.Vals = buffer;
  ret.Curr = buffer;
  ret.Len = 0;

  return ret;
}

PieceSlice
NewPieceSlice()
{
  PieceSlice ret;

  ret.Cap = INIT_PIECE_COUNT;
  ret.Len = 0;
  ret.Vals = (Piece*)allocate(sizeof(Piece), INIT_PIECE_COUNT);

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

Position
PopEnPassantSquare(EnPassantSlice *slice)
{
  Position ret;

  if(slice->Len <= 0) {
    panic("Invaild slice length %d on PopEnPassantSquare().", slice->Len);
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}

Move
PopMove(MoveSlice *slice)
{
  slice->Curr--;

  return *slice->Curr;
}

Piece
PopPiece(PieceSlice *slice)
{
  Piece ret;

  if(slice->Len <= 0) {
    panic("Invaild slice length %d on PopPiece().", slice->Len);
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}

static void
expandCastleEventSlice(CastleEventSlice *slice)
{
  CastleEvent *buffer;

  slice->Cap *= 2;
  buffer = (CastleEvent*)allocate(sizeof(CastleEvent), slice->Cap);
  memcpy(buffer, slice->Vals, slice->Len*sizeof(CastleEvent*));
  release(slice->Vals);
  slice->Vals = buffer;
}

static void
expandEnPassantSlice(EnPassantSlice *slice)
{
  Position *buffer;

  slice->Cap *= 2;
  buffer = (Position*)allocate(sizeof(Position), slice->Cap);
  memcpy(buffer, slice->Vals, slice->Len*sizeof(Position*));
  release(slice->Vals);
  slice->Vals = buffer;
}
