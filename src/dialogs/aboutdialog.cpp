/*
* AboutDialog.cpp - A dialog showing information about the application.
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
#include "aboutdialog.h"

static HCURSOR LinkCursor = LoadCursor(NULL, MAKEINTRESOURCE(32649));
static LONG DefaultStaticWndProc;

// WINAPI FUNCTIONS ------------------------------------------------------------

static LRESULT __stdcall StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      char* Str = GetWindowText(hWnd);
      ShellExecute(NULL, "open", Str, NULL, NULL, SW_SHOW);
      delete[] Str;
      /* Fall through */
    }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MOUSEMOVE:
    {
      SetCursor(LinkCursor);
      return FALSE;
    }
  }
  return CallWindowProc((WNDPROC)DefaultStaticWndProc, hWnd, Msg, wParam, lParam);
}

static INT_PTR __stdcall AboutDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  HDC DC;
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      /* Process control messages */
      switch (LOWORD(wParam))
      {
        case IDCANCEL:        // Generic message sent by IsDialogMessage for Esc key
        case IDOK:
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
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Initialize controls */
      DC = GetDC(hDlg);
      SendDlgItemMessage(hDlg, IDC_APPLICATIONNAME, WM_SETFONT, (WPARAM)EasyCreateFont(DC, "MS Shell Dlg", 14, fsBold), 0);
      SendDlgItemMessage(hDlg, IDC_APPLICATIONVERSION, WM_SETFONT, (WPARAM)EasyCreateFont(DC, "MS Shell Dlg", 9, fsBold), 0);
      DefaultStaticWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_URL), GWL_WNDPROC, (LONG)&StaticWndProc);
      SendDlgItemMessage(hDlg, IDC_URL, WM_SETFONT, (WPARAM)EasyCreateFont(DC, "MS Shell Dlg", 8, fsUnderline), 0);
      ReleaseDC(hDlg, DC);
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

void ShowAboutDialog(HINSTANCE Instance, HWND hParent)
{
  DialogBox(Instance, MAKEINTRESOURCE(IDD_ABOUT), hParent, (DLGPROC)AboutDialogProc);
}
