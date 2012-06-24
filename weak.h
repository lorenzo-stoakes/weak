#ifndef WEAK_HEADER
#define WEAK_HEADER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// See http://chessprogramming.wikispaces.com/Bitboards.
#define C64(constantU64) constantU64##ULL
#define RANK(pos) ((Rank)(pos/8))
#define FILE(pos) ((File)(pos%8))
#define OPPOSITE(side) ((Side)(1-side))
#define POSBOARD(pos) ((BitBoard)(1ULL<<pos))
#define POSITION(rank, file) ((Position)(rank*8 + file))

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
typedef struct Positions        Positions;
typedef enum Rank               Rank;
typedef enum File               File;
typedef struct Set              Set;
typedef enum Side               Side;


struct CastleEventSlice {
  int Len, Cap;
  CastleEvent *Vals;
};

struct MoveSlice {
  int Len, Cap;
  Move *Vals;
};

struct PieceSlice {
  int Len, Cap;
  Piece *Vals;
};

struct Set {
  BitBoard Pawns, Rooks, Knights, Bishops, Queens, King;
};

struct MoveHistory {
  CastleEventSlice CastleEvents;
  MoveSlice        Moves;
  PieceSlice       CapturedPieces;
};

struct ChessSet {
  Set White, Black;
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

struct Positions {
  Position *Vals;
  int      Len;
};

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

// bishop.c
BitBoard AllBishopCaptureTargets(ChessSet*, Side);
BitBoard AllBishopMoveTargets(ChessSet*, Side);
BitBoard AllBishopThreats(ChessSet*, Side);
BitBoard BishopCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard BishopMoveTargets(ChessSet*, Side, BitBoard);

// bitboard.c
Position  BitScanBackward(BitBoard);
Position  BitScanForward(BitBoard);
Positions BoardPositions(BitBoard);
BitBoard  EastOne(BitBoard);
BitBoard  FlipDiagA1H8(BitBoard);
BitBoard  FlipVertical(BitBoard);
BitBoard  NoEaOne(BitBoard);
BitBoard  NortOne(BitBoard);
BitBoard  NoWeOne(BitBoard);
int       PopCount(BitBoard);
bool      PositionOccupied(BitBoard, Position);
BitBoard  Rotate90AntiClockwise(BitBoard);
BitBoard  Rotate90Clockwise(BitBoard);
BitBoard  SoEaOne(BitBoard);
BitBoard  SoutOne(BitBoard);
BitBoard  SoWeOne(BitBoard);
BitBoard  WestOne(BitBoard);

// castleevent.c
CastleEventSlice AppendCastleEvent(CastleEventSlice, CastleEvent);
CastleEventSlice NewCastleEventSlice(void);
CastleEvent      PopCastleEvent(CastleEventSlice*);

// chessset.c
BitBoard AllThreats(ChessSet*, Side);
bool     Checked(ChessSet*, Side);
void     ChessSetPlacePiece(ChessSet*, Side, Piece, Position);
void     ChessSetRemovePiece(ChessSet*, Side, Piece, Position);
BitBoard ChessSetOccupancy(ChessSet*);
Piece    ChessSetPieceAt(ChessSet*, Side, Position);
BitBoard EmptySquares(ChessSet*);
ChessSet NewChessSet(void);

// game.c
MoveSlice AllMoves(Game*);
bool      Checkmated(Game*);
void      DoCastleKingSide(Game*);
void      DoCastleQueenSide(Game*);
bool      Legal(Game*, Move*);
void      DoMove(Game*, Move*);
Game      NewGame(bool, Side);
bool      Stalemated(Game*);
void      ToggleTurn(Game *game);
void      Unmove(Game*);

// knight.c
void      InitKnight(void);
BitBoard  AllKnightCaptureTargets(ChessSet*, Side);
BitBoard  AllKnightMoveTargets(ChessSet*, Side);
BitBoard  AllKnightThreats(ChessSet*, Side);
BitBoard  KnightCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard  KnightMoveTargets(ChessSet*, Side, BitBoard);

// move.c
MoveSlice AppendMove(MoveSlice, Move);
MoveSlice NewMoveSlice(void);
Move      PopMove(MoveSlice*);

// movehistory.c
MoveHistory NewMoveHistory(void);

// pawn.c
BitBoard AllPawnCaptureSources(ChessSet*, Side);
BitBoard AllPawnCaptureTargets(ChessSet*, Side);
BitBoard AllPawnPushSources(ChessSet*, Side);
BitBoard AllPawnPushTargets(ChessSet*, Side);
BitBoard AllPawnThreats(ChessSet*, Side);
BitBoard PawnCaptureSources(ChessSet*, Side, BitBoard);
BitBoard PawnCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard PawnPushSources(ChessSet*, Side, BitBoard);
BitBoard PawnPushTargets(ChessSet*, Side, BitBoard);

// perft.c
PerftStats Perft(Game*, int);

// piece.c
PieceSlice AppendPiece(PieceSlice, Piece);
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
BitBoard AllQueenThreats(ChessSet*, Side);
BitBoard QueenCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard QueenMoveTargets(ChessSet*, Side, BitBoard);

// rook.c
BitBoard AllRookCaptureTargets(ChessSet*, Side);
BitBoard AllRookMoveTargets(ChessSet*, Side);
BitBoard AllRookThreats(ChessSet*, Side);
BitBoard RookCaptureTargets(ChessSet*, Side, BitBoard);
BitBoard RookMoveTargets(ChessSet*, Side, BitBoard);

// set.c
Set      NewBlackSet(void);
Set      NewWhiteSet(void);
BitBoard SetOccupancy(Set*);
Piece    SetPieceAt(Set*, Position);
void     SetPlacePiece(Set*, Piece, Position);
void     SetRemovePiece(Set*, Piece, Position);

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
void* allocate(size_t);
void  release(void*);
void  panic(char*, ...);

#endif
