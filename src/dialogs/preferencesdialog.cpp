/*
* PreferencesDialog.cpp
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
#include "preferencesdialog.h"

#include "themesdialog.h"
#include "../chessset.h"
#include "../chessboardtheme.h"
#include "../resource.h"
#include "../panels/chessboardpanel.h"

extern ChessBoardPanel* hChessBoardPanel;

extern LinkedList<ChessSet> ChessSets;
extern int ChessSetIndex;

extern bool AutomaticUpdate;
extern bool AlwaysOnTop;
extern bool AlwaysVisible;
extern bool AutomaticallySwitchView;
extern bool AlwaysPromoteToQueen;
extern bool PlayersInfoVisible;
extern bool CaptureListVisible;
extern bool MoveListVisible;

LinkedList<string>* GetThemes();
void LoadChessSet(int Index);
void LoadTheme(char* FileName);
void SetAlwaysOnTop(bool Value);
void ShowPlayersInfo(bool Value);
void ShowCaptureList(bool Value);
void ShowMoveList(bool Value);

// PRIVATE FUNCTIONS -----------------------------------------------------------

static void ListThemes(HWND hDlg, int ControlID)
{
  LinkedList<string>* Themes = GetThemes();
  /* Save current item selection */
  int CurSel = SendDlgItemMessage(hDlg, ControlID, CB_GETCURSEL, 0, 0);
  /* Clears the list */
  SendDlgItemMessage(hDlg, ControlID, CB_RESETCONTENT, 0, 0);
  /* Populate the list */
  for (int i=0; i < Themes->Size(); i++)
    SendDlgItemMessage(hDlg, ControlID, CB_ADDSTRING, 0, (LPARAM)Themes->Get(i)->c_str());
  /* Select previously selected item */
  if (CurSel >= 0)
    SendDlgItemMessage(hDlg, ControlID, CB_SETCURSEL, CurSel, 0);
}

static void ListChessSets(HWND hDlg, int ControlID)
{
  /* Clears the list */
  SendDlgItemMessage(hDlg, ControlID, CB_RESETCONTENT, 0, 0);
  /* Populate the combobox */
  for (int i=0; i < ChessSets.Size(); i++)
    SendDlgItemMessage(hDlg, ControlID, CB_ADDSTRING, 0, (LPARAM)ChessSets.Get(i)->FontName);
}

static void SelectComboBoxItem(HWND hDlg, int ControlID, char* Text)
{
  int Index = SendDlgItemMessage(hDlg, ControlID, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)Text);
  if (Index != CB_ERR)
    SendDlgItemMessage(hDlg, ControlID, CB_SETCURSEL, Index, 0);
}

// WINAPI FUNCTIONS ------------------------------------------------------------

