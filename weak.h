#pragma once

#ifndef WEAK_HEADER
#define WEAK_HEADER

#include <stddef.h>
#include <stdint.h>

// See http://chessprogramming.wikispaces.com/Bitboards.
#define C64(constantU64) constantU64##ULL

typedef uint64_t BitBoard;

static const BitBoard
  EmptyBoard                     = C64(0),
  FullyOccupied                  = ~C64(0),
  CastleQueenSideWhiteMask       = C64(0x000000000000000e),
  CastleQueenSideWhiteAttackMask = C64(0x000000000000000c),
  CastleKingSideWhiteMask        = C64(0x0000000000000060),
  CastleQueenSideBlackMask       = C64(0x0e00000000000000),
  CastleQueenSideBlackAttackMask = C64(0x0c00000000000000),
  CastleKingSideBlackMask        = C64(0x6000000000000000),
  CentralSquaresMask             = C64(0x0000003c3c000000),
  Rank1Mask                      = C64(0x00000000000000ff),
  Rank2Mask                      = C64(0x000000000000ff00),
  Rank3Mask                      = C64(0x0000000000ff0000),
  Rank4Mask                      = C64(0x00000000ff000000),
  Rank5Mask                      = C64(0x000000ff00000000),
  Rank6Mask                      = C64(0x0000ff0000000000),
  Rank7Mask                      = C64(0x00ff000000000000),
  Rank8Mask                      = C64(0xff00000000000000),
  NotRank1Mask                   = ~C64(0x00000000000000ff),
  NotRank2Mask                   = ~C64(0x000000000000ff00),
  NotRank3Mask                   = ~C64(0x0000000000ff000),
  NotRank4Mask                   = ~C64(0x00000000ff00000),
  NotRank5Mask                   = ~C64(0x000000ff0000000),
  NotRank6Mask                   = ~C64(0x0000ff000000000),
  NotRank7Mask                   = ~C64(0x00ff00000000000),
  NotRank8Mask                   = ~C64(0xff0000000000000),
  FileAMask                      = C64(0x0101010101010101),
  FileBMask                      = C64(0x0202020202020202),
  FileCMask                      = C64(0x0404040404040404),
  FileDMask                      = C64(0x0808080808080808),
  FileEMask                      = C64(0x1010101010101010),
  FileFMask                      = C64(0x2020202020202020),
  FileGMask                      = C64(0x4040404040404040),
  FileHMask                      = C64(0x8080808080808080),
  NotFileAMask                   = ~C64(0x0101010101010101),
  NotFileBMask                   = ~C64(0x0202020202020202),
  NotFileCMask                   = ~C64(0x0404040404040404),
  NotFileDMask                   = ~C64(0x0808080808080808),
  NotFileEMask                   = ~C64(0x1010101010101010),
  NotFileFMask                   = ~C64(0x2020202020202020),
  NotFileGMask                   = ~C64(0x4040404040404040),
  NotFileHMask                   = ~C64(0x8080808080808080),
  InitWhitePawns                 = C64(0x000000000000ff00),
  InitWhiteRooks                 = C64(0x0000000000000081),
  InitWhiteKnights               = C64(0x0000000000000042),
  InitWhiteBishops               = C64(0x0000000000000024),
  InitWhiteQueens                = C64(0x0000000000000008),
  InitWhiteKing                  = C64(0x0000000000000010),
  InitBlackPawns                 = C64(0x00ff000000000000),
  InitBlackRooks                 = C64(0x8100000000000000),
  InitBlackKnights               = C64(0x4200000000000000),
  InitBlackBishops               = C64(0x2400000000000000),
  InitBlackQueens                = C64(0x0800000000000000),
  InitBlackKing                  = C64(0x1000000000000000);

// util.c
void* mal(size_t);
void free(void*);

// bitboard.c
int      PopCount(BitBoard);
BitBoard SoutOne(BitBoard);
BitBoard NortOne(BitBoard);
BitBoard WestOne(BitBoard);
BitBoard 
BitBoard  SoutOne(BitBoard);
BitBoard  NortOne(BitBoard);
BitBoard  WestOne(BitBoard);
BitBoard  EastOne(BitBoard);
BitBoard  NoEaOne(BitBoard);
BitBoard  SoEaOne(BitBoard);
BitBoard  SoWeOne(BitBoard);
BitBoard  NoWeOne(BitBoard);

#endif
