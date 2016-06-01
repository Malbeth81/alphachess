/*
* PlayerPanel.cpp
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
#include "playerpanel.h"

const char ClassName[] = "PlayersInfoPanel";

extern char DefaultFontName[];
extern ChessEngine Chess;

ATOM PlayerPanel::ClassAtom = 0;

// PUBLIC FUNCTIONS ------------------------------------------------------------

PlayerPanel::PlayerPanel(HWND hParent, RECT* R, ChessPieceColor NewColor)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));

  JoinButtonClickedProc = NULL;
  ReadyButtonClickedProc = NULL;

  Handle = NULL;
  JoinButton = NULL;
  ReadyButton = NULL;

  Color = NewColor;
  Set = NULL;
  State = 0;
  Height = R->bottom-R->top;
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
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.lpszMenuName = 0;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  /* Creates the window */
  if (ClassAtom != 0)
    CreateWindowEx(0,ClassName,NULL,WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    R->top,R->bottom,R->bottom-R->top,R->right-R->left,hParent,NULL,Instance,this);
}

PlayerPanel::~PlayerPanel()
{
  /* Destroys the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
}

HWND PlayerPanel::GetHandle()
{
  return Handle;
}

void PlayerPanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Handle, NULL, FALSE);
}

void PlayerPanel::SetChessSet(ChessSet* NewSet)
{
  Set = NewSet;
  Invalidate();
}

void PlayerPanel::SetState(int Result)
{
  State = Result;
  Invalidate();
}

void PlayerPanel::ShowJoinButton(const bool Value)
{
  if (Handle != NULL)
  {
    if (Value)
    {
      SetWindowText(JoinButton,"Join");
      SetWindowPos(JoinButton,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    }
    else
      SetWindowPos(JoinButton,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
  }
}

void PlayerPanel::ShowLeaveButton(const bool Value)
{
  if (Handle != NULL)
  {
    if (Value)
    {
      SetWindowText(JoinButton,"Leave");
      SetWindowPos(JoinButton,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    }
    else
      SetWindowPos(JoinButton,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
  }
}

void PlayerPanel::ShowReadyButton(const bool Value)
{
  if (Handle != NULL)
  {
    if (Value)
      SetWindowPos(ReadyButton,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    else
      SetWindowPos(ReadyButton,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
  }
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void PlayerPanel::DrawChessPiece(HDC DC, int X, int Y)
{
  if (Set != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, GetSysColor(COLOR_BTNTEXT));
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName,(int)(Height*.72),0));
    /* Draw the piece */
    if (Color == White)
      TextOut(DC,X,Y,&Set->Letters[0],1);
    else
      TextOut(DC,X,Y,&Set->Letters[6],1);
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

void PlayerPanel::DrawPlayerInformation(HDC DC, int X, int Y, int Width, int Height)
{
  SIZE S;
  const ChessPlayer* Player = Chess.GetPlayer(Color);
  SetBkMode(DC,TRANSPARENT);
  SetTextColor(DC, GetSysColor(COLOR_BTNTEXT));
  /* Draw a frame if player is active */
  if (Chess.GetActivePlayer() == Color && Chess.GetGameState() == Started)
  {
    SetTextColor(DC,GetSysColor(COLOR_HIGHLIGHTTEXT));
    HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT)));
    HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_HIGHLIGHT)));
    Rectangle(DC,X,Y,X+Width,Y+Height);
    DeleteObject(SelectObject(DC,OldPen));
    DeleteObject(SelectObject(DC,OldBrush));
  }
  /* Draw the player's name */
  HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultFontName,11,0));
  TextOut(DC,X+4,Y,Player->Name.c_str(),Player->Name.length());
  if (Chess.GetGameState() > Created)
  {
    /* Draw the player's time */
    DeleteObject(SelectObject(DC,OldFont));
    OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultFontName,8,0));
    char* Str = timeformat(Player->Time/1000);
    GetTextExtentPoint32(DC,Str,strlen(Str),&S);
    TextOut(DC,X+4,Height-S.cy-2,Str,strlen(Str));
    delete[] Str;
  }
  /* Draw the player's status */
  DeleteObject(SelectObject(DC,OldFont));
  OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultFontName,8,fsBold));
  char* Text = new char[16];
  strcpy(Text, "");
  if (State & Ready)
    strcpy(Text, "Ready");
  else if (State & Mated && State & Checked)
    strcpy(Text, "Checkmate");
  else if (State & Mated)
    strcpy(Text, "Stalemate");
  else if (State & Checked)
    strcpy(Text, "Check");
  if (strlen(Text) > 0)
  {
    GetTextExtentPoint32(DC,Text,strlen(Text),&S);
    TextOut(DC,X+Width-S.cx-4,Height-S.cy-2,Text,strlen(Text));
  }
  delete[] Text;
  /* Clean up */
  SetBkMode(DC,OPAQUE);
  DeleteObject(SelectObject(DC,OldFont));
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void PlayerPanel::Paint()
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
      /* Paint the background */
      HBRUSH OldBrush = (HBRUSH)SelectObject(Buffer,CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));
      HPEN OldPen = (HPEN)SelectObject(Buffer,CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNFACE)));
      Rectangle(Buffer,0,0,Width,Height);
      DeleteObject(SelectObject(Buffer,OldPen));
      DeleteObject(SelectObject(Buffer,OldBrush));
      /* Draw the chess piece */
      DrawChessPiece(Buffer,0,2);
      /* Draw the player's information */
      DrawPlayerInformation(Buffer,Height-2,1,Width-Height,Height-2);
      /* Draw the buffer into the destination DC */
      BitBlt(DC,0,0,Width,Height,Buffer,0,0,SRCCOPY);
      /* Cleanup */
      DeleteDC(Buffer);
      DeleteObject(Bitmap);
    }
    EndPaint(Handle, &PS);
  }
}

void PlayerPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Resize the child windows */
  SetWindowPos(JoinButton,NULL,Height-2,Width-21,(Width-Height-2)/2,21,SWP_NOZORDER);
  SetWindowPos(ReadyButton,NULL,Height+(Width-Height-2)/2,Height-21,(Width-Height-2)/2,21,SWP_NOZORDER);
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall PlayerPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      PlayerPanel* Panel = (PlayerPanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);

      if (Panel != NULL)
      {
        Panel->Handle = hWnd;

        /* Create the child windows */
        Panel->JoinButton = CreateWindowEx(0,"BUTTON",NULL,WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP,
            Panel->Height-2,Panel->Height-21,(Panel->Width-Panel->Height-2)/2,21,hWnd,NULL,Params->hInstance,NULL);
        Panel->ReadyButton = CreateWindowEx(0,"BUTTON","Ready",WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP,
            Panel->Height+(Panel->Width-Panel->Height-2)/2,Panel->Height-21,(Panel->Width-Panel->Height-2)/2,21,hWnd,NULL,Params->hInstance,NULL);

        /* Change the window's appearance */
        HFONT Font = EasyCreateFont(NULL,DefaultFontName,8,fsBold);
        PostMessage(Panel->JoinButton,WM_SETFONT,(WPARAM)Font,FALSE);
        PostMessage(Panel->ReadyButton,WM_SETFONT,(WPARAM)Font,FALSE);
      }
      return 0;
    }
    case WM_COMMAND:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      /* Process child window's messages */
      if (Panel != NULL)
      {
        if ((HWND)lParam == Panel->JoinButton && Panel->JoinButtonClickedProc != NULL)
          (*(Panel->JoinButtonClickedProc))(Panel->Color);
        if ((HWND)lParam == Panel->ReadyButton && Panel->ReadyButtonClickedProc != NULL)
          (*(Panel->ReadyButtonClickedProc))(Panel->Color);
      }
      return 0;
    }
    case WM_DESTROY:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      /* Destroy the child windows */
      if (Panel != NULL)
      {
        DestroyWindow(Panel->JoinButton);
        DestroyWindow(Panel->ReadyButton);
      }
      return 0;
    }
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_PAINT:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL)
        Panel->Paint();
      return 0;
    }
    case WM_SIZE:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
          Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
