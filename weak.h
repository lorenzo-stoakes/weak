#ifndef WEAK_HEADER
#define WEAK_HEADER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define USE_BITSCAN_ASM

// See http://chessprogramming.wikispaces.com/Bitboards.
#define C64(constantU64) constantU64##ULL
#define RANK(pos) ((Rank)((pos)/8))
#define FILE(pos) ((File)((pos)&7))
#define OPPOSITE(side) ((Side)(1-(side)))
#define POSBOARD(pos) ((BitBoard)(1ULL<<(pos)))
#define POSITION(rank, file) ((Position)((rank)*8 + (file)))

#define FORCE_INLINE inline __attribute__((always_inline))

#define INIT_MOVE_LEN 100

enum CastleEvent {
  NoCastleEvent = 1 << 0,
  LostKingSideWhite = 1 << 1,
  LostQueenSideWhite = 1 << 2,
  LostKingSideBlack = 1 << 3,
  LostQueenSideBlack = 1 << 4
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

enum MoveType {
  Normal,
  CastleQueenSide,
  CastleKingSide,
  PromoteKnight,
  PromoteBishop,
  PromoteRook,
  PromoteQueen,
  EnPassant
};

enum Piece {
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King,
  MissingPiece
};

enum Position {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8
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
  White,
  Black
};

// A bitboard is an efficient representation of the occupancy of a chessboard [0].
// We use little-endian rank-file (LERF) mapping [1].
typedef uint64_t                BitBoard;
typedef enum CastleEvent        CastleEvent;
typedef struct CastleEventSlice CastleEventSlice;
typedef struct ChessSet         ChessSet;
typedef struct Game             Game;
typedef struct Move             Move;
typedef struct MoveHistory      MoveHistory;
typedef struct MoveSlice        MoveSlice;
typedef enum MoveType           MoveType;
typedef struct PerftStats       PerftStats;
typedef enum Piece              Piece;
typedef struct PieceSlice       PieceSlice;
typedef enum Position           Position;
typedef enum Rank               Rank;
typedef enum File               File;
typedef struct Set              Set;
typedef enum Side               Side;

struct CastleEventSlice {
  int Len, Cap;
  CastleEvent *Vals;
};

struct MoveSlice {
  uint64_t Len, Cap;
  Move *Vals;
};

struct PieceSlice {
  int Len, Cap;
  Piece *Vals;
};

struct Set {
  BitBoard EmptySquares, Occupancy;
  BitBoard Boards[6];
};

struct MoveHistory {
  CastleEventSlice CastleEvents;
  MoveSlice        Moves;
  PieceSlice       CapturedPieces;
};

struct ChessSet {
  BitBoard EmptySquares, Occupancy;
  Set      Sets[2];
};

struct Game {
  bool        CastleKingSideWhite, CastleQueenSideWhite;
  bool        CastleKingSideBlack, CastleQueenSideBlack;
  ChessSet    ChessSet;
  bool        Debug;
  MoveHistory History;
  Side        WhosTurn, HumanSide;
};

struct Move {
  Piece    Piece;
  Position From, To;
  bool     Capture;
  MoveType Type;
};

struct PerftStats {
  uint64_t Count, Captures, EnPassants, Castles, Promotions, Checks, Checkmates;
};

static const BitBoard
  CastleKingSideBlackMask        =  C64(0x6000000000000000),
  CastleKingSideWhiteMask        =  C64(0x0000000000000060),
  CastleQueenSideWhiteMask       =  C64(0x000000000000000e),
  CastleQueenSideWhiteAttackMask =  C64(0x000000000000000c),
  CastleQueenSideBlackMask       =  C64(0x0e00000000000000),
  CastleQueenSideBlackAttackMask =  C64(0x0c00000000000000),
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

// bishop.c
BitBoard AllBishopCaptureTargets(ChessSet*, Side);
BitBoard AllBishopMoveTargets(ChessSet*, Side);
BitBoard BishopCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard BishopKingThreats(ChessSet*, Side);
BitBoard BishopMoveTargets(ChessSet*, Side, BitBoard);
BitBoard BishopSquareThreats(Position, BitBoard);
BitBoard BishopThreats(BitBoard, BitBoard);

// bitboard.c

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
Position BitScanBackward(BitBoard);
Position BitScanForward(BitBoard);
#endif

BitBoard EastOne(BitBoard);
BitBoard FlipDiagA1H8(BitBoard);
BitBoard FlipVertical(BitBoard);
BitBoard NoEaOne(BitBoard);
BitBoard NortOne(BitBoard);
BitBoard NoWeOne(BitBoard);
int      PopCount(BitBoard);

FORCE_INLINE Position
PopForward(BitBoard *bitBoard)
{
  Position ret = BitScanForward(*bitBoard);

  *bitBoard ^= POSBOARD(ret);

  return ret;
}

bool     PositionOccupied(BitBoard, Position);
BitBoard Rotate90AntiClockwise(BitBoard);
BitBoard Rotate90Clockwise(BitBoard);
BitBoard SoEaOne(BitBoard);
BitBoard SoutOne(BitBoard);
BitBoard SoWeOne(BitBoard);
BitBoard WestOne(BitBoard);

// castleevent.c
void             AppendCastleEvent(CastleEventSlice*, CastleEvent);
CastleEventSlice NewCastleEventSlice(void);
CastleEvent      PopCastleEvent(CastleEventSlice*);

// chessset.c
BitBoard KingThreats(ChessSet*, Side);
bool     Checked(ChessSet*, Side);
void     ChessSetPlacePiece(ChessSet*, Side, Piece, Position);
void     ChessSetRemovePiece(ChessSet*, Side, Piece, Position);
ChessSet NewChessSet(void);
ChessSet NewEmptyChessSet(void);

// game.c
void AllMoves(MoveSlice*, Game*);
bool Checkmated(Game*);
void DoCastleKingSide(Game*);
void DoCastleQueenSide(Game*);
bool ExposesCheck(Game*, BitBoard, Move*);
void InitCanSlideAttacks(void);
bool Legal(Game*, Move*);
void DoMove(Game*, Move*);
Game NewEmptyGame(bool, Side);
Game NewGame(bool, Side);
bool Stalemated(Game*);
void ToggleTurn(Game*);
void Unmove(Game*);

// king.c
BitBoard KingKingThreats(ChessSet*, Side);
BitBoard KingCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard KingMoveTargets(ChessSet*, Side, BitBoard);

// knight.c
void     InitKnight(void);
BitBoard AllKnightCaptureTargets(ChessSet*, Side);
BitBoard AllKnightMoveTargets(ChessSet*, Side);
BitBoard KnightCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard KnightKingThreats(ChessSet*, Side);
BitBoard KnightMoveTargets(ChessSet*, Side, BitBoard);

// magic.c
void InitMagics(void);

// move.c
void      AppendMove(MoveSlice*, Move);
void      AppendMoves(MoveSlice*, MoveSlice*);
MoveSlice NewMoveSlice(Move*, uint64_t);
Move      PopMove(MoveSlice*);

// movehistory.c
MoveHistory NewMoveHistory(void);

// pawn.c
BitBoard AllPawnCaptureSources(ChessSet*, Side);
BitBoard AllPawnCaptureTargets(ChessSet*, Side);
BitBoard AllPawnPushSources(ChessSet*, Side);
BitBoard AllPawnPushTargets(ChessSet*, Side);
BitBoard PawnCaptureSources(ChessSet*, Side, BitBoard);
BitBoard PawnCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard PawnKingThreats(ChessSet*, Side);
BitBoard PawnPushSources(ChessSet*, Side, BitBoard);
BitBoard PawnPushTargets(ChessSet*, Side, BitBoard);

// perft.c
PerftStats Perft(Game*, int);
uint64_t   QuickPerft(Game*, int);

// piece.c
void       AppendPiece(PieceSlice*, Piece);
PieceSlice NewPieceSlice(void);
Piece      PopPiece(PieceSlice*);

// position.c
void     InitRays(void);
BitBoard NortRay(Position);
BitBoard EastRay(Position);
BitBoard SoutRay(Position);
BitBoard WestRay(Position);
BitBoard NoEaRay(Position);
BitBoard NoWeRay(Position);
BitBoard SoEaRay(Position);
BitBoard SoWeRay(Position);

// queen.c
BitBoard AllQueenCaptureTargets(ChessSet*, Side);
BitBoard AllQueenMoveTargets(ChessSet*, Side);
BitBoard QueenKingThreats(ChessSet*, Side);
BitBoard QueenCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard QueenMoveTargets(ChessSet*, Side, BitBoard);
BitBoard QueenThreats(BitBoard, BitBoard);

// rook.c
BitBoard AllRookCaptureTargets(ChessSet*, Side);
BitBoard AllRookMoveTargets(ChessSet*, Side);
BitBoard RookCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard RookKingThreats(ChessSet*, Side);
BitBoard RookMoveTargets(ChessSet*, Side, BitBoard);
BitBoard RookSquareThreats(Position, BitBoard);
BitBoard RookThreats(BitBoard, BitBoard);

// set.c
Set   NewBlackSet(void);
Set   NewEmptySet(void);
Set   NewWhiteSet(void);
Piece PieceAt(Set*, Position);
void  SetPlacePiece(Set*, Piece, Position);
void  SetRemovePiece(Set*, Piece, Position);

// stringer.c
char  CharPiece(Piece);
char* StringBitBoard(BitBoard);
char* StringChessSet(ChessSet*);
char* StringMove(Move*);
char* StringPerft(PerftStats*);
char* StringPiece(Piece);
char* StringPosition(Position);
char* StringSide(Side);

// util.c
void* allocate(size_t, size_t);
void* allocateZero(size_t, size_t);
void  release(void*);
void  panic(char*, ...);

BitBoard (*GetMoveTargets[6])(ChessSet*, Side, BitBoard);
BitBoard (*GetCaptureTargets[6])(ChessSet*, Side, BitBoard);

// Array containing lookups determining whether any sliding attack is possible
// between the two positions.
bool CanSlideAttack[64][64];

#endif
