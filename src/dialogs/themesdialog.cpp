/*
* ThemesDialog.cpp
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
#include "themesdialog.h"

#include "inputdialog.h"
#include "../chessboardtheme.h"
#include "../resource.h"

static string CurrentTheme;
static ChessBoardTheme Theme;
static bool ThemeChanged = false;

LinkedList<string>* GetThemes();

// PRIVATE FUNCTIONS -----------------------------------------------------------

static void DeleteTheme()
{
  /* Create file name */
  char* FileName = new char[MAX_PATH];
  strcpy(FileName, "Themes\\");
  strcat(FileName, Theme.Name.c_str());
  strcat(FileName, ".ini");
  /* Delete file */
  DeleteFile(FileName);
  /* Clean up */
  delete[] FileName;
}

static void DisplayTheme(HWND hDlg)
{
  /* Display the selected theme */
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BORDERCOLOR),Theme.BorderColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_COORDINATESCOLOR),Theme.CoordinatesColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_WHITESQUARESCOLOR),Theme.WhiteSquaresColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKSQUARECOLOR),Theme.BlackSquaresColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_LASTMOVECOLOR),Theme.LastMovedColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_WHITEPIECESCOLOR),Theme.WhitePiecesColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKPIECESCOLOR),Theme.BlackPiecesColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_WHITEPIECESOUTLINECOLOR),Theme.WhitePiecesOutlineColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_BLACKPIECESOUTLINECOLOR),Theme.BlackPiecesOutlineColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_CURRENTSQUARECOLOR),Theme.CurrentSquareColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_SELECTEDSQUARECOLOR),Theme.SelectedSquareColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_VALIDMOVECOLOR),Theme.ValidMoveColor);
  SetColorChooserColor(GetDlgItem(hDlg, IDC_INVALIDMOVECOLOR),Theme.InvalidMoveColor);
  SendDlgItemMessage(hDlg, IDC_WHITEPIECESSTYLE, CB_SETCURSEL, Theme.WhitePiecesStyle, 0);
  SendDlgItemMessage(hDlg, IDC_BLACKPIECESSTYLE, CB_SETCURSEL, Theme.BlackPiecesStyle, 0);
}

static void ListThemes(HWND hDlg, int ControlID)
{
  LinkedList<string>* Themes = GetThemes();
  /* Save current item selection */
  int CurSel = SendDlgItemMessage(hDlg, ControlID, LB_GETCURSEL, 0, 0);
  /* Clears the list */
  SendDlgItemMessage(hDlg, ControlID, LB_RESETCONTENT, 0, 0);
  /* Populate the list */
  for (int i=0; i < Themes->Size(); i++)
  {
    SendDlgItemMessage(hDlg, ControlID, LB_ADDSTRING, 0, (LPARAM)Themes->Get(i)->c_str());
    if (CurSel < 0 && Themes->Get(i)->compare(CurrentTheme) == 0)
      CurSel = i;
  }
  /* Select previously selected item */
  SendDlgItemMessage(hDlg, ControlID, LB_SETCURSEL, max(0,CurSel), 0);
  /* Cleanup */
  while (Themes->Size() > 0)
    delete Themes->Remove();
  delete Themes;
}

static void LoadTheme(char* ThemeName)
{
  /* Create file name */
  char* FileName = new char[MAX_PATH];
  strcpy(FileName, "Themes\\");
  strcat(FileName, ThemeName);
  strcat(FileName, ".ini");
  /* Load theme from file */
  Theme.LoadFromFile(FileName);
  /* Clean up */
  delete[] FileName;
}

static void SaveTheme()
{
  /* Create file name */
  char* FileName = new char[MAX_PATH];
  strcpy(FileName, "Themes\\");
  strcat(FileName, Theme.Name.c_str());
  strcat(FileName, ".ini");
  /* Save theme to file */
  Theme.SaveToFile(FileName);
  /* Clean up */
  delete[] FileName;
}

