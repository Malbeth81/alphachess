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

ATOM RoomPanel::ClassAtom = 0;

// PUBLIC FUNCTIONS ------------------------------------------------------------

RoomPanel::RoomPanel(HINSTANCE hInstance, HWND hParent, RECT* R)
{
  Handle = NULL;
  PlayerList = NULL;
  LeaveRoomButton = NULL;
  KickPlayerButton = NULL;

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
    WndClass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
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
    Handle = CreateWindowEx(0,ClassName,NULL,WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        R->left,R->left,Width,Height,hParent,NULL,hInstance,this);
    if (Handle != NULL)
    {
      /* Create the child windows */
      PlayerList = CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      LeaveRoomButton = CreateWindowEx(0,"BUTTON","Leave room",WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE,
          0,0,0,0,Handle,NULL,hInstance,NULL);
      KickPlayerButton = CreateWindowEx(0,"BUTTON","Kick player",WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE,
          0,0,0,0,Handle,NULL,hInstance,NULL);

      /* Initialize child window placement */
      UpdateSize(Width, Height);

      /* Enable full row selection */
      SendMessage(PlayerList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

      /* Add collumns to list view */
      float scaleFactor = GetDPIScaleFactor();
      char* CollumnName = new char[20];
      LoadString(hInstance, IDS_PLAYERLIST_COL1, CollumnName, 20);
      AddListViewCollumn(PlayerList, CollumnName, 0, (int)(25*scaleFactor));
      LoadString(hInstance, IDS_PLAYERLIST_COL2, CollumnName, 20);
      AddListViewCollumn(PlayerList, CollumnName, 1, (int)(110*scaleFactor));
      delete[] CollumnName;

      /* Set controls font */
      HFONT Font = EasyCreateFont(NULL,DefaultFontName,8,0);
      PostMessage(PlayerList,WM_SETFONT,(WPARAM)Font,FALSE);
      PostMessage(LeaveRoomButton,WM_SETFONT,(WPARAM)Font,FALSE);
      PostMessage(KickPlayerButton,WM_SETFONT,(WPARAM)Font,FALSE);
    }
  }
}

RoomPanel::~RoomPanel()
{
  /* Destroys the window */
  if (Handle != NULL)
    DestroyWindow(Handle);
}

void RoomPanel::EnableKickPlayerButton(bool Value)
{
  if (KickPlayerButton != NULL)
    EnableWindow(KickPlayerButton,Value);
}

void RoomPanel::EnableLeaveRoomButton(bool Value)
{
  if (LeaveRoomButton != NULL)
    EnableWindow(LeaveRoomButton,Value);
}

HWND RoomPanel::GetHandle()
{
  return Handle;
}

const string RoomPanel::GetRoomName()
{
  return RoomName;
}

void RoomPanel::Invalidate()
{
  UpdatePlayerList();
}

unsigned int RoomPanel::GetSelectedPlayer()
{
  if (PlayerList != NULL)
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
  }
  return 0;
}

void RoomPanel::SetGameClient(GameClient* Client)
{
  NetworkClient = Client;
}

void RoomPanel::SetRoomName(string Name)
{
  RoomName = Name;
}

// PRIVATE FUNCTIONS -----------------------------------------------------------

void RoomPanel::AddPlayerToList(const PlayerInfo* Player)
{
  if (PlayerList != NULL && Player != NULL)
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
    ListView_SetItemText(PlayerList,ItemInfo.iItem,1,(CHAR*)Player->Name.c_str());
  }
}

void RoomPanel::UpdatePlayerList()
{
  if (NetworkClient != NULL && PlayerList != NULL)
  {
    ListView_DeleteAllItems(PlayerList);
    AddPlayerToList(NetworkClient->GetWhitePlayer());
    AddPlayerToList(NetworkClient->GetBlackPlayer());
    for (unsigned int i = 0; i < NetworkClient->GetObserverCount(); i++)
      AddPlayerToList(NetworkClient->GetObserver(i));
  }
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

void RoomPanel::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Resize child windows */
  float scaleFactor = GetDPIScaleFactor();
  if (PlayerList != NULL)
  {
    SetWindowPos(PlayerList,NULL,0,2,Width,Height-4-(int)(22*scaleFactor),SWP_NOZORDER);
    ListView_SetColumnWidth(PlayerList, 0, (int)(25*scaleFactor));
    ListView_SetColumnWidth(PlayerList, 1, (int)(110*scaleFactor));
  }
  if (LeaveRoomButton != NULL)
    SetWindowPos(LeaveRoomButton,NULL,0,Height-(int)(22*scaleFactor),Width/2,(int)(22*scaleFactor),SWP_NOZORDER);
  if (KickPlayerButton != NULL)
    SetWindowPos(KickPlayerButton,NULL,Width/2,Height-(int)(22*scaleFactor),Width/2,(int)(22*scaleFactor),SWP_NOZORDER);
}

// WINAPI FUNCTIONS ------------------------------------------------------------

LRESULT __stdcall RoomPanel::PanelWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_COMMAND:
    {
      RoomPanel* Panel = (RoomPanel*)GetWindowLong(hWnd, GWL_USERDATA);
      if (Panel != NULL)
      {
        if ((HWND)lParam == Panel->KickPlayerButton)
          SendMessage(GetParent(hWnd), WM_KICKPLAYERBUTTONCLICKED, 0, 0);
        else if ((HWND)lParam == Panel->LeaveRoomButton)
          SendMessage(GetParent(hWnd), WM_LEAVEROOMBUTTONCLICKED, 0, 0);
      }
      return 0;
    }
    case WM_CREATE:
    {
      CREATESTRUCT* Params = (CREATESTRUCT*)lParam;
      RoomPanel* Panel = (RoomPanel*)(Params->lpCreateParams);
      SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)Panel);
      return 0;
    }
    case WM_DESTROY:
    {
      RoomPanel* Panel = (RoomPanel*)GetWindowLong(hWnd, GWL_USERDATA);
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
