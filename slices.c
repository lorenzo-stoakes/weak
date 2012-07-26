#include "weak.h"

static const int INIT_CASTLE_EVENT_COUNT = 100;
static const int INIT_CHECK_STATS_COUNT  = 100;
static const int INIT_ENPASSANT_COUNT    = 100;
static const int INIT_PIECE_COUNT        = 32;

// TODO: Avoid duplication throughout.

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
LenCastleEvents(CastleEventSlice *slice)
{
  return slice->Curr - slice->Vals;
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

  ret.Vals = (CastleEvent*)allocate(sizeof(CastleEvent), INIT_CASTLE_EVENT_COUNT);
  ret.Curr = ret.Vals;

  return ret;
}

CheckStatsSlice
NewCheckStatsSlice()
{
  CheckStatsSlice ret;

  ret.Vals = (CheckStats*)allocate(sizeof(CheckStats), INIT_CHECK_STATS_COUNT);
  ret.Curr = ret.Vals;

  return ret;
}

EnPassantSlice
NewEnPassantSlice()
{
  EnPassantSlice ret;

  ret.Vals = (Position*)allocate(sizeof(Position), INIT_ENPASSANT_COUNT);
  ret.Curr = ret.Vals;

  return ret;
}

MoveSlice
NewMoveSlice(Move *buffer)
{
  MoveSlice ret;

  ret.Vals = buffer;
  ret.Curr = buffer;

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
  slice->Curr--;

  return *slice->Curr;  
}

CheckStats
PopCheckStats(CheckStatsSlice *slice)
{
  slice->Curr--;

  return *slice->Curr;
}

Position
PopEnPassantSquare(EnPassantSlice *slice)
{
  slice->Curr--;

  return *slice->Curr;
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
