/*
* SavedGamesDialog.cpp
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
#include "savedgamesdialog.h"

// Private functions -----------------------------------------------------------

static void AddGameToList(HWND List, map<string, string> Headers)
{
  if (List != NULL)
  {
    int Index = ListView_GetItemCount(List);
    /* Add a new item to the list view */
    LVITEM ItemInfo;
    ItemInfo.mask = 0;
    ItemInfo.iItem = Index;
    ItemInfo.iSubItem = 0;
    ListView_InsertItem(List,&ItemInfo);
    /* Set the item's other attributes */
    ListView_SetItemText(List,Index,0,(CHAR*)Headers["Date"].c_str());
    ListView_SetItemText(List,Index,1,(CHAR*)Headers["White"].c_str());
    ListView_SetItemText(List,Index,2,(CHAR*)Headers["Black"].c_str());
    char* Str = timeformat(atoi(Headers["WhiteTime"].c_str()));
    ListView_SetItemText(List,Index,3,(CHAR*)Str);
    delete[] Str;
    Str = timeformat(atoi(Headers["BlackTime"].c_str()));
    ListView_SetItemText(List,Index,4,(CHAR*)Str);
    delete[] Str;
    ListView_SetItemText(List,Index,5,(CHAR*)Headers["Round"].c_str());
    ListView_SetItemText(List,Index,6,(CHAR*)Headers["Result"].c_str());
  }
}

static map<string, string> ExtractHeadersFromFile(const string FileName)
{
  map<string, string> Result;
  /* Open the file */
  ifstream File(FileName.c_str(), ios::in);
  if (File.is_open())
  {
    string Buffer;
    while (!File.eof() && !File.fail())
    {
      getline(File, Buffer);
      if (!File.fail())
      {
        /* Skip byte-order mark */
        char Bom[] = {0xEF, 0xBB, 0xBF};
        if (memcmp(Buffer.c_str(), Bom, 3) == 0)
          Buffer.erase(0,3);
        if (Buffer[0] == '[')
        {
          /* Extract header information */
          unsigned int Pos = Buffer.find(" \"");
          if (Pos != string::npos)
            Result[Buffer.substr(1,Pos-1)] = Buffer.substr(Pos+2,Buffer.length()-Pos-4);
        }
        else
          break;
      }
    }
    File.close();
  }
  return Result;
}

static void ListChessGames(string GamesDirectory, HWND List, LinkedList<string>* GameFiles)
{
  /* Get the path to the games directory */
  if (!DirectoryExists(GamesDirectory.c_str()))
    CreateDirectory(GamesDirectory.c_str(), NULL);

  /* Get the games in the folder */
  WIN32_FIND_DATA FindData;
  string FileName = GamesDirectory + "*.pgn";
  HANDLE Handle = FindFirstFile(FileName.c_str(), &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    do if (FindData.cFileName[0] != '.' && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      GameFiles->Add(new string(GamesDirectory + FindData.cFileName));
      AddGameToList(List, ExtractHeadersFromFile(GamesDirectory + FindData.cFileName));
    }
    while (FindNextFile(Handle, &FindData));
    FindClose(Handle);
  }
}

// WINAPI FUNCTIONS ------------------------------------------------------------

