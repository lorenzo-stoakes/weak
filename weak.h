/*
  Weak, a chess engine derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Weak is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Weak is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef WEAK_HEADER
#define WEAK_HEADER

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define USE_BITSCAN_ASM

// See http://chessprogramming.wikispaces.com/Bitboards.
#define C64(constantU64) constantU64##ULL
#define RANK(pos) ((pos)/8)
#define FILE(pos) ((pos)&7)
#define OPPOSITE(side) ((Side)(1-(side)))
#define POSBOARD(pos) ((BitBoard)(1ULL<<(pos)))
#define POSITION(rank, file) ((Position)((rank)*8 + (file)))

#define FORCE_INLINE inline __attribute__((always_inline))
#define PACKED __attribute__((packed))

#define APPEND_STRING_BUFFER_LENGTH 2000
#define INIT_MOVE_LEN 192
#define KISS_WARMUP_ROUNDS 100
#define MAX_PIECE_LOCATION 10

#define BIG    INT_MAX
#define SMALL -INT_MAX

#define MAX_SEARCH_DEPTH 5

#define TRANS_CLUSTER_SIZE 4

/*

From    1 size = 6
To      2 size = 6
Type    3 size = 4

  3         2         1
21098765432109876543210987654321
00000000000000003333111111222222 shift
                |   |     222222   0
                |   111111......   6
                3333............   12

*/

#define MOVE_MASK(n) ((1<<n)-1)

#define MAKE_MOVE_QUICK(from, to) ((Move)(((from)<<6)|(to)))

#define MAKE_MOVE(from, to, type)             \
  ((Move)(((type)<<12)|((from)<<6)|(to)))

#define TO(move)      ((Position)(      (move)&(MOVE_MASK(6))))
#define FROM(move)    ((Position)( ((move)>>6)&(MOVE_MASK(6))))
#define TYPE(move)    ((MoveType)(((move)>>12)&(MOVE_MASK(4))))

enum CastleEvent {
  NoCastleEvent      = 0,
  LostKingSideWhite  = 1 << 0,
  LostKingSideBlack  = 1 << 1,
  LostQueenSideWhite = 1 << 2,
  LostQueenSideBlack = 1 << 3
};

enum CastleSide {
  KingSide,
  QueenSide
};

enum CommandType {
  CmdAnalysis,
  CmdBoard,
  CmdInvalid,
  CmdMove,
  CmdMoves,
  CmdPerft,
  CmdPerftFull,
  CmdPositionFen,
  CmdQuit
};

enum File {
  FileA = 0,
  FileB = 1,
  FileC = 2,
  FileD = 3,
  FileE = 4,
  FileF = 5,
  FileG = 6,
  FileH = 7
};


enum Piece {
  MissingPiece,
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King
};

// Various MoveType configurations:-

// EnPassant
// 0001

// Castle Queen Side
// 0010

// Castle King Side
// 0011

// Promote Knight
// 1010

// Promote Bishop
// 1011

// Promote Rook
// 1100

// Promote Queen
// 1101

// Invalid move type
// 1111

enum MoveType {
  Normal          = 0,
  EnPassant       = 1,
  // TODO: CastleMask isn't sufficient to definitely determine whether a move is a castle. Fix.
  CastleMask      = 1<<1,
  CastleQueenSide = CastleMask + 0,
  CastleKingSide  = CastleMask + 1,
  PromoteMask     = 1<<3,
  PromoteKnight   = PromoteMask + Knight,
  PromoteBishop   = PromoteMask + Bishop,
  PromoteRook     = PromoteMask + Rook,
  PromoteQueen    = PromoteMask + Queen,
  InvalidMoveType = (1<<4)-1
};

#define INVALID_MOVE MAKE_MOVE(InvalidMoveType, A1, A1)

enum Position {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8,
  EmptyPosition
};

enum Rank {
  Rank1 = 0,
  Rank2 = 1,
  Rank3 = 2,
  Rank4 = 3,
  Rank5 = 4,
  Rank6 = 5,
  Rank7 = 6,
  Rank8 = 7
};

