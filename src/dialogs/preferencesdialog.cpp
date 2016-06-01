/*
* PreferencesDialog.cpp
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
#include "preferencesdialog.h"

// PRIVATE FUNCTIONS -----------------------------------------------------------

static void ListThemes(HWND List, string ThemesDirectory)
{
  /* Get the path to the thenes directory */
  if (!DirectoryExists(ThemesDirectory.c_str()))
    CreateDirectory(ThemesDirectory.c_str(), NULL);

  /* Save current item selection */
  int CurSel = SendMessage(List, CB_GETCURSEL, 0, 0);

  /* Clears the list */
  SendMessage(List, CB_RESETCONTENT, 0, 0);

  /* Get the themes in the folder */
  WIN32_FIND_DATA FindData;
  string FileName = ThemesDirectory + "*.ini";
  HANDLE Handle = FindFirstFile(FileName.c_str(), &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    do if (FindData.cFileName[0] != '.' && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      char* Str = substr(FindData.cFileName, 0, strrpos(FindData.cFileName,"."));
      SendMessage(List, CB_ADDSTRING, 0, (LPARAM)Str);
      delete[] Str;
    }
    while (FindNextFile(Handle, &FindData));
    FindClose(Handle);
  }

  /* Select previously selected item */
  if (CurSel >= 0)
    SendMessage(List, CB_SETCURSEL, CurSel, 0);
}

static void ListChessSets(HWND List, LinkedList<ChessSet>* ChessSets)
{
  /* Clears the list */
  SendMessage(List, CB_RESETCONTENT, 0, 0);
  /* Populate the combobox */
  for (unsigned int i=0; i < ChessSets->Size(); i++)
    SendMessage(List, CB_ADDSTRING, 0, (LPARAM)ChessSets->Get(i)->FontName.c_str());
}

// WINAPI FUNCTIONS ------------------------------------------------------------