static INT_PTR __stdcall SavedGamesDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND: /* Process control messages */
    {
      if (lParam == 0)
      {
        switch (LOWORD(wParam))
        {
          case IDS_SAVEDGAMESMENU_LOAD:
          {
            PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_LOADGAME,0),0);
            break;
          }
          case IDS_SAVEDGAMESMENU_DELETE:
          {
            PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_DELETEGAME,0),0);
            break;
          }
        }
      }
      switch (LOWORD(wParam))
      {
        case IDC_LOADGAME:
        {
          LinkedList<string>* GameFiles = (LinkedList<string>*)GetWindowLong(hDlg, GWL_USERDATA);
          HWND Listview = GetDlgItem(hDlg, IDC_SAVEDGAMES);
          int Index = SendMessage(Listview,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED);
          string* FileName = GameFiles->Get(Index);
          char* Result = new char[FileName->length()+1];
          strcpy(Result, FileName->c_str());
          PostMessage(hDlg, WM_CLOSE, (WPARAM)Result, 0);
          break;
        }
        case IDC_DELETEGAME:
        {
          LinkedList<string>* GameFiles = (LinkedList<string>*)GetWindowLong(hDlg, GWL_USERDATA);

          if (MessageBox(hDlg,"Are you sure you want to delete this game? All data will be lost.","Delete game?",MB_YESNO|MB_ICONQUESTION) == IDYES)
          {
            HWND Listview = GetDlgItem(hDlg, IDC_SAVEDGAMES);
            int Index = SendMessage(Listview,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED);
            string* FileName = GameFiles->Remove(Index);
            DeleteFile(FileName->c_str());
            delete FileName;
            ListView_DeleteItem(Listview, Index);
          }
          break;
        }
        case IDC_LOADFROMFILE:
        {
          string FileName;
          if (OpenFileDialog(hDlg, FileName, "%USERPROFILE%\\Saved Games", "pgn", "Portable Game Notation (*.pgn)\0*.pgn\0", 0))
          {
            char* Result = new char[FileName.length()+1];
            strcpy(Result, FileName.c_str());
            PostMessage(hDlg, WM_CLOSE, (WPARAM)Result, 0);
          }
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
    case WM_CONTEXTMENU:
    {
      /* Create the menu */
      HMENU Menu = CreatePopupMenu();
      AppendMenu(Menu,IDS_SAVEDGAMESMENU_LOAD);
      AppendMenu(Menu,IDS_SAVEDGAMESMENU_DELETE);

      /* Show menu */
      TrackPopupMenu(Menu, TPM_LEFTALIGN, LOWORD(lParam), HIWORD(lParam), 0, hDlg, NULL);
      return 0;
    }
    case WM_CLOSE:
    {
      LinkedList<string>* GameFiles = (LinkedList<string>*)GetWindowLong(hDlg, GWL_USERDATA);

      /* Clean up */
      while (GameFiles->Size() > 0)
        delete GameFiles->Remove((unsigned int)0);
      delete GameFiles;

      /* Close the dialog */
      EndDialog(hDlg, wParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      char* GamesDirectory = (char *)lParam;
      LinkedList<string>* GameFiles = new LinkedList<string>();
      SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)GameFiles);

      /* Initialize controls */
      HWND Listview = GetDlgItem(hDlg, IDC_SAVEDGAMES);
      float scaleFactor = GetDPIScaleFactor();
      char* CollumnName = new char[20];
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL1, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 0, (int)(100*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL2, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 1, (int)(120*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL3, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 2, (int)(120*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL4, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 3, (int)(75*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL5, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 4, (int)(75*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL6, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 5, (int)(50*scaleFactor));
      LoadString((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), IDS_GAMEHISTORYLIST_COL7, CollumnName, 20);
      AddListViewCollumn(Listview, CollumnName, 6, (int)(50*scaleFactor));
      delete[] CollumnName;
      SendMessage(Listview, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
      ListChessGames(GamesDirectory, Listview, GameFiles);
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch(((LPNMHDR)lParam)->code)
      {
        case NM_DBLCLK:
        {
          PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_LOADGAME,0),0);
          break;
        }
        case LVN_KEYDOWN:
        {
          int Key = ((LPNMLVKEYDOWN)lParam)->wVKey;
          if (Key == VK_DELETE)
          {
            PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_DELETEGAME,0),0);
          }
          break;
        }
        case NM_RCLICK:
        {
          /* Show menu */
          POINT P;
          GetCursorPos(&P);
          PostMessage(GetParent(hDlg), WM_CONTEXTMENU, (WPARAM)hDlg, MAKELPARAM(P.x,P.y));
          break;
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

char* ShowSavedGamesDialog(HINSTANCE Instance, HWND hParent, string GamesDirectory)
{
  return (char*)DialogBoxParam(Instance,MAKEINTRESOURCE(IDD_SAVEDGAMES),hParent,(DLGPROC)SavedGamesDialogProc, (LPARAM)GamesDirectory.c_str());
}
