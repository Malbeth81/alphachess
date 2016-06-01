
#include "chessai.h"

// Public functions ------------------------------------------------------------

ChessAi::ChessAi()
{
  Color = White;
  SearchDepth = 0;
  SearchWindow = 0;
}

ChessAi::~ChessAi()
{

}

ChessAiMove* ChessAi::Execute(ChessGame* Game, int Depth, int Window)
{
  ChessAiMove* Result = NULL;
  //Board.Copy(&(Game->Board));
  Color = Game->GetActivePlayer();
  /* Get a list of all the possible moves */
  LinkedList<ChessAiMove>* Moves = CreateMoveList();
  if (Moves != NULL)
  {
    int Alpha = INT_MIN;
    int Beta = INT_MAX;
    /* Iteratively evaluate the move list */
    for (int i=2; i <= Depth; i++)
    {
      EvaluateMoveList(Moves,i,Alpha,Beta);
      Moves->Sort(MoveComparator,true);
      Alpha = Moves->Get(0)->Value-Window;
      Beta = Moves->Get(0)->Value+Window;
    }
    /* Find the best move */
    ChessAiMove* Move = FindBestMove(Moves);
    if (Move != NULL)
    {
      Result = new ChessAiMove;
      Result->From = Move->From;
      Result->To = Move->To;
      Result->Value = Move->Value;
    }
    /* Clean up */
    DeleteMoveList(Moves);
  }
  return Result;
}

// Private functions -----------------------------------------------------------

LinkedList<ChessAiMove>* ChessAi::CreateMoveList()
{
  LinkedList<ChessAiMove>* Result = new LinkedList<ChessAiMove>;
  Position From;
  Position To;
  // TODO Optimize the move list creation
  for (From.x = 0; From.x < 8; From.x++)
    for (From.y = 0; From.y < 8; From.y++)
    {
      ChessPiece* Piece = Board.GetPiece(From);
      if (Piece != NULL && Piece->Color == Color)
        for (To.x = 0; To.x < 8; To.x++)
          for (To.y = 0; To.y < 8; To.y++)
            if ((From.x != To.x || From.y != To.y) && Board.IsMoveValid(From, To))
            {
              ChessAiMove* Move = new ChessAiMove;
              Move->From = From;
              Move->To = To;
              Move->Value = 0;
              Move->Child = NULL;
              Result->Add(Move);
            }
    }
  return Result;
}

void ChessAi::DeleteMoveList(LinkedList<ChessAiMove>* Moves)
{
  if (Moves != NULL)
  {
    while (Moves->Size() > 0)
      delete Moves->Remove();
    delete Moves;
  }
}

