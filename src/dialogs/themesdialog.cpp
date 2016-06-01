/*
* ThemesDialog.cpp
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
#include "themesdialog.h"

struct ThemesDialogValues
{
  string SelectedTheme;
  bool ThemeChanged;
  string ThemesDirectory;
};


// PRIVATE FUNCTIONS -----------------------------------------------------------

static void ListThemes(HWND List, string ThemesDirectory)
{
  /* Get the path to the thenes directory */
  if (!DirectoryExists(ThemesDirectory.c_str()))
    CreateDirectory(ThemesDirectory.c_str(), NULL);

  /* Clears the list */
  SendMessage(List, LB_RESETCONTENT, 0, 0);

  /* Get the themes in the folder */
  WIN32_FIND_DATA FindData;
  string FileName = ThemesDirectory + "*.ini";
  HANDLE Handle = FindFirstFile(FileName.c_str(), &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    do if (FindData.cFileName[0] != '.' && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      char* Str = substr(FindData.cFileName, 0, strrpos(FindData.cFileName,"."));
      SendMessage(List, LB_ADDSTRING, 0, (LPARAM)Str);
      delete[] Str;
    }
    while (FindNextFile(Handle, &FindData));
    FindClose(Handle);
  }
}

static void LoadTheme(HWND hDlg, string ThemesDirectory, string Name)
{
  ChessBoardTheme Theme;

  /* Load theme from file */
  Theme.LoadFromFile(ThemesDirectory + Name + ".ini");

  /* Set controls' information */
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BORDERCOLOR), Theme.BorderColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_COORDINATESCOLOR), Theme.CoordinatesColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_WHITESQUARESCOLOR), Theme.WhiteSquaresColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKSQUARECOLOR), Theme.BlackSquaresColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_LASTMOVECOLOR), Theme.LastMovedColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_WHITEPIECESCOLOR), Theme.WhitePiecesColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKPIECESCOLOR), Theme.BlackPiecesColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_WHITEPIECESOUTLINECOLOR), Theme.WhitePiecesOutlineColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKPIECESOUTLINECOLOR), Theme.BlackPiecesOutlineColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_CURRENTSQUARECOLOR), Theme.CurrentSquareColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_SELECTEDSQUARECOLOR), Theme.SelectedSquareColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_VALIDMOVECOLOR), Theme.ValidMoveColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_INVALIDMOVECOLOR), Theme.InvalidMoveColor);
  SendDlgItemMessage(hDlg, IDC_WHITEPIECESSTYLE, CB_SETCURSEL, Theme.WhitePiecesStyle, 0);
  SendDlgItemMessage(hDlg, IDC_BLACKPIECESSTYLE, CB_SETCURSEL, Theme.BlackPiecesStyle, 0);
}

static void SaveTheme(HWND hDlg, string ThemesDirectory, string Name)
{
  ChessBoardTheme Theme;

  /* Get controls' information */
  Theme.Name = Name;
  Theme.BorderColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_BORDERCOLOR));
  Theme.CoordinatesColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_COORDINATESCOLOR));
  Theme.WhiteSquaresColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_WHITESQUARESCOLOR));
  Theme.BlackSquaresColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKSQUARECOLOR));
  Theme.LastMovedColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_LASTMOVECOLOR));
  Theme.WhitePiecesColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_WHITEPIECESCOLOR));
  Theme.BlackPiecesColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKPIECESCOLOR));
  Theme.WhitePiecesOutlineColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_WHITEPIECESOUTLINECOLOR));
  Theme.BlackPiecesOutlineColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKPIECESOUTLINECOLOR));
  Theme.CurrentSquareColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_CURRENTSQUARECOLOR));
  Theme.SelectedSquareColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_SELECTEDSQUARECOLOR));
  Theme.ValidMoveColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_VALIDMOVECOLOR));
  Theme.InvalidMoveColor = GetColorChooserColor(GetDlgItem(hDlg, IDC_INVALIDMOVECOLOR));
  Theme.WhitePiecesStyle = (ChessPieceStyle)SendMessage(GetDlgItem(hDlg, IDC_WHITEPIECESSTYLE), CB_GETCURSEL, 0, 0);
  Theme.BlackPiecesStyle = (ChessPieceStyle)SendMessage(GetDlgItem(hDlg, IDC_BLACKPIECESSTYLE), CB_GETCURSEL, 0, 0);

  /* Save theme to file */
  Theme.SaveToFile(ThemesDirectory + Name + ".ini");
}

// WINAPI FUNCTIONS ------------------------------------------------------------

