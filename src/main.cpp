/*
* Main.cpp
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
#include "alphachess.h"

char DefaultFontName[] = "MS Shell Dlg 2";

// WinAPI functions ------------------------------------------------------------

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR cmdLine, int Show)
{
  MSG Message;

  /* Create the main window */
  AlphaChess* alphachess = new AlphaChess(hInstance, HWND_DESKTOP);
  if (alphachess != NULL)
  {
    /* Start the message loop */
    while (GetMessage(&Message, NULL, 0, 0) != 0)
    {
      /* Process keyboard shortcuts */
      if (alphachess->ProcessAccelerators(&Message) == 0)
      {
        /* Process other messages */
        TranslateMessage(&Message);
        DispatchMessage(&Message);
      }
    }
  }
  /* Clean up */
  delete alphachess;
  return 0;
}