enum Side {
  White = 0,
  Black = 1
};

// Generated from 'git describe' command during build.
extern const char *version;

// A bitboard is an efficient representation of the occupancy of a chessboard [0].
// We use little-endian rank-file (LERF) mapping [1].
typedef uint64_t             BitBoard;
typedef enum CastleEvent     CastleEvent;
typedef enum CastleSide      CastleSide;
typedef struct CheckStats    CheckStats;
typedef struct ChessSet      ChessSet;
typedef struct Command       Command;
typedef enum CommandType     CommandType;
typedef struct Game          Game;
typedef uint16_t             QuickMove;
typedef struct Memory        Memory;
typedef struct MemorySlice   MemorySlice;
typedef uint64_t             Move;
typedef struct MoveSlice     MoveSlice;
typedef enum MoveType        MoveType;
typedef struct PerftStats    PerftStats;
typedef enum Piece           Piece;
typedef enum Position        Position;
typedef enum Rank            Rank;
typedef enum File            File;
typedef struct Set           Set;
typedef enum Side            Side;
typedef struct StringBuilder StringBuilder;
typedef struct TransCluster  TransCluster;
typedef struct TransEntry    TransEntry;

struct CheckStats {
  BitBoard CheckSquares[7], CheckSources, Discovered, Pinned;
  Position DefendedKing, AttackedKing;
};

struct Command {
  CommandType Type;
  Move        Move;
  int         PerftDepth;
  char        *Fen;
};

struct MemorySlice {
  Memory *Vals, *Curr;
};

struct MoveSlice {
  Move *Vals, *Curr;
};

struct Set {
  BitBoard EmptySquares, Occupancy;
  BitBoard Boards[7];
};

struct Memory {
  CastleEvent CastleEvent;
  CheckStats  CheckStats;
  Position    EnPassantSquare;
  Move        Move;
  Piece       Captured;
};

struct ChessSet {
  BitBoard EmptySquares, Occupancy;
  BitBoard PieceOccupancy[7];
  int      PieceCounts[2][7];
  int      PiecePositionIndexes[64];
  Position PiecePositions[2][7][MAX_PIECE_LOCATION];
  Piece    Squares[64];
  Set      Sets[2];
};

struct Game {
  bool        CastlingRights[2][2];
  CheckStats  CheckStats;
  ChessSet    ChessSet;
  bool        Debug;
  Position    EnPassantSquare;
  uint64_t    Hash;
  MemorySlice Memories;
  Side        WhosTurn, HumanSide;
};

struct PerftStats {
  uint64_t Count, Captures, EnPassants, Castles, Promotions, Checks, Checkmates;
};

struct StringBuilder {
  // Length is the total number of characters in the builder.
  int Length;
  // cap, len refer to the capacity/length of the number of strings in the builder.
  int cap, len;
  char **strings;
};

struct TransEntry {
  uint16_t Depth;
  uint8_t  Generation;
  uint32_t Key32;
  uint16_t QuickMove;
  uint16_t Value;
};

struct TransCluster {
  TransEntry Data[TRANS_CLUSTER_SIZE];
};