static BOOL __stdcall ThemesDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND: /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case IDC_THEMES:
        {
          if (HIWORD(wParam) == LBN_SELCHANGE)
          {
            ThemesDialogValues* Values = (ThemesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

            int Index = SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCURSEL, 0, 0);
            if (Index >= 0)
            {
              /* Show confirmation */
              if (Values->ThemeChanged && MessageBox(hDlg,"Do you want to save the changes to the selected theme?","Question",MB_YESNO|MB_ICONQUESTION) == IDYES)
                SaveTheme(hDlg, Values->ThemesDirectory, Values->SelectedTheme);

              /* Load the selected theme */
              char* ThemeName = new char[MAX_PATH];
              SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETTEXT, Index, (LPARAM)ThemeName);
              Values->SelectedTheme = ThemeName;
              delete[] ThemeName;
              LoadTheme(hDlg, Values->ThemesDirectory, Values->SelectedTheme);

              /* Reset change indicator */
              Values->ThemeChanged = false;
            }
          }
          break;
        }
        case IDC_NEWTHEME:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ThemesDialogValues* Values = (ThemesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

            string Name;
            if (InputDialog((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), hDlg, "Create a new theme", "Enter a name for the new theme:", &Name, 3) && Name.length() > 0)
            {
              /* Create the new theme */
              SaveTheme(hDlg, Values->ThemesDirectory, Name);

              /* Update the theme list */
              ListThemes(GetDlgItem(hDlg, IDC_THEMES), Values->ThemesDirectory);

              /* Select the new theme */
              SelectComboBoxItem(hDlg, IDC_THEMES, Name.c_str());

              /* Enable the delete button */
              EnableWindow(GetDlgItem(hDlg,IDC_DELETETHEME), TRUE);
            }
          }
          break;
        }
        case IDC_DELETETHEME:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ThemesDialogValues* Values = (ThemesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

            int Index = SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCURSEL, 0, 0);
            if (Index >= 0 && MessageBox(hDlg,"Are you certain you want to permanently delete the selected theme?","Question",MB_YESNO|MB_ICONQUESTION) == IDYES)
            {
              /* Delete the selected theme */
              string FileName = Values->ThemesDirectory + Values->SelectedTheme + ".ini";
              DeleteFile(FileName.c_str());

              /* Update the theme list */
              ListThemes(GetDlgItem(hDlg, IDC_THEMES), Values->ThemesDirectory);

              /* Reset change indicator */
              Values->ThemeChanged = false;

              /* Select a theme if none is selected */
              int ItemCount = SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCOUNT, 0, 0);
              if (Index >= ItemCount)
              {
                SendDlgItemMessage(hDlg, IDC_THEMES, LB_SETCURSEL, ItemCount-1, 0);
                Index = ItemCount-1;
              }
              char* ThemeName = new char[MAX_PATH];
              SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETTEXT, Index, (LPARAM)ThemeName);
              Values->SelectedTheme = ThemeName;
              delete[] ThemeName;
              LoadTheme(hDlg, Values->ThemesDirectory, Values->SelectedTheme);

              /* Disable the delete button */
              if (ItemCount == 1)
                EnableWindow(GetDlgItem(hDlg,IDC_DELETETHEME), FALSE);
            }
          }
          break;
        }
        case IDC_BORDERCOLOR:
        case IDC_COORDINATESCOLOR:
        case IDC_WHITESQUARESCOLOR:
        case IDC_BLACKSQUARECOLOR:
        case IDC_LASTMOVECOLOR:
        case IDC_WHITEPIECESCOLOR:
        case IDC_BLACKPIECESCOLOR:
        case IDC_WHITEPIECESOUTLINECOLOR:
        case IDC_BLACKPIECESOUTLINECOLOR:
        case IDC_CURRENTSQUARECOLOR:
        case IDC_SELECTEDSQUARECOLOR:
        case IDC_VALIDMOVECOLOR:
        case IDC_INVALIDMOVECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ThemesDialogValues* Values = (ThemesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);
            ClickColorChooser((HWND)lParam);
            Values->ThemeChanged = true;
          }
          break;
        }
        case IDC_WHITEPIECESSTYLE:
        case IDC_BLACKPIECESSTYLE:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            ThemesDialogValues* Values = (ThemesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);
            Values->ThemeChanged = true;
          }
          break;
        }
        case IDOK:
        case IDCANCEL:
        {
          ThemesDialogValues* Values = (ThemesDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          if (Values->ThemeChanged)
          {
            int Response = MessageBox(hDlg,"Do you want to save the changes to the selected theme?","Question",MB_YESNOCANCEL|MB_ICONQUESTION);
            if (Response == IDYES)
              SaveTheme(hDlg, Values->ThemesDirectory, Values->SelectedTheme);
            else if (Response == IDCANCEL)
              break;
          }
          /* Close the dialog */
          SendMessage(hDlg, WM_CLOSE, 1, 0);
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
    case WM_DRAWITEM:
    {
      DrawColorChooser(GetDlgItem(hDlg, (UINT)wParam), (LPDRAWITEMSTRUCT)lParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      ThemesDialogValues* Values = (ThemesDialogValues*)lParam;
      SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)Values);

      /* Initialize controls */
      PopulateComboList(hDlg, IDC_WHITEPIECESSTYLE, "Plain;Outlined;Plain & Outlined");
      PopulateComboList(hDlg, IDC_BLACKPIECESSTYLE, "Plain;Outlined;Plain & Outlined");
      ApplyThemeToColorChooser(hDlg);
      ListThemes(GetDlgItem(hDlg, IDC_THEMES), Values->ThemesDirectory);
      SelectListBoxItem(hDlg, IDC_THEMES, Values->SelectedTheme.c_str());
      LoadTheme(hDlg, Values->ThemesDirectory, Values->SelectedTheme);
      return TRUE;
    }
    case WM_THEMECHANGED:
    {
      ApplyThemeToColorChooser(hDlg);
      return TRUE;
    }
  }
  return FALSE;
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

void ShowThemesDialog(HINSTANCE Instance, HWND hParent, string ThemesDirectory, string SelectedTheme)
{
  ThemesDialogValues* Values = new ThemesDialogValues;
  Values->ThemesDirectory = ThemesDirectory;
  Values->SelectedTheme = SelectedTheme;
  Values->ThemeChanged = false;
  DialogBoxParam(Instance,MAKEINTRESOURCE(IDD_THEMESDIALOG),hParent,(DLGPROC)ThemesDialogProc, (LPARAM)Values);
  delete Values;
}
