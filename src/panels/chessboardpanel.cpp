/*
* ChessBoard.cpp - A Windows control to display a chess board.
*
* Copyright (C) 2007-2011 Marc-André Lamothe.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Library General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#include "chessboardpanel.h"

const char ClassName[] = "ChessBoardPanel";

ATOM ChessBoardPanel::ClassAtom = 0;

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PUBLIC FUNCTIONS ------------------------------------------------------------

ChessBoardPanel::ChessBoardPanel(HINSTANCE hInstance, HWND hParent, RECT* R)
{
  Handle = NULL;
  SelectionCursor = LoadCursor(NULL,IDC_ARROW);
  StandardCursor = SelectionCursor;

  Game = NULL;
  Set = NULL;
  Theme = NULL;
  PossibleMoves = new LinkedList<PossibleChessMove>;

  BorderSize = MinBorderSize;
  Height = R->bottom-R->top;
  HighlightedSquare.x = -1;
  HighlightedSquare.y = -1;
  InvertView = false;
  Locked = false;
  MouseMoved = false;
  Paused = false;
  SelectedSquare.x = -1;
  SelectedSquare.y = -1;
  ShowCoordinates = true;
  ShowLastMove = true;
  ShowInvalidMoves = true;
  SquareSize = MinSquareSize;
  Width = R->right-R->left;

  if (ClassAtom == 0)
  {
    /* Register the window's class */
    WNDCLASSEX WndClass;
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = ClassName;
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = PanelWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = NULL;
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = NULL;
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  /* Create the window */
  if (ClassAtom != 0)
  {
    Handle = CreateWindowEx(0,ClassName,NULL,WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
        R->left,R->top,Width,Height,hParent,NULL,hInstance,this);
  }
}

ChessBoardPanel::~ChessBoardPanel()
{
  /* Destroy the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
}

void ChessBoardPanel::ClearPossibleMoves()
{
  while (PossibleMoves->Size() > 0)
    delete PossibleMoves->Remove();
}

void ChessBoardPanel::DisplayPossibleMove(PossibleChessMove* Move)
{
  if (Move != NULL)
  {
    PossibleMoves->Add(Move);
    Invalidate();
  }
}

ChessSet* ChessBoardPanel::GetChessSet()
{
  return Set;
}

HWND ChessBoardPanel::GetHandle()
{
  return Handle;
}

bool ChessBoardPanel::GetInvertView()
{
  return InvertView;
}

bool ChessBoardPanel::GetShowCoordinates()
{
  return ShowCoordinates;
}

bool ChessBoardPanel::GetShowLastMove()
{
  return ShowLastMove;
}

bool ChessBoardPanel::GetShowInvalidMoves()
{
  return ShowInvalidMoves;
}

int ChessBoardPanel::GetSquareSize()
{
  return SquareSize;
}

HCURSOR ChessBoardPanel::GetStandardCursor()
{
  return StandardCursor;
}

HCURSOR ChessBoardPanel::GetSelectionCursor()
{
  return SelectionCursor;
}

const ChessBoardTheme* ChessBoardPanel::GetTheme()
{
  return Theme;
}

void ChessBoardPanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Handle, NULL, FALSE);
}

bool ChessBoardPanel::IsLocked()
{
  return Locked;
}

bool ChessBoardPanel::IsPaused()
{
  return Paused;
}

void ChessBoardPanel::SetChessSet(ChessSet* NewSet)
{
  Set = NewSet;
  Invalidate();
}

void ChessBoardPanel::SetGame(ChessGame* NewGame)
{
  Game = NewGame;
  Invalidate();
}

void ChessBoardPanel::SetInvertView(bool Value)
{
  InvertView = Value;
  HighlightedSquare.x = 7-HighlightedSquare.x;
  HighlightedSquare.y = 7-HighlightedSquare.y;
  Invalidate();
}

void ChessBoardPanel::SetLocked(bool Value)
{
  Locked = Value;
  if (Locked && SelectedSquare.x >= 0 && SelectedSquare.x < 8 && SelectedSquare.y >= 0 && SelectedSquare.y < 8)
  {
    /* Cancel piece selection */
    SelectedSquare.x = -1;
    SelectedSquare.y = -1;
    SetCursor(StandardCursor);
    ReleaseCapture();
    Invalidate();
  }
}