static const BitBoard
  CastlingAttackMasks[2][2]      = {{ C64(0x0000000000000060), C64(0x000000000000000c) },
                                    { C64(0x6000000000000000), C64(0x0c00000000000000) }},
  CastlingMasks[2][2]            = {{ C64(0x0000000000000060), C64(0x000000000000000e) },
                                    { C64(0x6000000000000000), C64(0x0e00000000000000) }},
  CentralSquaresMask             =  C64(0x0000003c3c000000),
  EdgeMask                       =  C64(0xff818181818181ff),
  EmptyBoard                     =  C64(0x0000000000000000),
  FileAMask                      =  C64(0x0101010101010101),
  FileBMask                      =  C64(0x0202020202020202),
  FileCMask                      =  C64(0x0404040404040404),
  FileDMask                      =  C64(0x0808080808080808),
  FileEMask                      =  C64(0x1010101010101010),
  FileFMask                      =  C64(0x2020202020202020),
  FileGMask                      =  C64(0x4040404040404040),
  FileHMask                      =  C64(0x8080808080808080),
  FullyOccupied                  = ~C64(0x0000000000000000),
  InitBlackBishops               =  C64(0x2400000000000000),
  InitBlackKing                  =  C64(0x1000000000000000),
  InitBlackKnights               =  C64(0x4200000000000000),
  InitBlackOccupancy             =  C64(0xffff000000000000),
  InitBlackPawns                 =  C64(0x00ff000000000000),
  InitBlackQueens                =  C64(0x0800000000000000),
  InitBlackRooks                 =  C64(0x8100000000000000),
  InitOccupancy                  =  C64(0xffff00000000ffff),
  InitWhiteBishops               =  C64(0x0000000000000024),
  InitWhiteKing                  =  C64(0x0000000000000010),
  InitWhiteKnights               =  C64(0x0000000000000042),
  InitWhiteOccupancy             =  C64(0x000000000000ffff),
  InitWhitePawns                 =  C64(0x000000000000ff00),
  InitWhiteQueens                =  C64(0x0000000000000008),
  InitWhiteRooks                 =  C64(0x0000000000000081),
  NotFileAMask                   = ~C64(0x0101010101010101),
  NotFileBMask                   = ~C64(0x0202020202020202),
  NotFileCMask                   = ~C64(0x0404040404040404),
  NotFileDMask                   = ~C64(0x0808080808080808),
  NotFileEMask                   = ~C64(0x1010101010101010),
  NotFileFMask                   = ~C64(0x2020202020202020),
  NotFileGMask                   = ~C64(0x4040404040404040),
  NotFileHMask                   = ~C64(0x8080808080808080),
  NotRank1Mask                   = ~C64(0x00000000000000ff),
  NotRank2Mask                   = ~C64(0x000000000000ff00),
  NotRank3Mask                   = ~C64(0x0000000000ff0000),
  NotRank4Mask                   = ~C64(0x00000000ff000000),
  NotRank5Mask                   = ~C64(0x000000ff00000000),
  NotRank6Mask                   = ~C64(0x0000ff0000000000),
  NotRank7Mask                   = ~C64(0x00ff000000000000),
  NotRank8Mask                   = ~C64(0xff00000000000000),
  Rank1Mask                      =  C64(0x00000000000000ff),
  Rank2Mask                      =  C64(0x000000000000ff00),
  Rank3Mask                      =  C64(0x0000000000ff0000),
  Rank4Mask                      =  C64(0x00000000ff000000),
  Rank5Mask                      =  C64(0x000000ff00000000),
  Rank6Mask                      =  C64(0x0000ff0000000000),
  Rank7Mask                      =  C64(0x00ff000000000000),
  Rank8Mask                      =  C64(0xff00000000000000);

// Ray lookup arrays. Used in inlined function, hence location.
BitBoard nortRays[64], eastRays[64], soutRays[64], westRays[64],
  noeaRays[64], soweRays[64], noweRays[64], soeaRays[64];

FORCE_INLINE void
AppendMemory(MemorySlice *slice, Memory memory)
{
  *slice->Curr++ = memory;
}

FORCE_INLINE Move
PopMove(MoveSlice *slice)
{
  slice->Curr--;

  return *slice->Curr;
}

FORCE_INLINE Memory
PopMemory(MemorySlice *slice)
{
  slice->Curr--;

  return *slice->Curr;
}

FORCE_INLINE void
AppendMove(MoveSlice *slice, Move move)
{
  *slice->Curr++ = move;
}

// See http://chessprogramming.wikispaces.com/BitScan#bsfbsr
#if defined(USE_BITSCAN_ASM)
FORCE_INLINE Position
BitScanBackward(BitBoard bitBoard)
{
  BitBoard posBoard;
  __asm__("bsrq %1, %0": "=r"(posBoard): "rm"(bitBoard));
  return (Position)posBoard;
}

