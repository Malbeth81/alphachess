/*
* RoomPanel.h
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
#ifndef ROOMPANEL_H_
#define	ROOMPANEL_H_

#define _WIN32_IE 0x0300 // IE 3.0 controls
#include <winutils.h>
#include <windows.h>
#include <commctrl.h>
#include "../gameclient.h"

typedef void (__stdcall *ROOMPANELBUTTONCLICKEDPROC)();

class RoomPanel {
  public :
    ROOMPANELBUTTONCLICKEDPROC LeaveRoomButtonClickedProc;
    ROOMPANELBUTTONCLICKEDPROC KickPlayerButtonClickedProc;

    RoomPanel(HWND hParent, RECT* R);
    ~RoomPanel();
    void EnableKickPlayerButton(bool Value);
    void EnableLeaveRoomButton(bool Value);
    HWND GetHandle();
    const char* GetRoomName();
    void Invalidate();
    unsigned int GetSelectedPlayer();
    void SetRoomName(char* Name);
  private:
    static ATOM ClassAtom;

    HWND Handle;
    HWND PlayerList;
    HWND LeaveRoomButton;
    HWND KickPlayerButton;

    int Height;
    char* RoomName;
    int Width;

    /* Private functions */
    void AddPlayerToList(const PlayerInfo* Player);
    void UpdatePlayerList();
    /* Event functions */
    void UpdateSize(int NewWidth, int NewHeight);
    /* WINAPI functions */
    static LRESULT __stdcall PanelWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
