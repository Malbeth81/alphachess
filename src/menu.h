/*
* Menu.h
*
* Copyright (C) 2007-2008 Marc-André Lamothe.
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
#ifndef MENU_H_
#define	MENU_H_

#include <winutils.h>
#include <windows.h>
#include "resource.h"

/* Sub menus */
HMENU GameMenu();
HMENU EditMenu();
HMENU ViewMenu();
HMENU HelpMenu();

/* Menus */
HMENU MainMenu();

void __stdcall MenuProc(WORD ID);

#endif