FORCE_INLINE Position
BitScanForward(BitBoard bitBoard)
{
  BitBoard posBoard;
  __asm__("bsfq %1, %0": "=r"(posBoard): "rm"(bitBoard));
  return (Position)posBoard;
}
#else
// bitboard.c
Position BitScanBackward(BitBoard);
Position BitScanForward(BitBoard);
#endif

static FORCE_INLINE int
FileDistance(Position from, Position to)
{
  return abs(FILE(from) - FILE(to));
}

static FORCE_INLINE int
LenMoves(Move *start, Move *end)
{
  return end - start;
}

static FORCE_INLINE Piece
PieceAt(ChessSet *chessSet, Position pos)
{
  return chessSet->Squares[pos];
}

static FORCE_INLINE void
PlacePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  chessSet->Squares[pos] = piece;
  chessSet->Sets[side].Boards[piece] |= POSBOARD(pos);
}

static FORCE_INLINE void
RemovePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  BitBoard complement = ~POSBOARD(pos);

  chessSet->Squares[pos] = MissingPiece;
  chessSet->Sets[side].Boards[piece] &= complement;
}

static FORCE_INLINE bool
SingleBit(BitBoard bitBoard) {
  // This is clever (not my idea!) - if there is only 1 bit, it'll be the msb. (n-1) gives you
  // all the bits below the msb, so if there's only 1 bit set, anding these will return 0.
  return (bitBoard & (bitBoard - 1)) == C64(0);
}

static FORCE_INLINE int
RankDistance(Position from, Position to)
{
  return abs(RANK(from) - RANK(to));
}

static FORCE_INLINE Position
PopForward(BitBoard *bitBoard)
{
  Position ret = BitScanForward(*bitBoard);

  *bitBoard ^= POSBOARD(ret);

  return ret;
}

static FORCE_INLINE BitBoard
NortOne(BitBoard bitBoard)
{
  return bitBoard << 8;
}

static FORCE_INLINE BitBoard
NortRay(Position pos)
{
  return nortRays[pos];
}

static FORCE_INLINE BitBoard
EastOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileHMask) << 1;
}

static FORCE_INLINE BitBoard
EastRay(Position pos)
{
  return eastRays[pos];
}

static FORCE_INLINE BitBoard
SoutOne(BitBoard bitBoard)
{
  return bitBoard >> 8;
}

static FORCE_INLINE BitBoard
SoutRay(Position pos)
{
  return soutRays[pos];
}

static FORCE_INLINE BitBoard
WestOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileAMask) >> 1;
}

static FORCE_INLINE BitBoard
WestRay(Position pos)
{
  return westRays[pos];
}

static FORCE_INLINE BitBoard
NoEaOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileHMask) << 9;
}

static FORCE_INLINE BitBoard
NoEaRay(Position pos)
{
  return noeaRays[pos];
}

static FORCE_INLINE BitBoard
NoWeOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileAMask) << 7;
}

static FORCE_INLINE BitBoard
NoWeRay(Position pos)
{
  return noweRays[pos];
}

static FORCE_INLINE BitBoard
SoEaOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileHMask) >> 7;
}

static FORCE_INLINE BitBoard
SoEaRay(Position pos)
{
  return soeaRays[pos];
}

static FORCE_INLINE BitBoard
SoWeOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileAMask) >> 9;
}

static FORCE_INLINE BitBoard
SoWeRay(Position pos)
{
  return soweRays[pos];
}

// bitboard.c
bool     Aligned(Position, Position, Position);
BitBoard FlipDiagA1H8(BitBoard);
BitBoard FlipVertical(BitBoard);
void     InitRays(void);
int      PopCount(BitBoard);
bool     PositionOccupied(BitBoard, Position);
BitBoard Rotate90AntiClockwise(BitBoard);
BitBoard Rotate90Clockwise(BitBoard);

// eval.c
int Eval(Game*);

