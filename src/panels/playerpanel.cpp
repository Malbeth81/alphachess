/*
* PlayerPanel.cpp
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
#include "playerpanel.h"

const char ClassName[] = "PlayersInfoPanel";

ATOM PlayerPanel::ClassAtom = 0;
WNDPROC PlayerPanel::OldPlayerButtonProc = 0;

// PUBLIC FUNCTIONS ------------------------------------------------------------

PlayerPanel::PlayerPanel(HINSTANCE hInstance, HWND hParent, RECT* R, ChessPieceColor NewColor)
{
  Handle = NULL;
  PlayerButton = NULL;
  PlayerMenu = NULL;

  Game = NULL;
  Color = NewColor;
  Set = NULL;
  IsReady = false;
  Height = R->bottom-R->top;
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
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.lpszMenuName = NULL;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    ClassAtom = RegisterClassEx(&WndClass);
  }
  /* Creates the window */
  if (ClassAtom != 0)
  {
    Handle = CreateWindowEx(0,ClassName,NULL,WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
        R->top,R->bottom,Width,Height,hParent,NULL,hInstance,this);
    if (Handle != NULL)
    {
      /* Create the child windows */
      PlayerButton = CreateWindowEx(0,"BUTTON",NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      OldPlayerButtonProc = (WNDPROC)SetWindowLong(PlayerButton,GWL_WNDPROC,(LONG)PlayerButtonProc);

      /* Initialize child window placement */
      ApplyThemeToCustomButton(Handle);
      UpdateSize(Width, Height);

      /* Change the window's appearance */
      HFONT Font = EasyCreateFont(NULL,DefaultSystemFont,10,fsBold);
      PostMessage(PlayerButton,WM_SETFONT,(WPARAM)Font,FALSE);

      /* Create the popup menu */
      PlayerMenu = CreatePopupMenu();
      AppendMenu(PlayerMenu,Color == White ? IDS_PLAYERMENU_JOINASWHITE : IDS_PLAYERMENU_JOINASBLACK);
      AppendMenu(PlayerMenu,IDS_PLAYERMENU_LEAVE);
      AppendSeparator(PlayerMenu);
      AppendMenu(PlayerMenu,IDS_PLAYERMENU_READY);

      /* Set the menu items default state */
      EnableMenuItem(PlayerMenu,Color == White ? IDS_PLAYERMENU_JOINASWHITE : IDS_PLAYERMENU_JOINASBLACK,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(PlayerMenu,IDS_PLAYERMENU_LEAVE,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(PlayerMenu,IDS_PLAYERMENU_READY,MF_BYCOMMAND|MF_GRAYED);
    }
  }
}

PlayerPanel::~PlayerPanel()
{
  /* Destroys the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
  if (PlayerButton != NULL)
    DestroyWindow(PlayerButton);
}

void PlayerPanel::EnableJoinButton(const bool Value)
{
  if (PlayerMenu != NULL)
    EnableMenuItem(PlayerMenu,Color == White ? IDS_PLAYERMENU_JOINASWHITE : IDS_PLAYERMENU_JOINASBLACK,MF_BYCOMMAND|(Value ? MF_ENABLED : MF_GRAYED));
}

void PlayerPanel::EnableLeaveButton(const bool Value)
{
  if (PlayerMenu != NULL)
    EnableMenuItem(PlayerMenu,IDS_PLAYERMENU_LEAVE,MF_BYCOMMAND|(Value ? MF_ENABLED : MF_GRAYED));
}

void PlayerPanel::EnableReadyButton(const bool Value)
{
  if (PlayerMenu != NULL)
    EnableMenuItem(PlayerMenu,IDS_PLAYERMENU_READY,MF_BYCOMMAND|(Value ? MF_ENABLED : MF_GRAYED));
}

HWND PlayerPanel::GetHandle()
{
  return Handle;
}

void PlayerPanel::Invalidate()
{
  if (Handle != NULL)
    InvalidateRect(Handle, NULL, FALSE);
  if (PlayerButton != NULL)
    InvalidateRect(PlayerButton, NULL, FALSE);
}

void PlayerPanel::SetChessSet(ChessSet* NewSet)
{
  Set = NewSet;
  Invalidate();
}

void PlayerPanel::SetGame(ChessGame* NewGame)
{
  Game = NewGame;
  Invalidate();
}

void PlayerPanel::SetReady(bool Value)
{
  IsReady = Value;
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void PlayerPanel::DrawChessPiece(HDC DC, int X, int Y)
{
  if (Set != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, GetSysColor(COLOR_BTNTEXT));
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,Set->FontName.c_str(),PixelsToPoints(DC, Height-4),0));
    /* Draw the piece */
    TextOut(DC,X,Y,Set->Letters.substr(Color == White ? 0 : 6, 1).c_str(),1);
    /* Clean up */
    DeleteObject(SelectObject(DC,OldFont));
    SetBkMode(DC,OPAQUE);
  }
}

void PlayerPanel::DrawPlayerInformation(HDC DC, int X, int Y, int Width, int Height)
{
  if (Game != NULL)
  {
    const ChessPlayer* Player = Game->GetPlayer(Color);
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, GetSysColor(COLOR_BTNTEXT));
    /* Highlight the active player */
    if (Game->GetActivePlayer() == Color && Game->GetState() != Undefined)
    {
      HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT)));
      HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(COLOR_HIGHLIGHT)));
      Rectangle(DC,X,Y,X+Width,Y+Height);
      DeleteObject(SelectObject(DC,OldPen));
      DeleteObject(SelectObject(DC,OldBrush));
      SetTextColor(DC,GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    if (Game->GetState() >= Started)
    {
      /* Draw the player's time */
      SIZE S;
      HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,8,0));
      char* Str = timeformat(Player->MoveTime/1000);
      GetTextExtentPoint32(DC,Str,strlen(Str),&S);
      TextOut(DC,X+4,Height-S.cy-2,Str,strlen(Str));
      delete[] Str;
      DeleteObject(SelectObject(DC,OldFont));
    }
    /* Draw the player's status */
    string Text;
    if (Game->GetActivePlayer() == Color)
    {
      if (Game->IsPlayerChecked() && Game->IsPlayerMated())
        Text = "Checkmate";
      else if (Game->IsPlayerMated())
        Text = "Stalemate";
      else if (Game->IsPlayerChecked())
        Text = "Check";
    }
    else if (IsReady)
      Text = "Ready";
    if (Text.length() > 0)
    {
      SIZE S;
      HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,8,fsBold));
      GetTextExtentPoint32(DC,Text.c_str(),Text.length(),&S);
      TextOut(DC,X+Width-S.cx-4,Height-S.cy-2,Text.c_str(),Text.length());
      DeleteObject(SelectObject(DC,OldFont));
    }
    /* Clean up */
    SetBkMode(DC,OPAQUE);
  }
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
  float scaleFactor = GetDPIScaleFactor();
  if (PlayerButton != NULL)
    SetWindowPos(PlayerButton,NULL,Height-2,1,Width-Height,(int)(21*scaleFactor),SWP_NOZORDER);
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