static void SelectTheme(HWND hDlg, int Index)
{
  if (Index >= 0)
  {
    /* Load the selected theme */
    char* ThemeName = new char[MAX_PATH];
    SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETTEXT, Index, (LPARAM)ThemeName);
    LoadTheme(ThemeName);
    delete[] ThemeName;
    /* Display the selected theme */
    DisplayTheme(hDlg);
    /* Reset change indicator */
    ThemeChanged = false;
  }
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
            if (ThemeChanged && MessageBox(hDlg,"Do you want to save the changes to the selected theme?","Question",MB_YESNO|MB_ICONQUESTION) == IDYES)
              SaveTheme();
            /* Select theme */
            SelectTheme(hDlg,SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCURSEL, 0, 0));
          }
          break;
        }
        case IDC_NEWTHEME:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            char* Name = InputDialog(hDlg, "Create a new theme", "Enter a name for the new theme:");
            if (Name != NULL & strlen(Name) > 0)
            {
              /* Create the new theme */
              Theme.Name.assign(Name);
              SaveTheme();
              /* Update the theme list */
              ListThemes(hDlg, IDC_THEMES);
              /* Select the new theme */
              if (SendDlgItemMessage(hDlg, IDC_THEMES, CB_SELECTSTRING, (WPARAM)0, (LPARAM)Name) == CB_ERR)
                SetDlgItemText(hDlg, IDC_THEMES, Name);
              /* Enable the delete button */
              EnableWindow(GetDlgItem(hDlg,IDC_DELETETHEME), TRUE);
            }
            if (Name != NULL)
              delete[] Name;
          }
          break;
        }
        case IDC_DELETETHEME:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            int Index = SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCURSEL, 0, 0);
            if (Index >= 0 && MessageBox(hDlg,"Are you certain you want to permanently delete the selected theme?","Question",MB_YESNO|MB_ICONQUESTION) == IDYES)
            {
              /* Delete the selected theme */
              DeleteTheme();
              ThemeChanged = false;
              ListThemes(hDlg, IDC_THEMES);
              /* Select a theme if none selected */
              int ItemCount = SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCOUNT, 0, 0);
              if (Index >= ItemCount)
              {
                SendDlgItemMessage(hDlg, IDC_THEMES, LB_SETCURSEL, ItemCount-1, 0);
                Index = ItemCount-1;
              }
              SelectTheme(hDlg,Index);
              /* Disable the button */
              if (ItemCount == 1)
                EnableWindow(GetDlgItem(hDlg,IDC_DELETETHEME), FALSE);
            }
          }
          break;
        }
        case IDC_BORDERCOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.BorderColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_COORDINATESCOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.CoordinatesColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_WHITESQUARESCOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.WhiteSquaresColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_BLACKSQUARECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.BlackSquaresColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_LASTMOVECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.LastMovedColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_WHITEPIECESCOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.WhitePiecesColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_BLACKPIECESCOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.BlackPiecesColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_WHITEPIECESOUTLINECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.WhitePiecesOutlineColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_BLACKPIECESOUTLINECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.BlackPiecesOutlineColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_CURRENTSQUARECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.CurrentSquareColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_SELECTEDSQUARECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.SelectedSquareColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_VALIDMOVECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.ValidMoveColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_INVALIDMOVECOLOR:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            ClickColorChooser((HWND)lParam);
            Theme.InvalidMoveColor = GetColorChooserColor((HWND)lParam);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_WHITEPIECESSTYLE:
        {
          if (HIWORD(wParam) == BN_CLICKED);
          {
            Theme.WhitePiecesStyle = (ChessPieceStyle)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            ThemeChanged = true;
          }
          break;
        }
        case IDC_BLACKPIECESSTYLE:
        {
          if (HIWORD(wParam) == BN_CLICKED);
          {
            Theme.BlackPiecesStyle = (ChessPieceStyle)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            ThemeChanged = true;
          }
          break;
        }
        case IDCANCEL:
        {
          if (ThemeChanged)
          {
            int Response = MessageBox(hDlg,"Do you want to save the changes to the selected theme?","Question",MB_YESNOCANCEL|MB_ICONQUESTION);
            if (Response == IDYES)
              SaveTheme();
            else if (Response == IDCANCEL)
              break;
          }
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
    case WM_DRAWITEM:
    {
      DrawColorChooser(GetDlgItem(hDlg, (UINT)wParam), (LPDRAWITEMSTRUCT)lParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Initialise the controls */
      PopulateComboList(hDlg, IDC_WHITEPIECESSTYLE, "Plain;Outlined;Plain & Outlined");
      PopulateComboList(hDlg, IDC_BLACKPIECESSTYLE, "Plain;Outlined;Plain & Outlined");
      ApplyThemeToColorChooser(hDlg);
      /* Display Preferences */
      ListThemes(hDlg,IDC_THEMES);
      SelectTheme(hDlg,SendDlgItemMessage(hDlg, IDC_THEMES, LB_GETCURSEL, 0, 0));
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

void ShowThemesDialog(HWND hParent, string Theme)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));
  CurrentTheme = Theme;
  DialogBox(Instance,MAKEINTRESOURCE(IDD_THEMESDIALOG),hParent,(DLGPROC)ThemesDialogProc);
}
