/*
* RoomPanel.h
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
#ifndef ROOMPANEL_H_
#define	ROOMPANEL_H_

#include "../gameclient.h"
#include "../system.h"
#include <commctrl.h>
#include <string>
#include <winutils.h>

using namespace std;

#define WM_KICKPLAYERBUTTONCLICKED WM_USER+504
#define WM_LEAVEROOMBUTTONCLICKED WM_USER+505

class RoomPanel {
  public :
    RoomPanel(HINSTANCE hInstance, HWND hParent, RECT* R);
    ~RoomPanel();
    void EnableKickPlayerButton(bool Value);
    void EnableLeaveRoomButton(bool Value);
    HWND GetHandle();
    const string GetRoomName();
    void Invalidate();
    unsigned int GetSelectedPlayer();
    void SetGameClient(GameClient* Client);
    void SetRoomName(string Name);
  private:
    static ATOM ClassAtom;

    HWND Handle;
    HWND PlayerList;
    HWND LeaveRoomButton;
    HWND KickPlayerButton;

    GameClient* NetworkClient;

    int Height;
    string RoomName;
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
