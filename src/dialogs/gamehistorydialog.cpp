/*
* GameHistoryDialog.cpp
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
#include "gamehistorydialog.h"

#include "../gameinfo.h"
#include "../resource.h"

extern LinkedList<GameInfo> GameHistory;

// Private functions -----------------------------------------------------------

static void AddGameToList(HWND List, const GameInfo* Game)
{
  if (Game != NULL)
  {
    int Index = ListView_GetItemCount(List);
    /* Add a new item to the list view */
    LVITEM ItemInfo;
    ItemInfo.mask = 0;
    ItemInfo.iItem = Index;
    ItemInfo.iSubItem = 0;
    ListView_InsertItem(List,&ItemInfo);
    /* Set the item's other attributes */
    ListView_SetItemText(List,Index,0,(CHAR*)Game->Datetime);
    ListView_SetItemText(List,Index,1,(CHAR*)Game->WhitePlayerName);
    ListView_SetItemText(List,Index,2,(CHAR*)Game->BlackPlayerName);
    ListView_SetItemText(List,Index,3,(CHAR*)Game->WhitePlayerTime);
    ListView_SetItemText(List,Index,4,(CHAR*)Game->BlackPlayerTime);
    ListView_SetItemText(List,Index,5,(CHAR*)Game->MoveCount);
    ListView_SetItemText(List,Index,6,(CHAR*)Game->Result);
  }
}

static void AddListCollumn(HWND List, UINT Id, int Index, int Width)
{
  LVCOLUMN CollumnInfo;
  CollumnInfo.mask = LVCF_WIDTH | LVCF_TEXT;
  CollumnInfo.cx = Width;
  CollumnInfo.pszText = new char[MAX_PATH];
  LoadString(GetModuleHandle(NULL), Id, CollumnInfo.pszText, MAX_PATH);
  ListView_InsertColumn(List, Index, &CollumnInfo);
  delete[] CollumnInfo.pszText;
}

// WINAPI FUNCTIONS ------------------------------------------------------------

static INT_PTR __stdcall GameHistoryDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND: /* Process control messages */
      switch (LOWORD(wParam))
      {
        case IDC_DELETEGAME:
        {
          HWND Listview = GetDlgItem(hDlg, IDC_GAMEHISTORYLIST);
          int Index = SendMessage(Listview,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED);
          delete GameHistory.Remove(Index);
          ListView_DeleteItem(Listview, Index);
          break;
        }
        case IDCANCEL:
        {
          SendMessage(hDlg, WM_CLOSE, 0, 0);
          break;
        }
      }
      return TRUE;
    case WM_CLOSE:
      /* Close the dialog */
      EndDialog(hDlg, wParam);
      return TRUE;
    case WM_INITDIALOG:
    {
      HWND Listview = GetDlgItem(hDlg, IDC_GAMEHISTORYLIST);
      /* Enable full row selection */
      SendMessage(Listview, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
      /* Add  columns */
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL1, 0, 100);
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL2, 1, 105);
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL3, 2, 105);
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL4, 3, 75);
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL5, 4, 75);
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL6, 5, 50);
      AddListCollumn(Listview, IDS_GAMEHISTORYLIST_COL7, 6, 125);
      /* List the games */
      for (int i = 0; i < GameHistory.Size(); i++)
        AddGameToList(Listview, GameHistory.Get(i));
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

void ShowGameHistoryDialog(HWND hParent)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));
  DialogBox(Instance,MAKEINTRESOURCE(IDD_GAMEHISTORY),hParent,(DLGPROC)GameHistoryDialogProc);
}