static BOOL __stdcall PreferencesDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND: /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case IDC_AUTOUPDATE:
        case IDC_ALWAYSONTOP:
        case IDC_ALWAYSVISIBLE:
        case IDC_AUTOINVERTVIEW:
        case IDC_PROMOTETOQUEEN:
        {
          if (HIWORD(wParam) == BN_CLICKED)
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          break;
        }
        case IDC_SHOWCOORDINATES:
        case IDC_SHOWLASTMOVE:
        case IDC_SHOWINVALIDMOVES:
        {
          if (HIWORD(wParam) == BN_CLICKED)
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          break;
        }
        case IDC_THEME:
        case IDC_CHESSSET:
        case IDC_MOVELISTNOTATION:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          break;
        }
        case IDC_MANAGETHEMES:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            /* Get selected theme */
            int Index = SendDlgItemMessage(hDlg, IDC_THEME, CB_GETCURSEL, 0, 0);
            if (Index >= 0)
            {
              char* ThemeName = new char[MAX_PATH];
              SendDlgItemMessage(hDlg, IDC_THEME, CB_GETLBTEXT, Index, (LPARAM)ThemeName);
              /* Show the dialog */
              ShowThemesDialog(hDlg,ThemeName);
              /* Update the theme list */
              ListThemes(hDlg,IDC_THEME);
              delete[] ThemeName;
            }
          }
          break;
        }
        case IDOK:
        case IDAPPLY:
        {
          /* Apply the preferences */
          AutomaticUpdate = IsDlgButtonChecked(hDlg, IDC_AUTOUPDATE) == BST_CHECKED;
          SetAlwaysOnTop(IsDlgButtonChecked(hDlg, IDC_ALWAYSONTOP) == BST_CHECKED);
          AlwaysVisible = IsDlgButtonChecked(hDlg, IDC_ALWAYSVISIBLE) == BST_CHECKED;
          AutomaticallySwitchView = IsDlgButtonChecked(hDlg, IDC_AUTOINVERTVIEW) == BST_CHECKED;
          AlwaysPromoteToQueen = IsDlgButtonChecked(hDlg, IDC_PROMOTETOQUEEN) == BST_CHECKED;
          int Index = SendDlgItemMessage(hDlg, IDC_THEME, CB_GETCURSEL, 0, 0);
          if (Index >= 0)
          {
            char* ThemeName = new char[MAX_PATH];
            SendDlgItemMessage(hDlg, IDC_THEME, CB_GETLBTEXT, Index, (LPARAM)ThemeName);
            LoadTheme(ThemeName);
            delete[] ThemeName;
          }
          LoadChessSet(SendDlgItemMessage(hDlg, IDC_CHESSSET, CB_GETCURSEL, 0, 0));
          hChessBoardPanel->SetShowCoordinates(IsDlgButtonChecked(hDlg, IDC_SHOWCOORDINATES) == BST_CHECKED);
          hChessBoardPanel->SetShowLastMove(IsDlgButtonChecked(hDlg, IDC_SHOWLASTMOVE) == BST_CHECKED);
          hChessBoardPanel->SetShowInvalidMoves(IsDlgButtonChecked(hDlg, IDC_SHOWINVALIDMOVES) == BST_CHECKED);
          /* Close the dialog */
          if (LOWORD(wParam) == IDOK)
            SendMessage(hDlg, WM_CLOSE, 0, 0);
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
      /* Initialise the controls */
      ListThemes(hDlg,IDC_THEME);
      ListChessSets(hDlg, IDC_CHESSSET);
      PopulateComboList(hDlg, IDC_MOVELISTNOTATION, "Algebraic;Mixed;Simple");
      /* Display the preferences */
      CheckDlgButton(hDlg, IDC_AUTOUPDATE, AutomaticUpdate ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_ALWAYSONTOP, AlwaysOnTop ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_ALWAYSVISIBLE, AlwaysVisible ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_AUTOINVERTVIEW, AutomaticallySwitchView ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_PROMOTETOQUEEN, AlwaysPromoteToQueen ? BST_CHECKED : BST_UNCHECKED);
      if (SendDlgItemMessage(hDlg, IDC_THEME, CB_SELECTSTRING, (WPARAM)0, (LPARAM)hChessBoardPanel->GetTheme()->Name.c_str()) == CB_ERR)
        SetDlgItemText(hDlg, IDC_THEME, hChessBoardPanel->GetTheme()->Name.c_str());
      ChessSet* CurrentChessSet = ChessSets.Get(ChessSetIndex);
      if (CurrentChessSet != NULL)
        SelectComboBoxItem(hDlg, IDC_CHESSSET, CurrentChessSet->FontName);
      CheckDlgButton(hDlg, IDC_SHOWCOORDINATES, hChessBoardPanel->GetShowCoordinates() ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_SHOWLASTMOVE, hChessBoardPanel->GetShowLastMove() ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_SHOWINVALIDMOVES, hChessBoardPanel->GetShowInvalidMoves() ? BST_CHECKED : BST_UNCHECKED);
      return TRUE;
    }
  }
  return FALSE;
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

void ShowPreferencesDialog(HWND hParent)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));
  DialogBox(Instance,MAKEINTRESOURCE(IDD_PREFERENCES),hParent,(DLGPROC)PreferencesDialogProc);
}
