/*
* Chatpanel.h
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
#ifndef CHATPANEL_H_
#define	CHATPANEL_H_

#define _WIN32_IE 0x0300 // IE 3.0 controls
#include <linkedlist.h>
#include <math.h>
#include <string>
#include <winutils.h>
#include <windows.h>

using namespace std;

typedef void (__stdcall *SENDTEXTPROC)(char* Text);

class ChatPanel
{
  public :
    SENDTEXTPROC SendTextProc;

    ChatPanel(HWND hParent, RECT* R);
    ~ChatPanel();
    void AddLine(char* Text);
    HWND GetHandle();
    void Invalidate();
  private :
    static ATOM ClassAtom;
    static WNDPROC OldInputFieldProc;

    HWND Handle;
    HWND Messages;
    HWND Input;
    HWND Button;

    LinkedList<string> Lines;

    int Height;
    int LineHeight;
    int MessageHeight;
    int MessageWidth;
    int TopRow;
    int VisibleRows;
    int Width;

    /* GUI functions */
    void SendString();
    void SetVScrollBar(int Pos, int Range);
    void VScroll(int ScrollCode, int Pos);
    /* Event function */
    void UpdateSize(int NewWidth, int NewHeight);
    void UpdateMessageSize(int NewWidth, int NewHeight);
    /* WinAPI functions */
    static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT __stdcall MessagesWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT __stdcall InputFieldProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
