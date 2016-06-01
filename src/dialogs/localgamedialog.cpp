/*
* LocalGameDialog.cpp
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
#include "localgamedialog.h"

INT_PTR __stdcall LocalGameDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDC_GAMETYPE:
        {
          int Index = SendDlgItemMessage(hDlg, IDC_GAMETYPE, CB_GETCURSEL, 0, 0);
          EnableWindow(GetDlgItem(hDlg,IDC_MOVETIME), Index == 1 ? TRUE : FALSE);
          break;
        }
        case IDC_WHITEPLAYER:
        {
          int Index = SendDlgItemMessage(hDlg, IDC_WHITEPLAYER, CB_GETCURSEL, 0, 0);
          if (Index == 1)
            SendDlgItemMessage(hDlg, IDC_BLACKPLAYER, CB_SETCURSEL, Index, 0);
          break;
        }
        case IDC_BLACKPLAYER:
        {
          int Index = SendDlgItemMessage(hDlg, IDC_BLACKPLAYER, CB_GETCURSEL, 0, 0);
          if (Index == 1)
            SendDlgItemMessage(hDlg, IDC_WHITEPLAYER, CB_SETCURSEL, Index, 0);
          break;
        }
        case IDCANCEL:
        {
          PostMessage(hDlg,WM_CLOSE,0,0);
          break;
        }
        case IDOK:
        {
          LocalGameDialogValues* Values = (LocalGameDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          /* Read controls' information */
          char* BlackPlayerName = GetWindowText(GetDlgItem(hDlg,IDC_BLACKPLAYERNAME));
          char* WhitePlayerName = GetWindowText(GetDlgItem(hDlg,IDC_WHITEPLAYERNAME));
          if (strlen(WhitePlayerName) > 0)
          {
            if (strlen(BlackPlayerName) > 0)
            {
              Values->BlackPlayerName.assign(BlackPlayerName);
              Values->WhitePlayerName.assign(WhitePlayerName);
              Values->WhitePlayerIsComputer = (SendDlgItemMessage(hDlg, IDC_WHITEPLAYER, CB_GETCURSEL, 0, 0) > 0);
              Values->BlackPlayerIsComputer = (SendDlgItemMessage(hDlg, IDC_BLACKPLAYER, CB_GETCURSEL, 0, 0) > 0);
              Values->GameMode = SendDlgItemMessage(hDlg, IDC_GAMETYPE, CB_GETCURSEL, 0, 0);
              char* TimePerMove = GetWindowText(GetDlgItem(hDlg,IDC_MOVETIME));
              Values->TimePerMove = (unsigned int)atoi(TimePerMove);
              delete[] TimePerMove;
              /* Close dialog */
              PostMessage(hDlg,WM_CLOSE,1,0);
            }
            else
              MessageBox(hDlg, "Please enter the black player's name.", "New local game", MB_OK|MB_ICONERROR);
          }
          else
            MessageBox(hDlg, "Please enter the white player's name.", "New local game", MB_OK|MB_ICONERROR);
          delete[] BlackPlayerName;
          delete[] WhitePlayerName;
          break;
        }
      }
      return TRUE;
    }
    case WM_CLOSE:
    {
      EndDialog(hDlg,wParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      LocalGameDialogValues* Values = (LocalGameDialogValues*)lParam;
      SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)Values);

      /* Initialize controls */
      PopulateComboList(hDlg, IDC_GAMETYPE, "Unlimited time;Time per move");
      PopulateComboList(hDlg, IDC_WHITEPLAYER, "Human;Computer");
      PopulateComboList(hDlg, IDC_BLACKPLAYER, "Human;Computer");

      /* Set initial state */
      SendDlgItemMessage(hDlg, IDC_GAMETYPE, CB_SETCURSEL, Values->GameMode, 0);
      char* Str = inttostr(Values->TimePerMove);
      SetWindowText(GetDlgItem(hDlg,IDC_MOVETIME), Str);
      delete[] Str;
      EnableWindow(GetDlgItem(hDlg,IDC_MOVETIME), Values->GameMode ? TRUE : FALSE);
      SetWindowText(GetDlgItem(hDlg,IDC_BLACKPLAYERNAME), Values->BlackPlayerName.c_str());
      SetWindowText(GetDlgItem(hDlg,IDC_WHITEPLAYERNAME), Values->WhitePlayerName.c_str());
      SendDlgItemMessage(hDlg, IDC_WHITEPLAYER, CB_SETCURSEL, Values->BlackPlayerIsComputer ? 1 : 0, 0);
      SendDlgItemMessage(hDlg, IDC_BLACKPLAYER, CB_SETCURSEL, Values->WhitePlayerIsComputer ? 1 : 0, 0);
      return TRUE;
    }
  }
  return FALSE;
}

int ShowLocalGameDialog(HINSTANCE Instance, HWND hParent, LocalGameDialogValues* Values)
{
  return DialogBoxParam(Instance,MAKEINTRESOURCE(IDD_LOCALGAME),hParent,(DLGPROC)LocalGameDialogProc, (LPARAM)Values);
}
