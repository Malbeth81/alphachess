/*
* GameClient.h
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
#ifndef GAMECLIENT_H_
#define GAMECLIENT_H_

#include <linkedlist.h>
#include <string>
#include <tcpclientsocket.h>
#include <windows.h>
#include "../../alphachessserver/src/gamedata.h"

using namespace std;

#define WM_UPDATEINTERFACE WM_USER+500

/* Value of the lower word value of the wParam parameter of the WM_UPDATEINTERFACE message */
enum InterfaceUpdate
{
  Disconnected, ConnectionSucceeded, ConnectionFailed, GameDataReceived, GameDataRequested, HostChanged,
  MessageReceived, MoveReceived, NotificationReceived, PlayerJoined, PlayerLeft, PlayerNameChanged,
  PlayerStateChanged, PlayerTimeChanged, PlayerTypeChanged, PromotionReceived, RequestReceived, RoomInfoReceived
};

/* Information on a room */
struct RoomInfo
{
  unsigned int RoomId;
  string Name;
  bool Locked;
  int PlayerCount;
};

/* Information on a player */
struct PlayerInfo
{
  unsigned int PlayerId;
  string Name;
  bool Ready;
  unsigned long Time;
};

/* Information on a message */
struct MessageInfo
{
  string PlayerName;
  string Message;
};

/* Network game client */
class GameClient
{
public:
  static const int Port;
  static const string Id;
  static const int SupportedVersion;
  static const int Version;

  GameClient(HWND hParent);
  ~GameClient();

  bool AcceptDrawRequest(const bool Value);
  bool AcceptTakebackRequest(const bool Value);
  bool Connect(string Address);
  bool CreateRoom(const string RoomName);
  bool Disconnect();
  bool GameHasEnded();
  const PlayerInfo* GetBlackPlayer();
  const PlayerInfo* GetHost();
  const PlayerInfo* GetLocalPlayer();
  const PlayerInfo* GetObserver(const int Index);
  unsigned int GetObserverCount();
  const PlayerInfo* GetWhitePlayer();
  bool IsConnected();
  bool IsInRoom();
  bool JoinRoom(const unsigned int RoomId);
  bool KickPlayer(const unsigned int PlayerId);
  bool LeaveRoom();
  bool MakeMove(const short FromX, const short FromY, const short ToX, const short ToY);
  bool PauseGame();
  bool PromoteTo(const int Type);
  bool RequestDrawGame();
  bool RequestTakeback();
  bool Resign();
  bool ResumeGame();
  bool SetPlayerName(const string Name);
  bool SetPlayerReady();
  bool SetPlayerType(const PlayerType Type);
  bool SendGameData(const void* Data, const unsigned long DataSize);
  bool SendText(const string Message);
  bool SendTime(const unsigned long Time);
  bool UpdateRoomList();
protected:
  HANDLE hThread;
  HWND hWindow;
  string RemoteHost;
  TCPClientSocket* Socket;
  bool Connected;
  bool InRoom;

  PlayerInfo* GameHost;
  PlayerInfo* LocalPlayer;
  PlayerInfo* BlackPlayer;
  PlayerInfo* WhitePlayer;
  LinkedList<PlayerInfo> Observers;

  bool CloseConnection();
  PlayerInfo* FindPlayer(unsigned int PlayerId);
  bool OpenConnection();
  bool ReceiveData();
  static unsigned long __stdcall ThreadEntry(void* arg);
};

#endif