int ChessAi::EvaluateChessBoard()
{
//  static const int IsolatedPawnsPenalty[8] = {12,14,16,20,20,16,14,12};
  int Result = 0;
  // TODO Optimize the evaluation function ...
  Position Pos;
  for (Pos.x = 0; Pos.x < 8; Pos.x++)
    for (Pos.y = 0; Pos.y < 8; Pos.y++)
    {
      int Value = 0;
      ChessPiece* Piece = Board.GetPiece(Pos);
      if (Piece != NULL)
      {
        /* Evaluate material */
        /* Piece values based on Hans Berliner's system x 100 */
        switch (Piece->Type)
        {
          case Pawn: Value += 100;
          case Knight: Value += 320;
          case Bishop: Value += 333;
          case Rook: Value += 510;
          case Queen: Value += 880;
          default: break;
        }
//        /* Evaluate mobility */
//        if (Board.Cases[i][j]->Type == Knight)
//        {
//          /* Horizontal mobility */
//          Value += min(i,7-i);
//          /* Vetical mobility */
//          Value += min(j,7-j);
//          /* Trojan horse bonus */
//          if (((Color == White && j >= 5) || (Color == Black && j <= 2)) &&
//          !IsAttacked(Board, Pos, Color))
//            Value += 10;
//        }
//        if (Board.Cases[i][j]->Type == Rook || Board.Cases[i][j]->Type == Queen)
//        {
//          /* North mobility */
//          for (int k = 1; k <= 7-j; k++)
//            if (Board.Cases[i][j+k] == NULL)
//              Value ++;
//            else
//              break;
//          /* East mobility */
//          for (int k = 1; k <= 7-i; k++)
//            if (Board.Cases[i+k][j] == NULL)
//              Value ++;
//            else
//              break;
//          /* South mobility */
//          for (int k = 1; k <= j; k++)
//            if (Board.Cases[i][j-k] == NULL)
//              Value ++;
//            else
//              break;
//          /* West mobility */
//          for (int k = 1; k <= i; k++)
//            if (Board.Cases[i-k][j] == NULL)
//              Value ++;
//            else
//              break;
//        }
//        if (Board.Cases[i][j]->Type == Bishop || Board.Cases[i][j]->Type == Queen)
//        {
//          /* North-East mobility */
//          for (int k = 1; k <= min(7-i, 7-j); k++)
//            if (Board.Cases[i+k][j+k] == NULL)
//              Value ++;
//            else
//              break;
//          /* South-East mobility */
//          for (int k = 1; k <= min(7-i, j); k++)
//            if (Board.Cases[i+k][j-k] == NULL)
//              Value ++;
//            else
//              break;
//          /* South-West mobility */
//          for (int k = 1; k <= min(i, j); k++)
//            if (Board.Cases[i-k][j-k] == NULL)
//              Value ++;
//            else
//              break;
//          /* North-West mobility */
//          for (int k = 1; k <= min(i, 7-j); k++)
//            if (Board.Cases[i-k][j+k] == NULL)
//              Value ++;
//            else
//              break;
//        }
//        /* Evaluate pawn structure */
//        if (Board.Cases[i][j]->Type == Pawn)
//        {
//          /* Isolated pawn penalty */
//          if (IsPawnIsolated(Board, Pos))
//            Value -= IsolatedPawnsPenalty[(Color == White ? j : 7-j)];
//          /* Doubled pawn penalty */
//          if (IsPawnDoubled(Board, Pos))
//            Value -= 12;
//          /* Blocked pawn penalty */
//          if (IsPawnBlocked(Board, Pos))
//            Value -= 15;
//          /* Passed pawn bonus */
//          if (IsPawnPassed(Board, Pos))
//            Value += 20;
//        }
//        /* Evaluate threats from the enemy */
//        if (IsAttacked(Board, Pos, Color))
//          Value++;
        /* Add the value to the result */
        if (Piece->Color == Color)
          Result += Value;
        else
          Result -= Value;
      }
//      else
//      {
//        /* Evaluate center control */
//        if ((Color == White && j >= 4) || (Color == Black && j < 4))
//          if (IsAttacked(Board, Pos, Color))
//            Result++;
//      }
    }
  /* Evaluate king protection, will encourage castling when it improves protection */
//  Position Pos = GetKingPosition(Board, Color);
//  for (int i = max(0,Pos.x-1); i < min(7,Pos.x+1); i++)
//    for (int j = max(0,Pos.y-1); j < min(7,Pos.y-1); j++)
//      if ((i != Pos.x || j != Pos.y) && (Board.Cases[i][j] != NULL && Board.Cases[i][j]->Color == Color))
//        Result ++;
  return Result;
}

void ChessAi::EvaluateMoveList(LinkedList<ChessAiMove>* Moves, int Depth, int Alpha, int Beta)
{
  if (Moves != NULL)
  {
    int BestValue = INT_MIN;
    /* Evaluate every move */
    ChessAiMove* Move = Moves->GetFirst();
    while (Move != NULL)
    {
      /* Alpha-Beta prunning */
      if (BestValue >= Beta)
        break;
      if (BestValue > Alpha)
        Alpha = BestValue;
      /* Evaluate the move */
      bool EnPassant = false;
      ChessPiece* Capture = Board.MovePiece(Move->From,Move->To,EnPassant);
      if (Depth == 0)
        Move->Value = EvaluateChessBoard();
      else
      {
        /* Find all possible counter moves */
        Color = !Color;
        LinkedList<ChessAiMove>* CounterMoves = CreateMoveList();
        if (CounterMoves != NULL)
        {
          /* Evaluates the counter moves */
          EvaluateMoveList(CounterMoves,Depth-1,-Beta,-Alpha);
//          Move->Child = new AIMove;
          /* Find the best counter move */
          ChessAiMove* Ptr = FindBestMove(CounterMoves);
          if (Ptr != NULL)
          {
//            *(Move->Child) = *Ptr;
            Move->Value = -Ptr->Value;
          }
          /* Clean up */
          DeleteMoveList(CounterMoves);
        }
        Color = !Color;
      }
      Board.MoveBackPiece(Move->From,Move->To,Capture);
      /* Store the best value */
      if (Move->Value > BestValue)
        BestValue = Move->Value;
      Move = Moves->GetNext();
    }
  }
}

ChessAiMove* ChessAi::FindBestMove(LinkedList<ChessAiMove>* Moves)
{
  if (Moves != NULL)
  {
    ChessAiMove* BestMove = Moves->GetFirst();
    ChessAiMove* Ptr = BestMove;
    while (Ptr != NULL)
    {
      if (Ptr->Value > BestMove->Value)
        BestMove = Ptr;
      Ptr = Moves->GetNext();
    }
    return BestMove;
  }
  return NULL;
}

int __stdcall ChessAi::MoveComparator(void* A, void* B)
{
  if (((ChessAiMove*)B)->Value < ((ChessAiMove*)A)->Value)
    return -1;
  else if (((ChessAiMove*)B)->Value > ((ChessAiMove*)A)->Value)
    return 1;
  else
    return 0;
}
