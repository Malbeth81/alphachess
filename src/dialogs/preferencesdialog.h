/*
* PreferencesDialog.h - A dialog allowing the user to modify some settings.
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
#ifndef PREFERENCESDIALOG_H_
#define PREFERENCESDIALOG_H_

#include "../resource.h"
#include "../system.h"
#include "themesdialog.h"
#include <winutils.h>

#define WM_APPLYPREFERENCES WM_USER+507

struct PreferencesDialogValues
{
  LinkedList<ChessSet>* ChessSets;
  string ThemesDirectory;

  bool AutomaticUpdate;
  bool AlwaysOnTop;
  bool AlwaysVisible;
  bool AutomaticallySwitchView;
  bool AlwaysPromoteToQueen;
  bool PlayersInfoVisible;
  bool CaptureListVisible;
  bool MoveListVisible;
  bool BoardCoordinatesVisible;
  bool BoardLastMoveVisible;
  bool BoardInvalidMovesVisible;
  string CurrentChessSet;
  string CurrentTheme;
  bool ShowMoveListIcons;
};

int ShowPreferencesDialog(HINSTANCE Instance, HWND hWindow, PreferencesDialogValues* Values);

#endif
