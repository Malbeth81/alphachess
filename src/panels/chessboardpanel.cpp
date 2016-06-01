/*
* ChessBoard.cpp - A Windows control to display a chess board.
*
* Copyright (C) 2007-2010 Marc-André Lamothe.
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

extern ChessEngine Chess;

extern char DefaultFontName[];

ATOM ChessBoardPanel::ClassAtom = 0;

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PUBLIC FUNCTIONS ------------------------------------------------------------

ChessBoardPanel::ChessBoardPanel(HWND hParent, RECT* R)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));

  MovePieceProc = NULL;

  Handle = NULL;
  Set = NULL;
  SelectionCursor = LoadCursor(NULL,IDC_ARROW);
  StandardCursor = SelectionCursor;
  Theme = NULL;

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
    WndClass.hInstance = Instance;
    WndClass.lpfnWndProc = PanelWindowProc;
    WndClass.style = 0;
    WndClass.hbrBackground = NULL;
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = NULL;
    WndClass.lpszMenuName = 0;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  /* Create the window */
  if (ClassAtom != 0)
    CreateWindowEx(0,ClassName,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    R->left,R->top,R->right-R->left,R->bottom-R->top,hParent,NULL,Instance,this);
}

ChessBoardPanel::~ChessBoardPanel()
{
  /* Destroy the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
}

HWND ChessBoardPanel::GetHandle()
{
  return Handle;
}

void ChessBoardPanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Handle, NULL, FALSE);
}

// PUBLIC ACCESS FUNCTIONS -----------------------------------------------------

ChessSet* ChessBoardPanel::GetChessSet()
{
  return Set;
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

// PUBLIC MUTATOR FUNCTIONS ----------------------------------------------------

void ChessBoardPanel::SetChessSet(ChessSet* NewSet)
{
  Set = NewSet;
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
  SetLocked(Value);
  Paused = Value;
  Invalidate();
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
    const char* Indexes = (Vertical ? (InvertView ? "12345678" : "87654321") : (InvertView ? "HGFEDCBA" : "ABCDEFGH"));
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, Theme->CoordinatesColor);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultFontName,max(int(SquareSize*.20),7),0));
    for (int i=0; i < 8; i++)
    {
      /* Draw the index */
      SIZE Size;
      GetTextExtentPoint32(DC,Indexes,1,&Size);
      if (Vertical)
        TextOut(DC,X-Size.cx/2,Y+(i)*SquareSize-Size.cy/2,Indexes,1);
      else
        TextOut(DC,X+(i)*SquareSize-Size.cx/2,Y-Size.cy/2,Indexes,1);
      Indexes++;
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

void ChessBoardPanel::DrawBoardSquares(HDC DC, int X, int Y)
{
  if (Theme != NULL)
  {
    COLORREF Color;
    int HighlightWidth = SquareSize/20;
    const ChessMove* LastMove = Chess.GetMove(Chess.GetMoveCount()-1);
    /* Draw the board squares */
    for (int i=0; i < 8; i++)
    {
      for (int j=0; j < 8; j++)
      {
        RECT R = {X+i*SquareSize,Y+j*SquareSize,X+(i+1)*SquareSize,Y+(j+1)*SquareSize};
        Position Pos = MakePos(i,7-j);
        /* Calculate the color for the square */
        if (!Paused && LastMove != NULL && ShowLastMove && LastMove->To.x == Pos.x && LastMove->To.y == Pos.y)
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
            if (Chess.IsMoveValid(SelectedSquare, HighlightedSquare))
              Color = Theme->ValidMoveColor;
            else
              Color = Theme->InvalidMoveColor;
          }
          else
            Color = Theme->CurrentSquareColor;
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
  if (Set != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName,(int)(SquareSize*.75),0));
    /* Draw the chess piece */
    for (int i=0; i < 8; i++)
      for (int j=0; j < 8; j++)
      {
        Position Pos = MakePos(i,7-j);
        if (SelectedSquare.x != Pos.x || SelectedSquare.y != Pos.y)
          DrawChessPiece(DC,X+SquareSize*i,Y+SquareSize*j,Chess.GetPiece(Pos));
      }
    /* Draw the selected piece */
    if (SelectedSquare.x >= 0 && SelectedSquare.y >= 0)
    {
      POINT Point;
      GetCursorPos(&Point);
      RECT Rect;
      GetWindowRect(Handle, &Rect);
      DrawChessPiece(DC,Point.x-Rect.left,Point.y-Rect.top,Chess.GetPiece(SelectedSquare));
    }
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
    GetTextExtentPoint32(DC,&Set->Letters[Index+6],1,&Size);
    if ((Piece->Color == White && Theme->WhitePiecesStyle != Outline) || (Piece->Color == Black && Theme->BlackPiecesStyle != Outline))
    {
      if (Piece->Color == White)
        SetTextColor(DC, Theme->WhitePiecesColor);
      else
        SetTextColor(DC, Theme->BlackPiecesColor);
      TextOut(DC,X-Size.cx/2,Y-Size.cy/2,&Set->Letters[Index+6],1);
    }
    /* Draw an outline */
    if ((Piece->Color == White && Theme->WhitePiecesStyle != Plain) || (Piece->Color == Black && Theme->BlackPiecesStyle != Plain))
    {
      if (Piece->Color == White)
        SetTextColor(DC, Theme->WhitePiecesOutlineColor);
      else
        SetTextColor(DC, Theme->BlackPiecesOutlineColor);
      TextOut(DC,X-Size.cx/2,Y-Size.cy/2,&Set->Letters[Index],1);
    }
    /* Clean up */
    SetTextColor(DC, 0x000000);
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
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultFontName,max(int(SquareSize*.5),7),0));
    HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,int(SquareSize*.1),Theme->BorderColor));
    GetTextExtentPoint32(DC,Str,strlen(Str),&Size);
    BeginPath(DC);
    TextOut(DC,(min(Width,Height)-Size.cx)/2,(min(Width,Height)-Size.cy)/2,Str,strlen(Str));
    EndPath(DC);
    StrokePath(DC);
    /* Draw the text */
    SetTextColor(DC, Theme->CoordinatesColor);
    DeleteObject(SelectObject(DC,EasyCreateFont(DC,DefaultFontName,max(int(SquareSize*.5),7),0)));
    GetTextExtentPoint32(DC,Str,strlen(Str),&Size);
    TextOut(DC,(min(Width,Height)-Size.cx)/2,(min(Width,Height)-Size.cy)/2,Str,strlen(Str));
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
    DeleteObject(SelectObject(DC,OldPen));
  }
}