LRESULT __stdcall PlayerPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_COMMAND:
    {
      /* Forward message to parent */
      return SendMessage(GetParent(hWnd), WM_COMMAND, wParam, lParam);
    }
    case WM_CONTEXTMENU:
    {
      /* Show menu */
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        TrackPopupMenu(Panel->PlayerMenu, TPM_LEFTALIGN, LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);
      }
      return 0;
    }
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      PlayerPanel* Panel = (PlayerPanel*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
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
      {
        Panel->Paint();
      }
      return 0;
    }
    case WM_SIZE:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        if (LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
            Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      }
      return 0;
    }
    case WM_THEMECHANGED:
    {
      ApplyThemeToCustomButton(hWnd);
      return 0;
    }
    case WM_UNINITMENUPOPUP:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL && Panel->PlayerButton != NULL)
      {
        POINT Pos;
        GetCursorPos(&Pos);
        RECT R;
        GetWindowRect(hWnd, &R);
        Pos.x = Pos.x-R.left;
        Pos.y = Pos.y-R.top;
        if (ChildWindowFromPoint(hWnd,Pos) != Panel->PlayerButton)
        {
          /* Set button status to none */
          SetWindowLong(Panel->PlayerButton, GWL_USERDATA, 0);
          InvalidateRect(Panel->PlayerButton, NULL, TRUE);
        }
      }
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT __stdcall PlayerPanel::PlayerButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_LBUTTONDOWN:
    {
      SetCapture(hWnd);

      /* Show menu */
      if (GetWindowLong(hWnd, GWL_USERDATA) < 2)
      {
        RECT R;
        GetWindowRect(hWnd, &R);
        PostMessage(GetParent(hWnd), WM_CONTEXTMENU, (WPARAM)hWnd, MAKELPARAM(R.left,R.bottom));
      }

      /* Set button status to pressed */
      SetWindowLong(hWnd, GWL_USERDATA, 2);
      InvalidateRect(hWnd, NULL, TRUE);
      return 0;
    }
    case WM_LBUTTONUP:
    {
      ReleaseCapture();

      /* Set button status to hover */
      SetWindowLong(hWnd, GWL_USERDATA, 1);
      InvalidateRect(hWnd, NULL, TRUE);
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      /* Track mouse leave */
      TRACKMOUSEEVENT* Event = new TRACKMOUSEEVENT;
      Event->cbSize = sizeof(TRACKMOUSEEVENT);
      Event->dwFlags = TME_LEAVE;
      Event->hwndTrack = hWnd;
      TrackMouseEvent(Event);

      /* Set button status to hover */
      SetWindowLong(hWnd, GWL_USERDATA, 1);
      InvalidateRect(hWnd, NULL, TRUE);
      return 0;
    }
    case WM_MOUSELEAVE:
    {
      if (GetWindowLong(hWnd, GWL_USERDATA) < 2)
      {
        /* Set button status to none */
        SetWindowLong(hWnd, GWL_USERDATA, 0);
        InvalidateRect(hWnd, NULL, TRUE);
      }
      return 0;
    }
    case WM_PAINT:
    {
      PlayerPanel* Panel = (PlayerPanel*)GetWindowLong(GetParent(hWnd), GWL_USERDATA);

      if (Panel != NULL)
      {
        PAINTSTRUCT PS;
        HDC DC = BeginPaint(hWnd, &PS);
        if (DC != NULL)
        {
          RECT R;
          GetClientRect(hWnd, &R);

          /* Paint the background */
          if (GetWindowLong(hWnd, GWL_USERDATA) == 0)
          {
            /* Plain background */
            HBRUSH OldBrush = (HBRUSH)SelectObject(DC,CreateSolidBrush(GetSysColor(Panel->Game != NULL && Panel->Game->GetActivePlayer() == Panel->Color && Panel->Game->GetState() != Undefined ? COLOR_HIGHLIGHT : COLOR_BTNFACE)));
            HPEN OldPen = (HPEN)SelectObject(DC,CreatePen(PS_SOLID,1,GetSysColor(Panel->Game != NULL && Panel->Game->GetActivePlayer() == Panel->Color && Panel->Game->GetState() != Undefined ? COLOR_HIGHLIGHT : COLOR_BTNFACE)));
            Rectangle(DC,R.top,R.left,R.right-R.left,R.bottom-R.top);
            DeleteObject(SelectObject(DC,OldPen));
            DeleteObject(SelectObject(DC,OldBrush));
          }
          else
          {
            /* Button frame background */
            DRAWITEMSTRUCT* Item = new DRAWITEMSTRUCT;
            Item->hDC = DC;
            Item->itemState = (GetWindowLong(hWnd, GWL_USERDATA) > 1 ? ODS_SELECTED : ODS_DEFAULT);
            Item->rcItem = R;
            DrawCustomButton(hWnd, Item);
            delete Item;
          }

          /* Draw the player's name */
          if (Panel->Game != NULL)
          {
            InflateRect(&R,-5,-1);
            const ChessPlayer* Player = Panel->Game->GetPlayer(Panel->Color);
            SetBkMode(DC,TRANSPARENT);
            SetTextColor(DC, GetSysColor(Panel->Game->GetActivePlayer() == Panel->Color && Panel->Game->GetState() != Undefined && GetWindowLong(hWnd, GWL_USERDATA) == 0 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
            HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,DefaultSystemFont,9,0));
            const char* Text = Player->Name.c_str();
            if (strlen(Text) == 0)
            {
              if (Panel->Color == White)
                Text = "White";
              else
                Text = "Black";
            }
            DrawText(DC,Text,strlen(Text),&R,DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
            DeleteObject(SelectObject(DC,OldFont));
          }
        }
        EndPaint(hWnd, &PS);
      }
      return 0;
    }
  }
  return CallWindowProc(OldPlayerButtonProc, hWnd, Msg, wParam, lParam);
}
