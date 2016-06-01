/*
* NetworkGameDialog.cpp
*
* Copyright (C) 2007-2009 Marc-André Lamothe.
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
#include "networkgamedialog.h"

#include "inputdialog.h"
#include "../resource.h"
#include "../chess/chessengine.h"
#include "../panels/roompanel.h"

static HWND hDialog = NULL;

extern ChessEngine Chess;
extern GameClient* NetworkClient;
extern RoomPanel* hRoomPanel;

extern char NetworkPlayerName[60];

// WINAPI FUNCTIONS ------------------------------------------------------------

static INT_PTR __stdcall NetworkGameDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND: /* Process control messages */
      switch (LOWORD(wParam))
      {
        case IDC_NEWGAMEROOM:
        {
          /* Confirmation */
          if (Chess.GetGameState() == Started || Chess.GetGameState() == Paused)
            if (MessageBox(hDlg,"You have a game in progress, are you sure you want to start a new network game?","Start a new network game",MB_OKCANCEL|MB_ICONQUESTION) == IDCANCEL)
              break;
          char* RoomName = InputDialog(hDlg, "Start a new game", "Enter the name of the new game room:");
          if (RoomName != NULL)
          {
            if (strlen(RoomName))
            {
              char* Str = GetWindowText(GetDlgItem(hDlg,IDC_PLAYERNAME));
              strncpy(NetworkPlayerName,Str,60);
              delete[] Str;
              if (strlen(NetworkPlayerName))
              {
                /* Set the player's name */
                NetworkClient->SetPlayerName(NetworkPlayerName);
                /* Create the room */
                if (NetworkClient->CreateRoom(RoomName))
                {
                  hRoomPanel->SetRoomName(RoomName);
                  /* Dialog will be closed when confirmation is received from the server */
                  SetCursor(LoadCursor(NULL,IDC_WAIT));
                }
              }
              // Don't delete RoomName, Room panel will do it when it is replaced
            }
            else
              MessageBox(hDlg, "Error", "Error", MB_OK);
          }
          break;
        }
        case IDC_REFRESHGAMEROOMS:
        {
          /* Update the game room list */
          ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_GAMEROOMLIST));
          NetworkClient->UpdateRoomList();
          break;
        }
        case IDOK:
        {
          /* Confirmation */
          if (Chess.GetGameState() == Started || Chess.GetGameState() == Paused)
            if (MessageBox(hDlg,"You have a game in progress, are you sure you want to join a new game?","Join a network game",MB_OKCANCEL|MB_ICONQUESTION) == IDCANCEL)
              break;
          char* Str = GetWindowText(GetDlgItem(hDlg,IDC_PLAYERNAME));
          strncpy(NetworkPlayerName,Str,60);
          delete[] Str;
          if (strlen(NetworkPlayerName))
          {
            /* Set the player's name */
            NetworkClient->SetPlayerName(NetworkPlayerName);
            HWND Listview = GetDlgItem(hDlg, IDC_GAMEROOMLIST);
            int Index = SendMessage(Listview,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED);
            if (Index >= 0)
            {
              /* Get the selected room */
              char* RoomName = new char[60];
              ListView_GetItemText(Listview,Index,0,RoomName,60);
              int RoomID = atoi(RoomName);
              ListView_GetItemText(Listview,Index,1,RoomName,60);
              /* Join the room */
              if (RoomID > 0 && NetworkClient->JoinRoom(RoomID))
              {
                hRoomPanel->SetRoomName(RoomName);
                /* Dialog will be closed when confirmation is received from the server */
                SetCursor(LoadCursor(NULL,IDC_WAIT));
              }
            }
          }
          break;
        }
        case IDCANCEL:
        {
          if (!NetworkClient->IsInRoom())
            NetworkClient->Disconnect();
          SendMessage(hDlg, WM_CLOSE, 0, 0);
          break;
        }
      }
      return TRUE;
    case WM_CLOSE:
    {
      /* Close the dialog */
      EndDialog(hDlg, wParam);
      /* Free allocated memory */

      hDialog = NULL;
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      hDialog = hDlg;
      HWND Listview = GetDlgItem(hDlg, IDC_GAMEROOMLIST);
      /* Enable full row selection */
      SendMessage(Listview, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
      /* Add columns */
      char* CollumnName = new char[20];
      LoadString(GetModuleHandle(NULL), IDS_GAMEROOMLIST_COL1, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 0, 25);
      LoadString(GetModuleHandle(NULL), IDS_GAMEROOMLIST_COL2, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 1, 265);
      LoadString(GetModuleHandle(NULL), IDS_GAMEROOMLIST_COL3, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 2, 60);
      delete[] CollumnName;
      /* Update the window's content */
      SetWindowText(GetDlgItem(hDlg,IDC_PLAYERNAME),NetworkPlayerName);
      PostMessage(GetDlgItem(hDlg,IDC_PLAYERNAME), EM_SETSEL, (WPARAM)-1, 0);
      NetworkClient->UpdateRoomList();
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch(((LPNMHDR)lParam)->code)
      {
        case NM_DBLCLK:
        {
          PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDOK,0),0);
          break;
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

void AddRoomToList(const RoomInfo* Room)
{
  if (Room != NULL)
  {
    HWND Listview = GetDlgItem(hDialog, IDC_GAMEROOMLIST);
    /* Add a new item to the list view */
    LVITEM ItemInfo;
    ItemInfo.mask = 0;
    ItemInfo.iItem = ListView_GetItemCount(Listview);
    ItemInfo.iSubItem = 0;
    ListView_InsertItem(Listview,&ItemInfo);
    /* Set the item's other attributes */
    char* Str = inttostr(Room->RoomId);
    ListView_SetItemText(Listview,ItemInfo.iItem,0,Str);
    delete[] Str;
    ListView_SetItemText(Listview,ItemInfo.iItem,1,(CHAR*)Room->Name);
    Str = inttostr(Room->PlayerCount);
    ListView_SetItemText(Listview,ItemInfo.iItem,2,Str);
    delete[] Str;
    /* Select the first room */
    if (ItemInfo.iItem == 0)
    {
      ItemInfo.mask = LVIF_STATE;
      ItemInfo.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
      ItemInfo.state = LVIS_SELECTED | LVIS_FOCUSED;
      SendMessage(Listview,LVM_SETITEMSTATE,0,(LPARAM)&ItemInfo);
    }

  }
}

void CloseNetworkGameDialog()
{
  if (hDialog != NULL)
    PostMessage(hDialog,WM_CLOSE,0,0);
}

void ShowNetworkGameDialog(HWND hParent)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));
  DialogBox(Instance,MAKEINTRESOURCE(IDD_NETWORKGAME),hParent,(DLGPROC)NetworkGameDialogProc);
}