void ChessBoardPanel::SetPaused(bool Value)
{
  Paused = Value;
  Invalidate();
  SetLocked(Value);
}

void ChessBoardPanel::SetShowCoordinates(bool Value)
{
  ShowCoordinates = Value;
  Invalidate();
}

void ChessBoardPanel::SetShowLastMove(bool Value)
{
  ShowLastMove = Value;
  Invalidate();
}

void ChessBoardPanel::SetShowInvalidMoves(bool Value)
{
  ShowInvalidMoves = Value;
  Invalidate();
}

void ChessBoardPanel::SetStandardCursor(HCURSOR Cursor)
{
  StandardCursor = Cursor;
}

void ChessBoardPanel::SetSelectionCursor(HCURSOR Cursor)
{
  SelectionCursor = Cursor;
}

void ChessBoardPanel::SetTheme(ChessBoardTheme* NewTheme)
{
  Theme = NewTheme;
  Invalidate();
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void ChessBoardPanel::DrawBoardIndexes(HDC DC, int X, int Y, bool Vertical)
{
  if (Theme != NULL)
  {
    char VerticalIndexes[] = "12345678";
    char HorizontalIndexes[] = "ABCDEFGH";
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, Theme->CoordinatesColor);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,PixelsToPoints(DC, BorderSize-4),0));
    for (int i=0; i < 8; i++)
    {
      if (Vertical)
      {
        /* Draw the vertical indexes */
        SIZE Size;
        char* Str = new char[2];
        Str[0] = VerticalIndexes[i];
        Str[1] = '\0';
        GetTextExtentPoint32(DC,Str,strlen(Str),&Size);
        TextOut(DC,X+(InvertView ? SquareSize*8+(int)(Size.cx*1.5) : 0-Size.cx/2),Y+SquareSize*(InvertView ? i : 7-i)-Size.cy/2,Str,strlen(Str));
        delete[] Str;
      }
      else
      {
        /* Draw the horizontal indexes */
        SIZE Size;
        char* Str = new char[2];
        Str[0] = HorizontalIndexes[i];
        Str[1] = '\0';
        GetTextExtentPoint32(DC,Str,strlen(Str),&Size);
        TextOut(DC,X+SquareSize*(InvertView ? 7-i : i)-Size.cx/2,Y+(InvertView ? 0-Size.cy/2 : SquareSize*8+Size.cy/2),Str,strlen(Str));
        delete[] Str;
      }
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

void ChessBoardPanel::DrawBoardSquares(HDC DC, int X, int Y)
{
  if (Game != NULL && Theme != NULL)
  {
    COLORREF Color;
    int HighlightWidth = SquareSize/20;
    const ChessMove* LastMove = Game->GetMoves()->Get(Game->GetDisplayedMove());
    /* Draw the board squares */
    Position Pos;
    for (Pos.x=0; Pos.x < 8; Pos.x++)
    {
      for (Pos.y=0; Pos.y < 8; Pos.y++)
      {
        RECT R = {X+SquareSize*(InvertView ? 7-Pos.x : Pos.x),Y+SquareSize*(InvertView ? Pos.y : 7-Pos.y),X+SquareSize*(InvertView ? 8-Pos.x : Pos.x+1),Y+SquareSize*(InvertView ? Pos.y+1 : 8-Pos.y)};
        /* Calculate the color for the square */
        if (LastMove != NULL && ShowLastMove && LastMove->To.x == Pos.x && LastMove->To.y == Pos.y)
          Color = Theme->LastMovedColor;
        else if ((Pos.x-Pos.y)%2 == 0)
          Color = Theme->BlackSquaresColor;
        else
          Color = Theme->WhiteSquaresColor;
        /* Draw the square */
        HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(Color));
        HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,Color));
        Rectangle(DC,R.left,R.top,R.right,R.bottom);
        DeleteObject(SelectObject(DC,OldPen));
        DeleteObject(SelectObject(DC,OldBrush));
        /* Calculate the color for the highlight */
        if (SelectedSquare.x == Pos.x && SelectedSquare.y == Pos.y)
          Color = Theme->SelectedSquareColor;
        else if (HighlightedSquare.x == Pos.x && HighlightedSquare.y == Pos.y)
        {
          if (ShowInvalidMoves && SelectedSquare.x >= 0 && SelectedSquare.y >= 0)
          {
            if (Game->IsMoveValid(SelectedSquare, HighlightedSquare))
              Color = Theme->ValidMoveColor;
            else
              Color = Theme->InvalidMoveColor;
          }
          else
            Color = Theme->CurrentSquareColor;
        }
        else
        {
          PossibleChessMove* BestMove = FindBestPossibleMove(PossibleMoves);
          if (BestMove != NULL && Pos.x == BestMove->From.x && Pos.y == BestMove->From.y)
            Color = Theme->CoordinatesColor;
        }
        /* Draw the highlight */
        InflateRect(&R,-HighlightWidth/2,-HighlightWidth/2);
        if (HighlightWidth % 2 == 0)
        {
          R.right++;
          R.bottom++;
        }
        OldBrush = (HBRUSH)SelectObject(DC,CreateHollowBrush());
        OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,HighlightWidth,Color));
        Rectangle(DC,R.left,R.top,R.right,R.bottom);
        DeleteObject(SelectObject(DC,OldPen));
        DeleteObject(SelectObject(DC,OldBrush));
      }
    }
  }
}

