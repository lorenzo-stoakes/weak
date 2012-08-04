 #include <stdio.h>
 #include <strings.h>
 #include <ctype.h>
 #include "weak.h"

 char
 CharPiece(Piece piece)
 {
   switch(piece) {
   case Pawn:
     return 'P';
   case Rook:
     return 'R';
   case Knight:
     return 'N';
   case Bishop:
     return 'B';
   case Queen:
     return 'Q';
   case King:
     return 'K';
   default:
     return '?';
   }
 }

 char*
 StringBitBoard(BitBoard bitBoard)
 {
   Rank rank;
   File file;
   Position pos;
   int newline, ind;

   char ret[64+8+1];

   for(pos = A1; pos <= H8; pos++) {
     rank = Rank8 - RANK(pos);
     file = FILE(pos);
     newline = rank;
     ind = 8*rank + file + newline;

     if(file == 7) {
         ret[ind+1] = '\n';
     }

     if((POSBOARD(pos)&bitBoard) == 0) {
       ret[ind] = '.';
     } else {
       ret[ind] = '1';
     }
   }
   ret[64+8] = '\0';

   return strdup(ret);
 }

 // ASCII-art representation of chessboard.
 char*
 StringChessSet(ChessSet *chessSet)
 {
   // Include space for newlines.
   char ret[64+8+1];
   char pieceChr;
   int ind, newline;
   File file;
   Piece piece;
   Position pos;
   Rank rank;
   Side side;

   // Outputs chess set as ascii-art.
   // pieces are upper-case if white, lower-case if black.
   // P=pawn, R=rook, N=knight, B=bishop, Q=queen, K=king, .=empty square.

   // e.g., the initial position is output as follows:-

   // rnbqkbnr
   // pppppppp
   // ........
   // ........
   // ........
   // ........
   // PPPPPPPP
   // RNBQKBNR

   for(pos = A1; pos <= H8; pos++) {
     // Vertical flip.
     rank = Rank8 - RANK(pos);
     file = FILE(pos);

     // We need to leave space for a newline after each rank. This is equal to the
     // number of ranks which will appear before this one in the ASCII board,
     // e.g. the vertically flipped rank.
     newline = rank;
     ind = 8*rank + file + newline;

     if(file == 7) {
       ret[ind+1] = '\n';
     }

     piece = PieceAt(chessSet, pos);
     if(piece == MissingPiece) {
       ret[ind] = '.';
     } else {
       if((chessSet->Sets[White].Occupancy & POSBOARD(pos)) != EmptyBoard) {
         side = White;
       } else {
         side = Black;
       }

       pieceChr = CharPiece(piece);

       switch(side) {
       case White:
         ret[ind] = pieceChr;
         break;
       case Black:
         ret[ind] = tolower(pieceChr);
         break;
       default:
         panic("Impossible.");
       }
     }
   }

   ret[64+8] = '\0';

   for(pos = A1; pos <= H8; pos++) {
     if(ret[pos] == '\0') {
         ret[pos] = '?';
     }
   }

   return strdup(ret);
 }

 // String move in long algebraic form.
 char*
 StringMove(Move move, Piece piece, bool capture)
 {
   char actionChr, pieceChr;
   char *suffix, *from, *to;
   char ret[1+2+1+2+2+1];

   from = StringPosition(FROM(move));
   to = StringPosition(TO(move));

   switch(TYPE(move)) {
   default:
     suffix = "??";
     break;
   case CastleQueenSide:
     return strdup("O-O-O");
   case CastleKingSide:
     return strdup("O-O");
   case EnPassant:
     suffix = "ep";
     break;
   case PromoteKnight:
     suffix = "=N";
     break;
   case PromoteBishop:
     suffix = "=B";
     break;
   case PromoteRook:
     suffix = "=R";
     break;
   case PromoteQueen:
     suffix = "=Q";
     break;
   case Normal:
     suffix = "";
     break;
   }

   if(capture) {
     actionChr = 'x';
   } else {
     actionChr = '-';
   }

   pieceChr = CharPiece(piece);

   if(pieceChr == 'P' || pieceChr == 'p') {
     sprintf(ret, "%s%c%s%s", from, actionChr, to, suffix);
   } else {
     sprintf(ret, "%c%s%c%s%s", pieceChr, from, actionChr, to, suffix);
   }

   return strdup(ret);
 }

 char*
 StringMoveHistory(MoveHistory *history)
 {
   Move move;
   Move *curr;
   Side side = White;
   StringBuilder builder = NewStringBuilder();
   int fullMoveCount = 1;

   // TODO: Determine positions + capture condition for moves.

   for(curr = history->Moves.Vals; curr != history->Moves.Curr; curr++) {
     move = *curr;

     switch(side) {
     case White:
       AppendString(&builder, "%d. ?%s",
                    fullMoveCount, StringMove(move, Pawn, false));
       break;
     case Black:
       AppendString(&builder, " ?%s\n",
                    StringMove(move, Pawn, false));

       fullMoveCount++;
       
       break;
     }

     side = OPPOSITE(side);
   }

   return builder.Length == 0 ? NULL : BuildString(&builder, true);
}

char*
StringPerft(PerftStats *s)
{
  char ret[1000];

  sprintf(ret, ""
          "Count:       %llu\n"
          "Captures:    %llu\n"
          "En Passants: %llu\n"
          "Castles:     %llu\n"
          "Promotions:  %llu\n"
          "Checks:      %llu\n"
          "Checkmates:  %llu\n",
          s->Count, s->Captures, s->EnPassants, s->Castles, s->Promotions, s->Checks,
          s->Checkmates);

  return strdup(ret);
}

char*
StringPiece(Piece piece)
{
  char *ret;

  switch(piece) {
  case Pawn:
    ret = "pawn";
    break;
  case Rook:
    ret = "rook";
    break;
  case Knight:
    ret = "knight";
    break;
  case Bishop:
    ret = "bishop";
    break;
  case Queen:
    ret = "queen";
    break;
  case King:
    ret = "king";
    break;
  case MissingPiece:
    ret = "#missing piece";
    break;    
  default:
    ret = "#invalid piece";
    break;
  } 

  return strdup(ret);
}

char*
StringPosition(Position pos)
{
  char ret[2+1];

  // Unsigned so we don't need to check < 0.
  if(pos > 63) {
    return strdup("#invalid position");
  }

  if(sprintf(ret, "%c%d", 'a'+FILE(pos), RANK(pos)+1) != 2) {
    panic("Couldn't string position");
  }

  ret[2] = '\0';

  return strdup(ret);
}

char*
StringSide(Side side)
{
  char *ret;

  switch(side) {
  case White:
    ret = "white";
    break;
  case Black:
    ret = "black";
    break;
  default:
    ret = "#invalid side";
    break;
  }

  return strdup(ret);
}
