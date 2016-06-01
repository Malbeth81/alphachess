/*
* AlphaChess.h
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
#ifndef ALPHACHESS_H_
#define ALPHACHESS_H_

#define _WIN32_WINDOWS 0x0410   // Windows 98 +
#define _WIN32_IE 0x0300 // IE 3.0 controls
#define WINVER 0x0500

#include "gameclient.h"
#include "resource.h"
#include "theme.h"
#include "chess/chessai.h"
#include "chess/chessgame.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/savedgamesdialog.h"
#include "dialogs/localgamedialog.h"
#include "dialogs/networkgamedialog.h"
#include "dialogs/preferencesdialog.h"
#include "dialogs/promotiondialog.h"
#include "panels/chatpanel.h"
#include "panels/capturepanel.h"
#include "panels/chessboardpanel.h"
#include "panels/historypanel.h"
#include "panels/roompanel.h"
#include "panels/playerpanel.h"
#include <dialogutils.h>
#include <linkedlist.h>
#include <observer.h>
#include <strutils.h>
#include <string>
#include <winutils.h>
#include <windows.h>

using namespace std;

class AlphaChess : public Observer
{
public:
  AlphaChess(HINSTANCE hInstance, HWND hParent);
  ~AlphaChess();

  HWND GetHandle();
  int ProcessAccelerators(MSG* Message);
private:
  static ATOM ClassAtom;
  static string ClassName;

  static string ApplicationPath;
  static string ConfigFileName;
  static string FontsDirectory;
  static string GamesDirectory;
  static string ThemesDirectory;

  HWND Handle;
  HACCEL Accelerators;
  HCURSOR Cursor;
  HMENU GameMenu;
  HMENU HelpMenu;
  HMENU MainMenu;
  HMENU ViewMenu;

  ChessGame* Game;
  GameClient* NetworkClient;

  ChessBoardPanel* hChessBoardPanel;
  PlayerPanel* hWhitePlayerPanel;
  PlayerPanel* hBlackPlayerPanel;
  CapturePanel* hCapturePanel;
  HistoryPanel* hHistoryPanel;
  RoomPanel* hRoomPanel;
  ChatPanel* hChatPanel;

  bool Minimized;
  bool Maximized;
  int InnerWidth;
  int InnerHeight;
  int Left;
  int Top;
  int Width;
  int Height;

  int BottomPanelHeight;
  int LeftPanelWidth;
  int MoveListMinimumHeight;
  int PlayerPanelHeight;
  bool NetworkPanelsVisible;
  bool CaptureListAutoHide;

  LinkedList<ChessSet> ChessSets;
  bool BlackPlayerIsComputer;
  string BlackPlayerName;
  ChessGameMode GameMode;
  bool NetworkGame;
  string NetworkPlayerName;
  unsigned int TimePerMove;
  bool WhitePlayerIsComputer;
  string WhitePlayerName;
  unsigned long LastTickCount;

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

  HWND SendNetworkRoomsTo;

  void ApplyPreferences();
  void CalculateBlackPlayerPosition(RECT* Rect);
  void CalculateCaptureListPosition(RECT* Rect);
  void CalculateChatPosition(RECT* Rect);
  void CalculateChessBoardPosition(RECT* Rect);
  void CalculateMoveListPosition(RECT* Rect);
  void CalculateRoomPosition(RECT* Rect);
  void CalculateWhitePlayerPosition(RECT* Rect);
  void ChangeCursor(HCURSOR NewCursor);
  ChessSet* FindChessSet(string Name);
  void LoadChessSet(ChessSet* Set);
  bool LoadGame();
  void LoadSettings();
  void LoadTheme(string ThemeName);
  void RotateView(bool Value);
  bool SaveGame();
  void SaveSettings();
  void ShowCapturePanel(bool Value);
  void ShowHistoryPanel(bool Value);
  void ShowNetworkPanels(bool Value);
  void ShowPlayerPanels(bool Value);
  void Update(const Observable* object, int Event);
  void UpdateInterface(WPARAM wParam, LPARAM lParam);
  void UpdatePos(int NewLeft, int NewTop);
  void UpdateSize(int NewWidth, int NewHeight);
  void UpdateState();
  void UpdateTime(const unsigned long TickCount);

  static LRESULT __stdcall WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
