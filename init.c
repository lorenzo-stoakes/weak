#include "weak.h"

// Initialise all lookups in the engine.
void
InitEngine()
{
  GetMoveTargets[Pawn] = NULL;
  GetMoveTargets[Knight] = &KnightMoveTargets;
  GetMoveTargets[Bishop] = &BishopMoveTargets;
  GetMoveTargets[Rook] = &RookMoveTargets;
  GetMoveTargets[Queen] = &QueenMoveTargets;
  GetMoveTargets[King] = &KingMoveTargets;

  GetCaptureTargets[Pawn] = NULL;
  GetCaptureTargets[Knight] = &KnightCaptureTargets;
  GetCaptureTargets[Bishop] = &BishopCaptureTargets;
  GetCaptureTargets[Rook] = &RookCaptureTargets;
  GetCaptureTargets[Queen] = &QueenCaptureTargets;
  GetCaptureTargets[King] = &KingCaptureTargets;

  InitKnight();
  InitRays();

  // Relies on above.
  InitMagics();
  InitCanSlideAttacks();
}
