/*
* NetworkGameDialog.h
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
#ifndef NETWORKGAMEDIALOG_H_
#define NETWORKGAMEDIALOG_H_

#define _WIN32_IE 0x0300 // IE 3.0 controls
#include <winutils.h>
#include <windows.h>
#include <commctrl.h>
#include "../gameclient.h"

void AddRoomToList(const RoomInfo* Room);
void CloseNetworkGameDialog();
void ShowNetworkGameDialog(HWND hWindow);

#endif