// game.c
CheckStats CalculateCheckStats(Game*);
bool       Checked(Game*);
bool       Checkmated(Game*);
bool       GivesCheck(Game*, Move);
void       InitEngine(void);
void       DoMove(Game*, Move);
bool       Legal(Game*, Move);
CheckStats NewCheckStats(void);
Game       NewEmptyGame(bool, Side);
Game       NewGame(bool, Side);
bool       PseudoLegal(Game*, Move, BitBoard);
bool       Stalemated(Game*);
void       Unmove(Game*);

// hash.c
uint64_t HashGame(Game*);
void     InitZobrist(void);

// interface.c
void RunInterface(Game*);

// magic.c
void InitMagics(void);

// movegen.c
Move* AllCaptures(Move*, Game*);
Move* AllMoves(Move*, Game*);
bool  AnyMoves(Game*);
Move* CastleMoves(Game*, Move*);
Move* Evasions(Move*, Game*);

// parser.c
Command ParseCommand(char*);
Game    ParseFen(char*);
Move    ParseMove(char*);

// perft.c
PerftStats Perft(Game*, int);
uint64_t   QuickPerft(Game*, int);

// pieces.c
BitBoard AllAttackersTo(ChessSet*, Position, BitBoard);
BitBoard BishopAttacksFrom(Position, BitBoard);
BitBoard CalcBishopSquareThreats(Position, BitBoard);
BitBoard CalcRookSquareThreats(Position, BitBoard);
void     InitKing(void);
void     InitKnight(void);
void     InitPawn(void);
BitBoard KingAttacksFrom(Position);
BitBoard KnightAttacksFrom(Position);
BitBoard PawnAttacksFrom(Position, Side);

// prng.c
uint64_t randk(void);
void     randk_seed(void);
void     randk_warmup(int);

// search.c
Move Search(Game*, uint64_t*, int);

// set.c
Set      NewBlackSet(void);
ChessSet NewChessSet(void);
ChessSet NewEmptyChessSet(void);
Set      NewEmptySet(void);
Set      NewWhiteSet(void);
BitBoard PinnedPieces(ChessSet*, Side, Position, bool);
void     UpdateOccupancies(ChessSet*);

// slices.c
MemorySlice NewMemorySlice(void);
MoveSlice   NewMoveSlice(Move*);

// stringer.c
char  CharPiece(Piece);
char* StringBitBoard(BitBoard);
char* StringChessSet(ChessSet*);
char* StringMove(Move);
char* StringMoveFull(Move, Piece, bool);
char* StringMoveHistory(MemorySlice*);
char* StringPerft(PerftStats*);
char* StringPiece(Piece);
char* StringPosition(Position);
char* StringSide(Side);

// trans.c
void        InitTrans(void);
TransEntry* LookupPosition(uint64_t);
void        NextSearch(void);
void        ResizeTrans(uint64_t);
void        SavePosition(uint64_t, uint16_t, uint16_t, uint16_t);

// util.c
void*         allocate(size_t, size_t);
void*         allocateZero(size_t, size_t);
void          release(void*);
void          panic(char*, ...);
void          AppendString(StringBuilder *, char*, ...);
char*         BuildString(StringBuilder*, bool);
int           Max(int, int);
StringBuilder NewStringBuilder(void);
void          ReleaseStringBuilder(StringBuilder*);
void          SetUnbufferedOutput(void);

// Array containing BitBoard of positions between two specified squares, as long as
// they are on the same rank/file/diagonal. This is exclusive of the from and to squares.
BitBoard Between[64][64];

// Array containing lookups determining whether any sliding attack is possible
// between the two positions.
bool CanSlideAttack[64][64];

// Array containing distances between positions on the board.
int Distance[64][64];

// Array containing attacks for a specified piece and position on an empty BitBoard.
// We only calculate this for sliding pieces.
BitBoard EmptyAttacks[6][64];

uint64_t ZobristCastlingHash[2][2];
uint64_t ZobristEnPassantFileHash[8];
uint64_t ZobristPositionHash[2][7][64];
uint64_t ZobristBlackHash;

#endif
