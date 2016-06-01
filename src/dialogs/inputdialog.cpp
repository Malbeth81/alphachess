/*
* InputDialog.cpp
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
#include "inputdialog.h"

struct InputDialogValues
{
  string Title;
  string Message;
  string* Value;
};

// WINAPI FUNCTIONS ------------------------------------------------------------

static BOOL __stdcall InputDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      /* Process control messages */
      switch (LOWORD(wParam))
      {
        case IDOK:
        {
          InputDialogValues* Values = (InputDialogValues*)GetWindowLong(hDlg, GWL_USERDATA);

          Values->Value->assign(GetWindowText(GetDlgItem(hDlg, IDC_INPUT)));
          PostMessage(hDlg, WM_CLOSE, 1, 0);
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
      EndDialog(hDlg, wParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      InputDialogValues* Values = (InputDialogValues*)lParam;
      SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)Values);

      /* Initialize controls */
      SetWindowText(hDlg, Values->Title.c_str());
      SetWindowText(GetDlgItem(hDlg, IDC_INPUTLBL), Values->Message.c_str());
      SetWindowText(GetDlgItem(hDlg, IDC_INPUT), Values->Value->c_str());
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

int InputDialog(HINSTANCE Instance, HWND hParent, const string Title, const string Message, string* Value)
{
  InputDialogValues* Values = new InputDialogValues;
  Values->Title = Title;
  Values->Message = Message;
  Values->Value = Value;
  int Result = DialogBoxParam(Instance, MAKEINTRESOURCE(IDD_INPUT), hParent, (DLGPROC)InputDialogProc, (LPARAM)Values);
  delete Values;
  return Result;
}
