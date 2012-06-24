#include "weak.h"

static BitBoard singlePushSources(Side, ChessSet*, BitBoard);
static BitBoard doublePushSources(Side, ChessSet*, BitBoard);
static BitBoard singlePushTargets(Side, ChessSet*, BitBoard);
static BitBoard doublePushTargets(Side, ChessSet*, BitBoard);

// Get BitBoard encoding capture sources for *all* pawns on specified side.
BitBoard
AllPawnCaptureSources(Side side, ChessSet *chessSet)
{
  switch(side) {
  case White:
    return PawnCaptureSources(side, chessSet, chessSet->White.Pawns);
  case Black:
    return PawnCaptureSources(side, chessSet, chessSet->Black.Pawns);
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

// Get BitBoard encoding capture targets for *all* pawns on specified side.
BitBoard
AllPawnCaptureTargets(Side side, ChessSet *chessSet)
{
  switch(side) {
  case White:
    return PawnCaptureTargets(side, chessSet, chessSet->White.Pawns);
  case Black:
    return PawnCaptureTargets(side, chessSet, chessSet->Black.Pawns);
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

// Get BitBoard encoding push sources for *all* pawns on specified side.
BitBoard
AllPawnPushSources(Side side, ChessSet *chessSet)
{
  switch(side) {
  case White:
    return PawnPushSources(side, chessSet, chessSet->White.Pawns);
  case Black:
    return PawnPushSources(side, chessSet, chessSet->Black.Pawns);
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

// Get BitBoard encoding push targets for *all* pawns on specified side.
BitBoard
AllPawnPushTargets(Side side, ChessSet *chessSet)
{
  switch(side) {
  case White:
    return PawnPushTargets(side, chessSet, chessSet->White.Pawns);
  case Black:
    return PawnPushTargets(side, chessSet, chessSet->Black.Pawns);
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

// Get BitBoard encoding all squares threatened by pawns.
BitBoard
AllPawnThreats(Side side, ChessSet *chessSet)
{
  BitBoard pawns;

  switch(side) {
  case White:
    pawns = chessSet->White.Pawns;
    pawns = NoWeOne(pawns) | NoEaOne(pawns);
    return pawns & ~SetOccupancy(&chessSet->White);
  case Black:
    pawns = chessSet->Black.Pawns;
    pawns = SoWeOne(pawns) | SoEaOne(pawns);
    return pawns & ~SetOccupancy(&chessSet->Black);
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;  
}

// Get BitBoard encoding specified pawns which are able to capture.
BitBoard
PawnCaptureSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  BitBoard whiteOccupancy, blackOccupancy;

  switch(side) {
  case White:
    blackOccupancy = SetOccupancy(&chessSet->Black);

    return pawns & (SoWeOne(blackOccupancy) | SoEaOne(blackOccupancy));
  case Black:
    whiteOccupancy = SetOccupancy(&chessSet->White);

    return pawns & (NoWeOne(whiteOccupancy) | NoEaOne(whiteOccupancy));
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

// Get BitBoard encoding squares able to be captured by specified pawns.
BitBoard
PawnCaptureTargets(Side side, ChessSet *chessSet, BitBoard pawns)
{
  BitBoard whiteOccupancy, blackOccupancy;

  switch(side) {
  case White:
    blackOccupancy = SetOccupancy(&chessSet->Black);

    return (NoWeOne(pawns) | NoEaOne(pawns)) & blackOccupancy;
  case Black:
    whiteOccupancy = SetOccupancy(&chessSet->White);

    return (SoWeOne(pawns) | SoEaOne(pawns)) & whiteOccupancy;
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

// Get BitBoard encoding push sources for specified pawns.
BitBoard
PawnPushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  return singlePushSources(side, chessSet, pawns) |
    doublePushSources(side, chessSet, pawns);
}

// Get BitBoard encoding push targets for specified pawns.
BitBoard
PawnPushTargets(Side side, ChessSet *chessSet, BitBoard pawns)
{
  return singlePushTargets(side, chessSet, pawns) |
    doublePushTargets(side, chessSet, pawns);
}

// Get BitBoard encoding which of specified pawns are able to single push.
static BitBoard
singlePushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  // Any pawn which is able to move forward one place must have an empty square
  // immediately in front of it. If we move all empty squares south one place, we end up
  // with a bitmask for all pawns able to move forward one place.  We do this, rather
  // than shift pawns one place forward and intersect with empty squares, because that
  // would result in targets rather than sources.
  switch(side) {
  case White:
    return SoutOne(EmptySquares(chessSet)) & pawns;
  case Black:
    return NortOne(EmptySquares(chessSet)) & pawns;
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding which of specified pawns are able to double push.
static BitBoard
doublePushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  BitBoard emptyRank4, emptyRank3And4, emptyRank5, emptyRank5And6;

  BitBoard empty = EmptySquares(chessSet);

  // Uses logic similar to the single push, however we have to a. take into account
  // double push is only possible if the target is rank 4 (5 for black), and b. consider
  // empty squares in both rank 3 and 4 (5 and 6 for black), as the pawn must pass
  // through both these ranks.
  switch(side) {
  case White:
    emptyRank4 = empty & Rank4Mask;
    emptyRank3And4 = empty & SoutOne(emptyRank4);
    return SoutOne(emptyRank3And4) & pawns;
  case Black:
    emptyRank5 = empty & Rank5Mask;
    emptyRank5And6 = empty & NortOne(emptyRank5);
    return NortOne(emptyRank5And6) & pawns;
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding single push targets for specified pawns.
static BitBoard
singlePushTargets(Side side, ChessSet *chessSet, BitBoard pawns)
{
  // Take advantage of the fact that shifting off the end of the uint64 simply pushes the
  // bits into oblivion, so we need not worry about pawns on the last rank (assuming this
  // method is used before promotion concerns are applied).
  switch(side) {
  case White:
    return NortOne(pawns) & EmptySquares(chessSet);
  case Black:
    return SoutOne(pawns) & EmptySquares(chessSet);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding double push targets for specified pawns.
static BitBoard
doublePushTargets(Side side, ChessSet *chessSet, BitBoard pawns)
{
  BitBoard singlePushes = singlePushTargets(side, chessSet, pawns);

  // We can only double-push pawns if they are on the 2nd rank and thus end up on the 4th
  // rank (5th for black).
  switch(side) {
  case White:
    return NortOne(singlePushes) & EmptySquares(chessSet) & Rank4Mask;
  case Black:
    return SoutOne(singlePushes) & EmptySquares(chessSet) & Rank5Mask;
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}