void ChessBoardPanel::DrawChessPieces(HDC DC, int X, int Y)
{
  if (Game != NULL && Set != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName.c_str(),PixelsToPoints(DC, SquareSize-4),0));
    /* Draw the chess piece */
    for (int i=0; i < 8; i++)
      for (int j=0; j < 8; j++)
        if (SelectedSquare.x != i || SelectedSquare.y != j)
          DrawChessPiece(DC,X+SquareSize*(InvertView ? 7-i : i),Y+SquareSize*(InvertView ? j : 7-j),Game->GetPiece(Position(i,j)));
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

void ChessBoardPanel::DrawChessPiece(HDC DC, int X, int Y, const ChessPiece* Piece)
{
  if (Theme != NULL && Set != NULL && Piece != NULL)
  {
    /* Obtain the letter to use */
    int Index;
    switch (Piece->Type)
    {
      case King: Index = 0; break;
      case Queen: Index = 1; break;
      case Rook: Index = 2; break;
      case Bishop: Index = 3; break;
      case Knight: Index = 4; break;
      default: Index = 5; break;
    }
    /* Draw the piece */
    SIZE Size;
    GetTextExtentPoint32(DC,Set->Letters.substr(Index+6,1).c_str(),1,&Size);
    if ((Piece->Color == White && Theme->WhitePiecesStyle != Outline) || (Piece->Color == Black && Theme->BlackPiecesStyle != Outline))
    {
      SetTextColor(DC, (Piece->Color == White ? Theme->WhitePiecesColor : Theme->BlackPiecesColor));
      TextOut(DC,X-Size.cx/2,Y-Size.cy/2,Set->Letters.substr(Index+6,1).c_str(),1);
    }
    /* Draw an outline */
    if ((Piece->Color == White && Theme->WhitePiecesStyle != Plain) || (Piece->Color == Black && Theme->BlackPiecesStyle != Plain))
    {
      SetTextColor(DC, (Piece->Color == White ? Theme->WhitePiecesOutlineColor : Theme->BlackPiecesOutlineColor));
      TextOut(DC,X-Size.cx/2,Y-Size.cy/2,Set->Letters.substr(Index,1).c_str(),1);
    }
  }
}

void ChessBoardPanel::DrawPaused(HDC DC)
{
  if (Theme != NULL)
  {
    char Str[] = "Paused";
    SIZE Size;
    SetBkMode(DC,TRANSPARENT);
    /* Draw the text outline */
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,PixelsToPoints(DC, SquareSize-4),0));
    HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,int(SquareSize*.1),Theme->BorderColor));
    GetTextExtentPoint32(DC,Str,strlen(Str),&Size);
    BeginPath(DC);
    TextOut(DC,(min(Width,Height)-Size.cx)/2,(min(Width,Height)-Size.cy)/2,Str,strlen(Str));
    EndPath(DC);
    StrokePath(DC);
    /* Draw the text */
    SetTextColor(DC, Theme->CoordinatesColor);
    DeleteObject(SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,PixelsToPoints(DC, SquareSize-4),0)));
    GetTextExtentPoint32(DC,Str,strlen(Str),&Size);
    TextOut(DC,(min(Width,Height)-Size.cx)/2,(min(Width,Height)-Size.cy)/2,Str,strlen(Str));
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
    DeleteObject(SelectObject(DC,OldPen));
  }
}

