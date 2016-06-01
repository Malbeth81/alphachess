/*
* RoomPanel.cpp
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
#include "roompanel.h"
#include "../resource.h"

const char ClassName[] = "RoomPanel";

extern char DefaultFontName[];
extern GameClient* NetworkClient;

ATOM RoomPanel::ClassAtom = 0;

// PUBLIC FUNCTIONS ------------------------------------------------------------

RoomPanel::RoomPanel(HWND hParent, RECT* R)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));

  LeaveRoomButtonClickedProc = NULL;
  KickPlayerButtonClickedProc = NULL;

  Handle = NULL;
  PlayerList = NULL;
  LeaveRoomButton = NULL;
  KickPlayerButton = NULL;

  Height = R->bottom-R->top;
  RoomName = NULL;
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
    WndClass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
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
    R->left,R->left,R->bottom-R->top,R->right-R->left,hParent,NULL,Instance,this);
}

RoomPanel::~RoomPanel()
{
  /* Destroys the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
}

void RoomPanel::EnableKickPlayerButton(bool Value)
{
  if (Handle != NULL)
    EnableWindow(KickPlayerButton,Value);
}

void RoomPanel::EnableLeaveRoomButton(bool Value)
{
  if (Handle != NULL)
    EnableWindow(LeaveRoomButton,Value);
}

HWND RoomPanel::GetHandle()
{
  return Handle;
}

const char* RoomPanel::GetRoomName()
{
  return RoomName;
}

void RoomPanel::Invalidate()
{
  UpdatePlayerList();
}

unsigned int RoomPanel::GetSelectedPlayer()
{
  int Index = SendMessage(PlayerList,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED);
  if (Index >= 0)
  {
    char* Str = new char[6];
    ListView_GetItemText(PlayerList,Index,0,Str,6);
    int PlayerId = atoi(Str);
    delete[] Str;
    return PlayerId;
  }
  return 0;
}

void RoomPanel::SetRoomName(char* Name)
{
  delete[] RoomName;
  RoomName = Name;
}

// PRIVATE FUNCTIONS -----------------------------------------------------------

void RoomPanel::AddPlayerToList(const PlayerInfo* Player)
{
  if (Handle != NULL && Player != NULL)
  {
    /* Add a new item to the list view */
    LVITEM ItemInfo;
    ItemInfo.mask = 0;
    ItemInfo.iItem = ListView_GetItemCount(PlayerList);
    ItemInfo.iSubItem = 0;
    ListView_InsertItem(PlayerList,&ItemInfo);
    /* Set the item's other attributes */
    char* Str = inttostr(Player->PlayerId);
    ListView_SetItemText(PlayerList,ItemInfo.iItem,0,Str);
    delete[] Str;
    ListView_SetItemText(PlayerList,ItemInfo.iItem,1,(CHAR*)Player->Name);
  }
}

void RoomPanel::UpdatePlayerList()
{
  ListView_DeleteAllItems(PlayerList);
  AddPlayerToList(NetworkClient->GetWhitePlayer());
  AddPlayerToList(NetworkClient->GetBlackPlayer());
  for (unsigned int i = 0; i < NetworkClient->GetObserverCount(); i++)
    AddPlayerToList(NetworkClient->GetObserver(i));
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void RoomPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Resize child windows */
  SetWindowPos(PlayerList,NULL,0,2,Width,Height-26,SWP_NOZORDER);
  SetWindowPos(LeaveRoomButton,NULL,0,Height-22,Width/2,22,SWP_NOZORDER);
  SetWindowPos(KickPlayerButton,NULL,Width/2,Height-22,Width/2,22,SWP_NOZORDER);
}

// WINAPI FUNCTIONS ------------------------------------------------------------

LRESULT __stdcall RoomPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      RoomPanel* Panel = (RoomPanel*)Params->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);

      if (Panel != NULL)
      {
        Panel->Handle = hWnd;

        /* Create the child windows */
        Panel->PlayerList = CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS,
            0,2,Panel->Width,Panel->Height-26,hWnd,NULL,GetModuleHandle(NULL),NULL);
        Panel->LeaveRoomButton = CreateWindowEx(0,"BUTTON","Leave room",WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE,
            0,Panel->Height-22,Panel->Width/2,22,hWnd,NULL,GetModuleHandle(NULL),NULL);
        Panel->KickPlayerButton = CreateWindowEx(0,"BUTTON","Kick player",WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE,
            Panel->Width/2,Panel->Height-22,Panel->Width/2,22,hWnd,NULL,GetModuleHandle(NULL),NULL);

        /* Change the window's appearance */
        SendMessage(Panel->PlayerList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
        char* CollumnName = new char[20];
        LoadString(GetModuleHandle(NULL), IDS_PLAYERLIST_COL1, CollumnName, 20);
        AddListViewCollumn(Panel->PlayerList, CollumnName, 0, 25);
        LoadString(GetModuleHandle(NULL), IDS_PLAYERLIST_COL2, CollumnName, 20);
        AddListViewCollumn(Panel->PlayerList, CollumnName, 1, 110);
        delete[] CollumnName;
        HFONT Font = EasyCreateFont(NULL,DefaultFontName,8,0);
        PostMessage(Panel->PlayerList,WM_SETFONT,(WPARAM)Font,FALSE);
        PostMessage(Panel->LeaveRoomButton,WM_SETFONT,(WPARAM)Font,FALSE);
        PostMessage(Panel->KickPlayerButton,WM_SETFONT,(WPARAM)Font,FALSE);
      }
      return 0;
    }
    case WM_COMMAND:
    {
      RoomPanel* Panel = (RoomPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      /* Process child window's messages */
      if (Panel != NULL)
      {
        if ((HWND)lParam == Panel->LeaveRoomButton && Panel->LeaveRoomButtonClickedProc != NULL)
          (*(Panel->LeaveRoomButtonClickedProc))();
        if ((HWND)lParam == Panel->KickPlayerButton && Panel->KickPlayerButtonClickedProc != NULL)
          (*(Panel->KickPlayerButtonClickedProc))();
      }
      return 0;
    }
    case WM_DESTROY:
    {
      RoomPanel* Panel = (RoomPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      /* Destroy the child windows */
      if (Panel != NULL)
      {
        DestroyWindow(Panel->PlayerList);
        DestroyWindow(Panel->LeaveRoomButton);
        DestroyWindow(Panel->KickPlayerButton);
      }
      return 0;
    }
    case WM_SIZE:
    {
      RoomPanel* Panel = (RoomPanel*)GetWindowLong(hWnd, GWL_USERDATA);

      if (Panel != NULL && LOWORD(lParam) != Panel->Width || HIWORD(lParam) != Panel->Height)
          Panel->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
  }
  return DefWindowProc(hWnd,Msg,wParam,lParam);
}
