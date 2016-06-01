/*
* NetworkGameDialog.cpp
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
#include "networkgamedialog.h"

static HWND Dialog = NULL;

// WINAPI FUNCTIONS ------------------------------------------------------------

static INT_PTR __stdcall NetworkGameDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_ADDROOMTOLIST:
    {
      NetworkGameRoomInfo* RoomInfo = (NetworkGameRoomInfo*)wParam;
      HWND Listview = GetDlgItem(hDlg, IDC_GAMEROOMLIST);
      /* Add a new item to the list view */
      LVITEM ItemInfo;
      ItemInfo.mask = 0;
      ItemInfo.iItem = ListView_GetItemCount(Listview);
      ItemInfo.iSubItem = 0;
      ListView_InsertItem(Listview,&ItemInfo);
      /* Set the item's other attributes */
      char* Str = inttostr(RoomInfo->RoomId);
      ListView_SetItemText(Listview,ItemInfo.iItem,0,Str);
      delete[] Str;
      ListView_SetItemText(Listview,ItemInfo.iItem,1,(CHAR*)RoomInfo->Name.c_str());
      Str = inttostr(RoomInfo->PlayerCount);
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
    case WM_COMMAND: /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case IDC_CREATEGAMEROOM:
        {
          NetworkGameDialogValues* Values = (NetworkGameDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          /* Read controls' information */
          char* PlayerName = GetWindowText(GetDlgItem(hDlg,IDC_PLAYERNAME));
          if (strlen(PlayerName))
          {
            string RoomName;
            if (InputDialog((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), hDlg, "Start a new game", "Enter the name of the new game room:", &RoomName, 3) && RoomName.length() > 0)
            {
              Values->PlayerName = PlayerName;
              Values->RoomId = 0;
              Values->RoomName = RoomName;

              /* Close dialog */
              PostMessage(hDlg,WM_CLOSE,1,0);
            }
          }
          else
            MessageBox(hDlg, "Please enter your player name.", "New online game", MB_OK|MB_ICONERROR);
          delete[] PlayerName;
          break;
        }
        case IDC_REFRESHGAMEROOMS:
        {
          /* Update the game room list */
          ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_GAMEROOMLIST));
          SendMessage(GetParent(hDlg), WM_UPDATEROOMLIST, 0, 0);
          break;
        }
        case IDOK:
        {
          NetworkGameDialogValues* Values = (NetworkGameDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          /* Read controls' information */
          char* PlayerName = GetWindowText(GetDlgItem(hDlg,IDC_PLAYERNAME));
          if (strlen(PlayerName))
          {
            /* Get index of selected room */
            int Index = SendMessage(GetDlgItem(hDlg, IDC_GAMEROOMLIST),LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED);
            if (Index >= 0)
            {
              char* RoomName = new char[60];
              ListView_GetItemText(GetDlgItem(hDlg, IDC_GAMEROOMLIST),Index,0,RoomName,60);
              unsigned int RoomID = atoi(RoomName);
              ListView_GetItemText(GetDlgItem(hDlg, IDC_GAMEROOMLIST),Index,1,RoomName,60);
              if (strlen(PlayerName) && RoomID > 0)
              {
                Values->PlayerName = PlayerName;
                Values->RoomId = RoomID;
                Values->RoomName = RoomName;

                /* Close dialog */
                PostMessage(hDlg,WM_CLOSE,1,0);
              }
              delete[] RoomName;
            }
            else
              MessageBox(hDlg, "Please select a game room from the list.", "New online game", MB_OK|MB_ICONERROR);
          }
          else
            MessageBox(hDlg, "Please enter your player name.", "New online game", MB_OK|MB_ICONERROR);
          delete[] PlayerName;
          break;
        }
        case IDCANCEL:
        {
          PostMessage(hDlg, WM_CLOSE, 0, 0);
          break;
        }
      }
      return TRUE;
    }
    case WM_CLOSE:
    {
      /* Close the dialog */
      EndDialog(hDlg, wParam);
      Dialog = NULL;
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      NetworkGameDialogValues* Values = (NetworkGameDialogValues*)lParam;
      SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)Values);

      /* Initialize controls */
      HWND Listview = GetDlgItem(hDlg, IDC_GAMEROOMLIST);
      float scaleFactor = GetDPIScaleFactor();
      char* CollumnName = new char[20];
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEROOMLIST_COL1, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 0, (int)(25*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEROOMLIST_COL2, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 1, (int)(300*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEROOMLIST_COL3, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 2, (int)(75*scaleFactor));
      delete[] CollumnName;
      SendMessage(Listview, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

      /* Set initial state */
      SetWindowText(GetDlgItem(hDlg,IDC_PLAYERNAME), Values->PlayerName.c_str());
      PostMessage(GetDlgItem(hDlg,IDC_PLAYERNAME), EM_SETSEL, (WPARAM)-1, 0);
      SendMessage(GetParent(hDlg), WM_UPDATEROOMLIST, 0, 0);

      Dialog = hDlg;
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

HWND GetNetworkDialogHandle()
{
  return Dialog;
}

int ShowNetworkGameDialog(HINSTANCE Instance, HWND hParent, NetworkGameDialogValues* Values)
{
  return DialogBoxParam(Instance,MAKEINTRESOURCE(IDD_NETWORKGAME), hParent, (DLGPROC)NetworkGameDialogProc, (LPARAM)Values);
}