static BOOL __stdcall PreferencesDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDC_AUTOUPDATE:
        case IDC_ALWAYSONTOP:
        case IDC_ALWAYSVISIBLE:
        case IDC_AUTOINVERTVIEW:
        case IDC_PROMOTETOQUEEN:
        case IDC_SHOWCOORDINATES:
        case IDC_SHOWLASTMOVE:
        case IDC_SHOWINVALIDMOVES:
        case IDC_SHOWMOVELISTICONS:
        {
          if (HIWORD(wParam) == BN_CLICKED)
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          break;
        }
        case IDC_THEME:
        case IDC_CHESSSET:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          break;
        }
        case IDC_MANAGETHEMES:
        {
          PreferencesDialogValues* Values = (PreferencesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          if (HIWORD(wParam) == BN_CLICKED)
          {
            /* Get selected theme */
            int Index = SendDlgItemMessage(hDlg, IDC_THEME, CB_GETCURSEL, 0, 0);
            if (Index >= 0)
            {
              char* ThemeName = new char[MAX_PATH];
              SendDlgItemMessage(hDlg, IDC_THEME, CB_GETLBTEXT, Index, (LPARAM)ThemeName);
              /* Show the dialog */
              ShowThemesDialog((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), hDlg, Values->ThemesDirectory, ThemeName);
              /* Update the theme list */
              ListThemes(GetDlgItem(hDlg,IDC_THEME), Values->ThemesDirectory);
              delete[] ThemeName;
            }
          }
          break;
        }
        case IDOK:
        case IDAPPLY:
        {
          PreferencesDialogValues* Values = (PreferencesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          /* Read controls' information */
          Values->AutomaticUpdate = IsDlgButtonChecked(hDlg, IDC_AUTOUPDATE) == BST_CHECKED;
          Values->AlwaysOnTop = IsDlgButtonChecked(hDlg, IDC_ALWAYSONTOP) == BST_CHECKED;
          Values->AlwaysVisible = IsDlgButtonChecked(hDlg, IDC_ALWAYSVISIBLE) == BST_CHECKED;
          Values->AutomaticallySwitchView = IsDlgButtonChecked(hDlg, IDC_AUTOINVERTVIEW) == BST_CHECKED;
          Values->AlwaysPromoteToQueen = IsDlgButtonChecked(hDlg, IDC_PROMOTETOQUEEN) == BST_CHECKED;
          int Index = SendDlgItemMessage(hDlg, IDC_THEME, CB_GETCURSEL, 0, 0);
          if (Index >= 0)
          {
            char* Str = new char[256];
            SendDlgItemMessage(hDlg, IDC_THEME, CB_GETLBTEXT, Index, (LPARAM)Str);
            Values->CurrentTheme = Str;
            delete[] Str;
          }
          Index = SendDlgItemMessage(hDlg, IDC_CHESSSET, CB_GETCURSEL, 0, 0);
          if (Index >= 0)
          {
            char* Str = new char[256];
            SendDlgItemMessage(hDlg, IDC_CHESSSET, CB_GETLBTEXT, Index, (LPARAM)Str);
            Values->CurrentChessSet = Str;
            delete[] Str;
          }
          Values->ShowMoveListIcons = IsDlgButtonChecked(hDlg, IDC_SHOWMOVELISTICONS) == BST_CHECKED;
          Values->BoardCoordinatesVisible = IsDlgButtonChecked(hDlg, IDC_SHOWCOORDINATES) == BST_CHECKED;
          Values->BoardLastMoveVisible = IsDlgButtonChecked(hDlg, IDC_SHOWLASTMOVE) == BST_CHECKED;
          Values->BoardInvalidMovesVisible = IsDlgButtonChecked(hDlg, IDC_SHOWINVALIDMOVES) == BST_CHECKED;

          /* Close the dialog */
          if (LOWORD(wParam) == IDAPPLY)
            SendMessage(GetParent(hDlg), WM_APPLYPREFERENCES, (WPARAM)Values, 0);
          else
            SendMessage(hDlg, WM_CLOSE, 1, 0);
          break;
        }
        case IDCANCEL:
        {
          /* Close the dialog */
          SendMessage(hDlg, WM_CLOSE, 0, 0);
          break;
        }
      }
      return TRUE;
    }
    case WM_CLOSE:
    {
      /* Close the dialog */
      EndDialog(hDlg, wParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      PreferencesDialogValues* Values = (PreferencesDialogValues*)lParam;
      SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)Values);

      /* Initialize controls */
      ListThemes(GetDlgItem(hDlg,IDC_THEME), Values->ThemesDirectory);
      ListChessSets(GetDlgItem(hDlg, IDC_CHESSSET), Values->ChessSets);

      /* Set initial state */
      CheckDlgButton(hDlg, IDC_AUTOUPDATE, Values->AutomaticUpdate ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_ALWAYSONTOP, Values->AlwaysOnTop ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_ALWAYSVISIBLE, Values->AlwaysVisible ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_AUTOINVERTVIEW, Values->AutomaticallySwitchView ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_PROMOTETOQUEEN, Values->AlwaysPromoteToQueen ? BST_CHECKED : BST_UNCHECKED);
      SelectComboBoxItem(hDlg, IDC_THEME, Values->CurrentTheme.c_str());
      SelectComboBoxItem(hDlg, IDC_CHESSSET, Values->CurrentChessSet.c_str());
      CheckDlgButton(hDlg, IDC_SHOWMOVELISTICONS, Values->ShowMoveListIcons ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_SHOWCOORDINATES, Values->BoardCoordinatesVisible ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_SHOWLASTMOVE, Values->BoardLastMoveVisible ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_SHOWINVALIDMOVES, Values->BoardInvalidMovesVisible ? BST_CHECKED : BST_UNCHECKED);
      return TRUE;
    }
  }
  return FALSE;
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

int ShowPreferencesDialog(HINSTANCE Instance, HWND hParent, PreferencesDialogValues* Values)
{
  return DialogBoxParam(Instance,MAKEINTRESOURCE(IDD_PREFERENCES),hParent,(DLGPROC)PreferencesDialogProc, (LPARAM)Values);
}