void ChessBoardPanel::DrawPossibleMoves(HDC DC, int X, int Y)
{
  if (Game != NULL && Set != NULL && PossibleMoves != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,int(SquareSize*.1),Theme->BorderColor));
    /* Draw the chess move */
    PossibleChessMove* Move = PossibleMoves->GetFirst();
    while (Move != NULL)
    {
      MoveToEx(DC, X+SquareSize*(InvertView ? 7-Move->From.x : Move->From.x), Y+SquareSize*(InvertView ? Move->From.y : 7-Move->From.y), NULL);
      LineTo(DC, X+SquareSize*(InvertView ? 7-Move->To.x : Move->To.x), Y+SquareSize*(InvertView ? Move->To.y : 7-Move->To.y));
      Move = PossibleMoves->GetNext();
    }
    DeleteObject(SelectObject(DC,OldPen));
    OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,int(SquareSize*.01),Theme->CoordinatesColor));
    if (SelectedSquare.x >= 0 && SelectedSquare.y >= 0)
    {
      /* Highlight the selected piece's moves */
      PossibleChessMove* Move = PossibleMoves->GetFirst();
      while (Move != NULL)
      {
        if (Move->From.x == SelectedSquare.x && Move->From.y == SelectedSquare.y)
        {
          MoveToEx(DC, X+SquareSize*(InvertView ? 7-Move->From.x : Move->From.x), Y+SquareSize*(InvertView ? Move->From.y : 7-Move->From.y), NULL);
          LineTo(DC, X+SquareSize*(InvertView ? 7-Move->To.x : Move->To.x), Y+SquareSize*(InvertView ? Move->To.y : 7-Move->To.y));
        }
        Move = PossibleMoves->GetNext();
      }
    }
    else
    {
      /* Highlight the best chess moves */
      for (int i=0; i < 8; i++)
        for (int j=0; j < 8; j++)
        {
          PossibleChessMove* BestMove = FindBestPossibleMove(PossibleMoves, i, j);
          if (BestMove != NULL)
          {
            MoveToEx(DC, X+SquareSize*(InvertView ? 7-BestMove->From.x : BestMove->From.x), Y+SquareSize*(InvertView ? BestMove->From.y : 7-BestMove->From.y), NULL);
            LineTo(DC, X+SquareSize*(InvertView ? 7-BestMove->To.x : BestMove->To.x), Y+SquareSize*(InvertView ? BestMove->To.y : 7-BestMove->To.y));
          }
        }
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldPen));
  }
}

void ChessBoardPanel::DrawPossibleMovesValues(HDC DC, int X, int Y)
{
  if (Game != NULL && Set != NULL && PossibleMoves != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, Theme->CoordinatesColor);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,PixelsToPoints(DC, BorderSize-4),0));
    HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,int(SquareSize*.1),Theme->BorderColor));
    if (SelectedSquare.x >= 0 && SelectedSquare.y >= 0)
    {
      /* Draw the selected piece's move value */
      PossibleChessMove* Move = PossibleMoves->GetFirst();
      while (Move != NULL)
      {
        if (Move->From.x == SelectedSquare.x && Move->From.y == SelectedSquare.y)
        {
          char* Str = inttostr(Move->Value-Game->GetBoardValue(Game->GetActivePlayer()));
          BeginPath(DC);
          TextOut(DC, X+2+SquareSize*(InvertView ? 7-Move->To.x : Move->To.x), Y+SquareSize*(InvertView ? Move->To.y : 7-Move->To.y),Str,strlen(Str));
          EndPath(DC);
          StrokePath(DC);
          TextOut(DC, X+2+SquareSize*(InvertView ? 7-Move->To.x : Move->To.x), Y+SquareSize*(InvertView ? Move->To.y : 7-Move->To.y),Str,strlen(Str));
          delete[] Str;
        }
        Move = PossibleMoves->GetNext();
      }
    }
    else
    {
      /* Draw the best chess move's value */
      for (int i=0; i < 8; i++)
        for (int j=0; j < 8; j++)
        {
          PossibleChessMove* BestMove = FindBestPossibleMove(PossibleMoves, i, j);
          if (BestMove != NULL)
          {
            char* Str = inttostr(BestMove->Value-Game->GetBoardValue(Game->GetActivePlayer()));
            BeginPath(DC);
            TextOut(DC, X+2+SquareSize*(InvertView ? 7-i : i), Y+SquareSize*(InvertView ? j : 7-j),Str,strlen(Str));
            EndPath(DC);
            StrokePath(DC);
            TextOut(DC, X+2+SquareSize*(InvertView ? 7-i : i), Y+SquareSize*(InvertView ? j : 7-j),Str,strlen(Str));
            delete[] Str;
          }
        }
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
    DeleteObject(SelectObject(DC,OldPen));
  }
}

