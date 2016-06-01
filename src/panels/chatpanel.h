/*
* Chatpanel.h
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
#ifndef CHATPANEL_H_
#define	CHATPANEL_H_

#include "../system.h"
#include <linkedlist.h>
#include <math.h>
#include <string>
#include <winutils.h>

#define WM_SENDMESSAGEBUTTONCLICKED WM_USER+503

using namespace std;

class ChatPanel
{
  public :
    ChatPanel(HINSTANCE hInstance, HWND hParent, RECT* R);
    ~ChatPanel();
    void AddLine(string Text);
    HWND GetHandle();
    void Invalidate();
  private :
    static ATOM ClassAtom;
    static ATOM MessagesClassAtom;
    static WNDPROC OldInputFieldProc;

    HWND Handle;
    HWND Messages;
    HWND Input;
    HWND Button;

    LinkedList<string> Lines;

    unsigned int Height;
    unsigned int LineHeight;
    unsigned int MessageHeight;
    unsigned int MessageWidth;
    unsigned int TopRow;
    unsigned int VisibleRows;
    unsigned int Width;

    /* GUI functions */
    void SendString();
    void SetVScrollBar(int Pos, int Range);
    void VScroll(int ScrollCode, int Pos);
    /* Event function */
    void Paint();
    void UpdateLineSize();
    void UpdateMessageSize(int NewWidth, int NewHeight);
    void UpdateSize(int NewWidth, int NewHeight);
    /* WinAPI functions */
    static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT __stdcall MessagesWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT __stdcall InputFieldProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
