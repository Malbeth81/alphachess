/*
* Menu.cpp
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
#include "menu.h"

void CloseApplication();
void DrawGame();
void LoadGame();
void PauseGame(bool Value);
void PlayLocalGame();
void PlayNetworkGame();
void ResignGame();
void SaveGame();
void ShowAbout();
void ShowPlayersInfo(bool Value);
void ShowPreferences();
void ShowCaptureList(bool Value);
void ShowMoveList(bool Value);
void ShowGameHistory();
void SwitchView(bool Value);
void TakebackMove();

// PUBLIC FUNCTIONS ------------------------------------------------------------

HMENU GameMenu()
{
  static HMENU hGameMenu = NULL;
  if (hGameMenu == NULL)
  {
    hGameMenu = CreatePopupMenu();
    AppendMenu(hGameMenu,IDS_MENU_GAME_LOCAL);
    AppendMenu(hGameMenu,IDS_MENU_GAME_NETWORK);
    AppendSeparator(hGameMenu);
    AppendMenu(hGameMenu,IDS_MENU_GAME_PAUSE);
    AppendSeparator(hGameMenu);
    AppendMenu(hGameMenu,IDS_MENU_GAME_TAKEBACKMOVE);
    AppendMenu(hGameMenu,IDS_MENU_GAME_OFFERDRAW);
    AppendMenu(hGameMenu,IDS_MENU_GAME_RESIGN);
    AppendSeparator(hGameMenu);
    AppendMenu(hGameMenu,IDS_MENU_GAME_EXIT);
  }
  return hGameMenu;
}

HMENU EditMenu()
{
  static HMENU hEditMenu = NULL;
  if (hEditMenu == NULL)
  {
    hEditMenu = CreatePopupMenu();
    AppendMenu(hEditMenu,IDS_MENU_EDIT_LOADGAME);
    AppendMenu(hEditMenu,IDS_MENU_EDIT_SAVEGAME);
    AppendSeparator(hEditMenu);/*
    AppendMenu(hEditMenu,IDS_MENU_EDIT_LOADPOSITION);
    AppendMenu(hEditMenu,IDS_MENU_EDIT_SAVEPOSITION);
    AppendSeparator(hEditMenu);
    AppendMenu(hEditMenu,IDS_MENU_EDIT_COPYGAME);
    AppendMenu(hEditMenu,IDS_MENU_EDIT_PASTEGAME);
    AppendSeparator(hEditMenu);
    AppendMenu(hEditMenu,IDS_MENU_EDIT_COPYPOSITION);
    AppendMenu(hEditMenu,IDS_MENU_EDIT_PASTEPOSITION);
    AppendSeparator(hEditMenu);*/
    AppendMenu(hEditMenu,IDS_MENU_EDIT_PREFERENCES);
  }
  return hEditMenu;
}

HMENU ViewMenu()
{
  static HMENU hViewMenu = NULL;
  if (hViewMenu == NULL)
  {
    hViewMenu = CreatePopupMenu();
    AppendMenu(hViewMenu,IDS_MENU_VIEW_PLAYERS);
    AppendMenu(hViewMenu,IDS_MENU_VIEW_MOVES);
    AppendMenu(hViewMenu,IDS_MENU_VIEW_CAPTURES);
    AppendSeparator(hViewMenu);
    AppendMenu(hViewMenu,IDS_MENU_VIEW_HISTORY);
    AppendMenu(hViewMenu,IDS_MENU_VIEW_SWITCH);
  }
  return hViewMenu;
}

HMENU HelpMenu()
{
  static HMENU hHelpMenu = NULL;
  if (hHelpMenu == NULL)
  {
    hHelpMenu = CreatePopupMenu();
    AppendMenu(hHelpMenu,IDS_MENU_HELP_HELP);
    AppendMenu(hHelpMenu,IDS_MENU_HELP_UPDATE);
    AppendSeparator(hHelpMenu);
    AppendMenu(hHelpMenu,IDS_MENU_HELP_ABOUT);
  }
  return hHelpMenu;
}

HMENU MainMenu()
{
  static HMENU hMainMenu = NULL;
  if (hMainMenu == NULL)
  {
    hMainMenu = CreateMenu();
    AppendMenu(hMainMenu,IDS_MENU_GAME,GameMenu());
    AppendMenu(hMainMenu,IDS_MENU_EDIT,EditMenu());
    AppendMenu(hMainMenu,IDS_MENU_VIEW,ViewMenu());
    AppendMenu(hMainMenu,IDS_MENU_HELP,HelpMenu());
  }
  return hMainMenu;
}

// WINAPI FUNCTIONS ------------------------------------------------------------

void __stdcall MenuProc(WORD ItemID)
{
  switch (ItemID)
  {
    case IDS_MENU_GAME_LOCAL:
    {
      PlayLocalGame();
    	break;
    }
    case IDS_MENU_GAME_NETWORK:
    {
      PlayNetworkGame();
      break;
    }
    case IDS_MENU_GAME_PAUSE:
    {
      PauseGame(!IsMenuItemChecked(GameMenu(),ItemID));
    	break;
    }
    case IDS_MENU_GAME_TAKEBACKMOVE:
    {
      TakebackMove();
    	break;
    }
    case IDS_MENU_GAME_OFFERDRAW:
    {
      DrawGame();
    	break;
    }
    case IDS_MENU_GAME_RESIGN:
    {
      ResignGame();
    	break;
    }
    case IDS_MENU_GAME_EXIT:
    {
      CloseApplication();
      break;
    }
    case IDS_MENU_EDIT_LOADGAME:
    {
      LoadGame();
      break;
    }
    case IDS_MENU_EDIT_SAVEGAME:
    {
      SaveGame();
      break;
    }
    case IDS_MENU_EDIT_LOADPOSITION:
    {
      break;
    }
    case IDS_MENU_EDIT_SAVEPOSITION:
    {
      break;
    }
    case IDS_MENU_EDIT_COPYGAME:
    {
      break;
    }
    case IDS_MENU_EDIT_PASTEGAME:
    {
      break;
    }
    case IDS_MENU_EDIT_PREFERENCES:
    {
      ShowPreferences();
    	break;
    }
    case IDS_MENU_VIEW_PLAYERS:
    {
      ShowPlayersInfo(!IsMenuItemChecked(ViewMenu(),ItemID));
    	break;
    }
    case IDS_MENU_VIEW_CAPTURES:
    {
      ShowCaptureList(!IsMenuItemChecked(ViewMenu(),ItemID));
      break;
    }
    case IDS_MENU_VIEW_MOVES:
    {
      ShowMoveList(!IsMenuItemChecked(ViewMenu(),ItemID));
    	break;
    }
    case IDS_MENU_VIEW_HISTORY:
    {
      ShowGameHistory();
    	break;
    }
    case IDS_MENU_VIEW_SWITCH:
    {
      SwitchView(!IsMenuItemChecked(ViewMenu(),ItemID));
    	break;
    }
    case IDS_MENU_HELP_HELP:
    {
    	break;
    }
    case IDS_MENU_HELP_UPDATE:
    {
    	break;
    }
    case IDS_MENU_HELP_ABOUT:
    {
      ShowAbout();
    	break;
    }
  }
}
