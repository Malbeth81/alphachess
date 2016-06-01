/*
* PromotionDialog.cpp - A dialog that allows to choose between 4 piece type.
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
#include "promotiondialog.h"

static ChessSet* CurSet;
static ChessPieceType SelectedType;

// PRIVATE FUNCTIONS -----------------------------------------------------------

static void DrawChessPiece(HDC DC, int X, int Y, int Piece)
{
  if (CurSet != NULL)
  {
    /* Set the style */
    SetBkMode(DC,TRANSPARENT);
    SetTextColor(DC, GetSysColor(COLOR_BTNTEXT));
    HFONT OldFont = (HFONT)SelectObject(DC,EasyCreateFont(DC,CurSet->FontName.c_str(),32,0));
    /* Draw the piece */
    SIZE Size;
    GetTextExtentPoint32(DC,CurSet->Letters.substr(Piece,1).c_str(),1,&Size);
    TextOut(DC,X-Size.cx/2,Y-Size.cy/2,CurSet->Letters.substr(Piece,1).c_str(),1);
    /* Clean up */
    SetBkMode(DC,OPAQUE);
    DeleteObject(SelectObject(DC,OldFont));
  }
}

// WINAPI FUNCTIONS ------------------------------------------------------------

static INT_PTR __stdcall PromotionDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      /* Process control messages */
      switch (LOWORD(wParam))
      {
        case IDC_KNIGHT:
        {
          SelectedType = Knight;
          SendMessage(hDlg, WM_CLOSE, 1, 0);
          break;
        }
        case IDC_BISHOP:
        {
          SelectedType = Bishop;
          SendMessage(hDlg, WM_CLOSE, 1, 0);
          break;
        }
        case IDC_ROOK:
        {
          SelectedType = Rook;
          SendMessage(hDlg, WM_CLOSE, 1, 0);
          break;
        }
        case IDC_QUEEN:
        {
          SelectedType = Queen;
          SendMessage(hDlg, WM_CLOSE, 1, 0);
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
    case WM_DRAWITEM:
    {
      DRAWITEMSTRUCT* DrawStruct = (LPDRAWITEMSTRUCT)lParam;
      /* Draw the button */
      DrawCustomButton(GetDlgItem(hDlg, (UINT)wParam), (LPDRAWITEMSTRUCT)lParam);
      /* Draw the chess piece */
      DrawChessPiece(DrawStruct->hDC,DrawStruct->rcItem.left+(DrawStruct->rcItem.right-DrawStruct->rcItem.left)/2,DrawStruct->rcItem.top+(DrawStruct->rcItem.bottom-DrawStruct->rcItem.top)/2,(wParam == IDC_QUEEN ? 1 : (wParam == IDC_ROOK ? 2 : (wParam == IDC_BISHOP ? 3 : 4))));
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      ApplyThemeToCustomButton(hDlg);
      return TRUE;
    }
    case WM_THEMECHANGED:
    {
      ApplyThemeToCustomButton(hDlg);
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

ChessPieceType ShowPromotionDialog(HINSTANCE Instance, HWND hParent, ChessSet* Set)
{
  CurSet = Set;
  SelectedType = Queen;
  DialogBox(Instance, MAKEINTRESOURCE(IDD_PROMOTION), hParent, (DLGPROC)PromotionDialogProc);
  return SelectedType;
}
