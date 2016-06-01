/*
* LocalGameDialog.cpp
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
#include "localgamedialog.h"

#include "../gameclient.h"
#include "../resource.h"
#include "../chess/chessengine.h"

extern ChessEngine Chess;
extern GameClient* NetworkClient;

extern bool BlackPlayerIsComputer;
extern bool WhitePlayerIsComputer;

INT_PTR __stdcall LocalGameDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND: /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case IDC_COMPUTERBLACK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
            CheckDlgButton(hDlg, IDC_COMPUTERWHITE, BST_UNCHECKED);
          break;
        }
        case IDC_COMPUTERWHITE:
        {
          if (HIWORD(wParam) == BN_CLICKED)
            CheckDlgButton(hDlg, IDC_COMPUTERBLACK, BST_UNCHECKED);
          break;
        }
        case IDCANCEL:
        {
          SendMessage(hDlg,WM_CLOSE,0,0);
          break;
        }
        case IDOK:
        {
          /* Confirmation */
          if (Chess.GetGameState() == Started || Chess.GetGameState() == Paused)
            if (MessageBox(hDlg,"You have a game in progress, are you sure you want to start a new local game?","Start a new local game",MB_OKCANCEL|MB_ICONQUESTION) == IDCANCEL)
              break;
          /* Read controls' information */
          char* BlackPlayerName = GetWindowText(GetDlgItem(hDlg,IDC_BLACKPLAYERNAME));
          char* WhitePlayerName = GetWindowText(GetDlgItem(hDlg,IDC_WHITEPLAYERNAME));
          BlackPlayerIsComputer = IsDlgButtonChecked(hDlg, IDC_COMPUTERBLACK) == BST_CHECKED;
          WhitePlayerIsComputer = IsDlgButtonChecked(hDlg, IDC_COMPUTERWHITE) == BST_CHECKED;
          /* Disconnect from server */
          if (NetworkClient->IsConnected())
            NetworkClient->Disconnect();
          /* Create the game */
          Chess.SetPlayerName(Black,BlackPlayerName);
          Chess.SetPlayerName(White,WhitePlayerName);
          Chess.NewGame();
          Chess.SetGameMode(FreeTime,0);
          Chess.StartGame();
          /* Close dialog */
          delete[] BlackPlayerName;
          delete[] WhitePlayerName;
          SendMessage(hDlg,WM_CLOSE,0,0);
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
      /* Write control's information */
      SetWindowText(GetDlgItem(hDlg,IDC_BLACKPLAYERNAME), Chess.GetPlayer(Black)->Name.c_str());
      SetWindowText(GetDlgItem(hDlg,IDC_WHITEPLAYERNAME), Chess.GetPlayer(White)->Name.c_str());
      CheckDlgButton(hDlg, IDC_COMPUTERBLACK, BlackPlayerIsComputer ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, IDC_COMPUTERWHITE, WhitePlayerIsComputer ? BST_CHECKED : BST_UNCHECKED);
      return TRUE;
    }
  }
  return FALSE;
}

void ShowLocalGameDialog(HWND hParent)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));
  DialogBox(Instance,MAKEINTRESOURCE(IDD_LOCALGAME),hParent,(DLGPROC)LocalGameDialogProc);
}