void ChessBoardPanel::DrawSelectedChessPiece(HDC DC, int X, int Y)
{
  if (Game != NULL && Set != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName.c_str(),PixelsToPoints(DC, SquareSize-4),0));
    /* Draw the selected piece */
    if (SelectedSquare.x >= 0 && SelectedSquare.y >= 0)
    {
      POINT Point;
      GetCursorPos(&Point);
      RECT Rect;
      GetWindowRect(Handle, &Rect);
      DrawChessPiece(DC,Point.x-Rect.left,Point.y-Rect.top,Game->GetPiece(SelectedSquare));
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void ChessBoardPanel::MouseDown(int x, int y)
{
  if (Game != NULL && Game->GetState() == Started && !Locked)
  {
    if (HighlightedSquare.x >= 0 && HighlightedSquare.x < 8 && HighlightedSquare.y >= 0 && HighlightedSquare.y < 8)
      if (SelectedSquare.x == -1 && SelectedSquare.y == -1 && Game->GetPiece(HighlightedSquare) != NULL && Game->GetPiece(HighlightedSquare)->Color == Game->GetActivePlayer())
      {
        /* Select a piece on the board */
        SelectedSquare = HighlightedSquare;
        SetCursor(SelectionCursor);
        SetCapture(Handle);
        MouseMoved = false;
      }
    Invalidate();
  }
}

void ChessBoardPanel::MouseUp(int x, int y)
{
  if (Game != NULL && !Locked)
  {
    bool PieceMoved = true; /* Prevent from updating the board if the piece was not moved */
    if (HighlightedSquare.x >= 0 && HighlightedSquare.x < 8 && HighlightedSquare.y >= 0 && HighlightedSquare.y < 8)
      if (SelectedSquare.x >= 0 && SelectedSquare.x < 8 && SelectedSquare.y >= 0 && SelectedSquare.y < 8)
        if (HighlightedSquare.x != SelectedSquare.x || HighlightedSquare.y != SelectedSquare.y)
          /* Move the selected piece to the highlighted square */
          PieceMoved = SendMessage(GetParent(Handle), WM_CHESSPIECEMOVED, (WPARAM)new Position(SelectedSquare), (LPARAM)new Position(HighlightedSquare));
    if (MouseMoved)
    {
      if (PieceMoved)
      {
        /* Cancel the selection */
        SelectedSquare.x = -1;
        SelectedSquare.y = -1;
        SetCursor(StandardCursor);
        ReleaseCapture();
        Invalidate();
      }
    }
    else
      MouseMoved = true; /* Allows to cancel the selection when clicking a piece */
  }
}

void ChessBoardPanel::MouseMove(int x, int y)
{
  if (x >= BorderSize && x < BorderSize+SquareSize*8 && y >= BorderSize && y < BorderSize+SquareSize*8)
  {
    int i = min((x-BorderSize)/SquareSize,7);
    int j = min((y-BorderSize)/SquareSize,7);
    HighlightedSquare.x = (InvertView ? 7-i : i);
    HighlightedSquare.y = (InvertView ? j : 7-j);
  }
  else
  {
    HighlightedSquare.x = -1;
    HighlightedSquare.y = -1;
  }
  MouseMoved = true; /* Allows to cancel the selection when dragging a piece */
  Invalidate();
  if (SelectedSquare.x >= 0 && SelectedSquare.y >= 0)
    SetCursor(SelectionCursor);
  else if (HighlightedSquare.x >= 0 && HighlightedSquare.y >= 0)
    SetCursor(StandardCursor);
  else if (Handle != NULL)
    SetCursor((HCURSOR)GetClassLong(GetParent(Handle),GCL_HCURSOR));
}

void ChessBoardPanel::Paint()
{
  if (Handle != NULL && GetUpdateRect(Handle, NULL, 0) != 0)
  {
    PAINTSTRUCT PS;
    HDC DC = BeginPaint(Handle, &PS);
    if (DC != NULL)
    {
      /* Create a buffer DC to draw on */
      HDC Buffer = CreateCompatibleDC(DC);
      HBITMAP Bitmap = CreateCompatibleBitmap(DC,Width,Height);
      SelectObject(Buffer, Bitmap);
      /* Erase the buffer */
      HBRUSH OldBrush = (HBRUSH)SelectObject(Buffer,CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));
      HPEN OldPen = (HPEN)SelectObject(Buffer,CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNFACE)));
      Rectangle(Buffer,0,0,Width,Height);
      DeleteObject(SelectObject(Buffer, OldPen));
      DeleteObject(SelectObject(Buffer, OldBrush));
      /* Draw the board border */
      if (Theme != NULL)
      {
        OldPen = (HPEN)SelectObject(Buffer,CreatePen(PS_SOLID,BorderSize,Theme->BorderColor));
        Rectangle(Buffer,BorderSize/2,BorderSize/2,SquareSize*8+(int)(BorderSize*1.5)+1,SquareSize*8+(int)(BorderSize*1.5)+1);
        DeleteObject(SelectObject(Buffer,OldPen));
      }
      /* Draw the indexes */
      if (ShowCoordinates)
      {
        DrawBoardIndexes(Buffer,BorderSize/2,BorderSize+SquareSize/2,true); /* Vertical */
        DrawBoardIndexes(Buffer,BorderSize+SquareSize/2,BorderSize/2,false); /* Horizontal */
      }
      /* Draw the board squares */
      DrawBoardSquares(Buffer, BorderSize, BorderSize);
      /* Draw the possible moves */
      DrawPossibleMoves(Buffer, BorderSize+SquareSize/2, BorderSize+SquareSize/2);
      /* Draw the chess pieces */
      DrawChessPieces(Buffer, BorderSize+SquareSize/2, BorderSize+SquareSize/2);
      /* Draw the possible moves values */
      DrawPossibleMovesValues(Buffer, BorderSize, BorderSize);
      /* Draw the selected chess piece */
      DrawSelectedChessPiece(Buffer, BorderSize+SquareSize/2, BorderSize+SquareSize/2);
      if (Paused)
        DrawPaused(Buffer);
      /* Copy the buffer into the destination DC */
      BitBlt(DC,0,0,Width,Height,Buffer,0,0,SRCCOPY);
      /* Cleanup */
      DeleteDC(Buffer);
      DeleteObject(Bitmap);
    }
    EndPaint(Handle, &PS);
  }
}

void ChessBoardPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Calculate the board size */
  int MinSize = min(Width,Height);
  SquareSize = (int)max(MinSquareSize,(MinSize-(int)max(MinBorderSize,MinSize*.04)*2)/8);
  BorderSize = (int)(MinSize-SquareSize*8)/2;
  Invalidate();
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall ChessBoardPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      ChessBoardPanel* Panel = (ChessBoardPanel*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
      return 0;
    }
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->MouseDown((int)LOWORD(lParam), (int)HIWORD(lParam));
      }
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->MouseUp((int)LOWORD(lParam), (int)HIWORD(lParam));
      }
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->MouseMove((int)LOWORD(lParam), (int)HIWORD(lParam));
      }
      return 0;
    }
    case WM_PAINT:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        Panel->Paint();
      }
      return 0;
    }
    case WM_SIZE:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        if (LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
            Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
