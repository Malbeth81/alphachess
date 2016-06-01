/*
* SavedGamesDialog.h
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
#ifndef SAVEDGAMESDIALOG_H_
#define SAVEDGAMESDIALOG_H_

#include "../resource.h"
#include "../system.h"
#include <commctrl.h>
#include <dialogutils.h>
#include <fstream>
#include <linkedlist.h>
#include <map>
#include <string>
#include <winutils.h>

using namespace std;

char* ShowSavedGamesDialog(HINSTANCE Instance, HWND hWindow, string GamesDirectory);

#endif