Position ChessBoardPanel::MakePos(int X, int Y)
{
  Position Pos;
  if (InvertView)
  {
    Pos.x = 7-X;
    Pos.y = 7-Y;
  }
  else
  {
    Pos.x = X;
    Pos.y = Y;
  }
  return Pos;
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void ChessBoardPanel::MouseDown(int x, int y)
{
  if (!Locked)
  {
    if (HighlightedSquare.x >= 0 && HighlightedSquare.x < 8 && HighlightedSquare.y >= 0 && HighlightedSquare.y < 8)
      if (SelectedSquare.x == -1 && SelectedSquare.y == -1 && Chess.GetPiece(HighlightedSquare) != NULL && Chess.GetPiece(HighlightedSquare)->Color == Chess.GetActivePlayer())
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
  if (!Locked)
  {
    bool PieceMoved = true; // Prevent from updating the board if the piece was not moved
    if (HighlightedSquare.x >= 0 && HighlightedSquare.x < 8 && HighlightedSquare.y >= 0 && HighlightedSquare.y < 8)
      if (SelectedSquare.x >= 0 && SelectedSquare.x < 8 && SelectedSquare.y >= 0 && SelectedSquare.y < 8)
        if (HighlightedSquare.x != SelectedSquare.x || HighlightedSquare.y != SelectedSquare.y)
          /* Move the selected piece to the highlighted square */
          if (MovePieceProc != NULL)
            PieceMoved = (*MovePieceProc)(SelectedSquare, HighlightedSquare);
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
      MouseMoved = true; // Allows to cancel the selection when clicking a piece
  }
}

void ChessBoardPanel::MouseMove(int x, int y)
{
  if (x >= BorderSize && x < BorderSize+SquareSize*8 && y >= BorderSize && y < BorderSize+SquareSize*8)
    HighlightedSquare = MakePos(min((x-BorderSize)/SquareSize,7),7-min((y-BorderSize)/SquareSize,7));
  else
  {
    HighlightedSquare.x = -1;
    HighlightedSquare.y = -1;
  }
  MouseMoved = true; // Allows to cancel the selection when dragging a piece
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
        DrawBoardIndexes(Buffer,BorderSize+SquareSize/2,SquareSize*8+(int)(BorderSize*1.5),false); /* Horizontal */
      }
      /* Draw the board squares */
      DrawBoardSquares(Buffer, BorderSize, BorderSize);
      /* Draw the chess pieces */
      if (Paused)
        DrawPaused(Buffer);
      else
        DrawChessPieces(Buffer, BorderSize+SquareSize/2, BorderSize+SquareSize/2);
      /* Draw the buffer into the destination DC */
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
      ChessBoardPanel* Panel = (ChessBoardPanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);

      if (Panel != NULL)
      {
        Panel->Handle = hWnd;
      }

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
        Panel->MouseDown((int)LOWORD(lParam), (int)HIWORD(lParam));
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->MouseUp((int)LOWORD(lParam), (int)HIWORD(lParam));
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->MouseMove((int)LOWORD(lParam), (int)HIWORD(lParam));
      return 0;
    }
    case WM_PAINT:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->Paint();
      return 0;
    }
    case WM_SIZE:
    {
      ChessBoardPanel* Panel = (ChessBoardPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
          Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
