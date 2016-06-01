/*
* GameClient.cpp
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
#include "gameclient.h"

#ifdef DEBUG
#include <iostream>
#endif

/* Static information */
const int GameClient::Port = 2570;
const string GameClient::Id = "AlphaChess";
const int GameClient::SupportedVersion = 403;
const int GameClient::Version = 403;

// Public functions ------------------------------------------------------------

GameClient::GameClient(HWND hParent)
{
  hThread = NULL;
  hWindow = hParent;
  Socket = new TCPClientSocket();
  Connected = false;
  InRoom = false;
  CurrentRoomName = "";

  /* Create information for the local player */
  GameHost = NULL;
  LocalPlayer = new PlayerInfo;
  LocalPlayer->PlayerId = 0;
  LocalPlayer->Ready = false;
  LocalPlayer->Time = 0;
  BlackPlayer = NULL;
  WhitePlayer = NULL;
}

GameClient::~GameClient()
{
  Disconnect();
  delete LocalPlayer;
}

bool GameClient::AcceptDrawRequest(const bool Value)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger((Value ? DrawRequestAccepted : DrawRequestRejected)))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::AcceptTakebackRequest(const bool Value)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger((Value ? TakebackRequestAccepted : TakebackRequestRejected)))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::Connect(string Address)
{
  if (!Socket->IsConnected())
  {
    RemoteHost = Address;
    /* Start the thread */
    unsigned long ThreadId;
    hThread = CreateThread(NULL,0,GameClient::ThreadEntry,(void*)this,0,&ThreadId);
    if (hThread == NULL)
      return false;
    return true;
  }
  else
    return (RemoteHost.compare(Address) == 0);
}

bool GameClient::CreateRoom(const string RoomName)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_CreateRoom))
      return false;
    if (!Socket->SendString(RoomName.c_str()))
      return false;
    CurrentRoomName = RoomName;
    return true;
  }
  else
    return false;
}

bool GameClient::Disconnect()
{
  if (Connected)
  {
    /* Send the disconnection signal */
    Socket->SendInteger(ND_Disconnection);
    /* Wait for the thread to stop */
    unsigned long ExitCode = STILL_ACTIVE;
    while (GetExitCodeThread(hThread,&ExitCode) != 0 && ExitCode == STILL_ACTIVE)
      Sleep(50);
    RemoteHost = "";
    return true;
  }
  else
    return false;
}

bool GameClient::GameHasEnded()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger(GameEnded))
      return false;
    return true;
  }
  else
    return false;
}

const PlayerInfo* GameClient::GetBlackPlayer()
{
  return BlackPlayer;
}

const PlayerInfo* GameClient::GetHost()
{
  return GameHost;
}

const PlayerInfo* GameClient::GetLocalPlayer()
{
  return LocalPlayer;
}

const PlayerInfo* GameClient::GetObserver(const int Index)
{
  return Observers.Get(Index);
}

unsigned int GameClient::GetObserverCount()
{
  return Observers.Size();
}

string GameClient::GetRoomName()
{
  return CurrentRoomName;
}

const PlayerInfo* GameClient::GetWhitePlayer()
{
  return WhitePlayer;
}

bool GameClient::IsConnected()
{
  return Connected;
}

bool GameClient::IsInRoom()
{
  return InRoom;
}

bool GameClient::JoinRoom(const unsigned int RoomId, const string RoomName)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_JoinRoom))
      return false;
    if (!Socket->SendInteger(RoomId))
      return false;
    CurrentRoomName = RoomName;
    return true;
  }
  else
    return false;
}

bool GameClient::KickPlayer(const unsigned int PlayerId)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_RemovePlayer))
      return false;
    if (!Socket->SendInteger(PlayerId))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::LeaveRoom()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_LeaveRoom))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::MakeMove(const short FromX, const short FromY, const short ToX, const short ToY)
{
  if (Connected)
  {
    unsigned long Data = MAKELONG(MAKEWORD(FromX,FromY),MAKEWORD(ToX,ToY));
    if (!Socket->SendInteger(ND_Move))
      return false;
    if (!Socket->SendInteger((long)Data))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::PauseGame()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger(GamePaused))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::PromoteTo(const int Type)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_PromoteTo))
      return false;
    if (!Socket->SendInteger(Type))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::RequestDrawGame()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_PlayerRequest))
      return false;
    if (!Socket->SendInteger(DrawRequest))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::RequestTakeback()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_PlayerRequest))
      return false;
    if (!Socket->SendInteger(TakebackRequest))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::Resign()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger(IResign))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::ResumeGame()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger(GameResumed))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::SetPlayerName(const string Name)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Name))
      return false;
    if (!Socket->SendString(Name.c_str()))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::SetPlayerReady()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Notification))
      return false;
    if (!Socket->SendInteger(IAmReady))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::SetPlayerType(const PlayerType Type)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_ChangeType))
      return false;
    if (!Socket->SendInteger(Type))
      return false;
    LocalPlayer->Ready = false;
    return true;
  }
  else
    return false;
}

bool GameClient::SendGameData(const void* Data, const unsigned long DataSize)
{
  if (Connected && Data != NULL && DataSize > 0)
  {
    if (!Socket->SendInteger(ND_GameData))
      return false;
    if (!Socket->SendInteger(DataSize))
      return false;
    if (!Socket->SendBytes(Data, DataSize))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::SendText(const string Message)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_Message))
      return false;
    if (!Socket->SendString(Message.c_str()))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::SendTime(const unsigned long Time)
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_PlayerTime))
      return false;
    if (!Socket->SendInteger(Time))
      return false;
    return true;
  }
  else
    return false;
}

bool GameClient::UpdateRoomList()
{
  if (Connected)
  {
    if (!Socket->SendInteger(ND_NetworkRequest))
      return false;
    if (!Socket->SendInteger(RoomList))
      return false;
    return true;
  }
  else
    return false;
}

// Private functions -----------------------------------------------------------

bool GameClient::CloseConnection()
{
  Socket->Close();

  /* Clean up data */
  Connected = false;
  InRoom = false;
  CurrentRoomName = "";
  if (BlackPlayer != NULL && BlackPlayer != LocalPlayer)
  {
    delete BlackPlayer;
    BlackPlayer = NULL;
  }
  if (WhitePlayer != NULL && WhitePlayer != LocalPlayer)
  {
    delete WhitePlayer;
    WhitePlayer = NULL;
  }
  while (Observers.Size() > 0)
  {
    PlayerInfo* Player = Observers.Remove();
    if (Player != LocalPlayer)
      delete Player;
  }

  /* Notify the interface */
  PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(Disconnected,0),0);
  return true;
}

PlayerInfo* GameClient::FindPlayer(unsigned int PlayerId)
{
  if (LocalPlayer != NULL && LocalPlayer->PlayerId == PlayerId)
    return LocalPlayer;
  else if (WhitePlayer != NULL && WhitePlayer->PlayerId == PlayerId)
    return WhitePlayer;
  else if (BlackPlayer != NULL && BlackPlayer->PlayerId == PlayerId)
    return BlackPlayer;
  else
  {
    PlayerInfo* Player = Observers.GetFirst();
    while (Player != NULL && Player->PlayerId != PlayerId)
      Player = Observers.GetNext();
    return Player;
  }
}

bool GameClient::OpenConnection()
{
  /* Connect to the server */
  if (Socket->Connect(RemoteHost.c_str(),Port))
  {
    /* Exchange version information */
    Socket->SendString(Id.c_str());
    Socket->SendInteger(Version);
    string RemoteId = Socket->ReceiveString();
    int RemoteVersion = Socket->ReceiveInteger();

    /* Validate the version information */
    if (Id.compare(RemoteId) == 0 && RemoteVersion >= SupportedVersion)
      return true;
    else
    {
      /* Close connection */
      Socket->Close();

      /* Update the interface */
      PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(VersionMismatch,0),0);
    }
  }
  else
  {
    /* Update the interface */
    PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(ConnectionFailed,0),0);
  }

  return false;
}

bool GameClient::ReceiveData()
{
  while (true)
  {
    long DataType = Socket->ReceiveInteger();
    switch (DataType)
    {
      case -1:
      {
        return false;
      }
      case ND_Disconnection:
      {
        return true;
      }
      case ND_GameData:
      {
        /* Read the size of the incoming data */
        long DataSize = (unsigned long)Socket->ReceiveInteger();
        if (DataSize == -1)
          return 0;
        /* Receive the incoming data */
        unsigned char* Data = new unsigned char[DataSize];
        memset(Data,sizeof(Data),0);
        if (Socket->ReceiveBytes(Data, DataSize) == (unsigned long)DataSize)
        {
          /* Update the interface */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(GameDataReceived,0),(LPARAM)Data);
        }
        break;
      }
      case ND_HostChanged:
      {
        /* Receive the id of the new host */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Player " << PlayerId << " is now the game host" << std::endl;
#endif
        /* Find the player */
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player != NULL)
        {
          /* Change the host */
          GameHost = Player;
          /* Notify the interface */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(HostChanged,0),0);
        }
        break;
      }
      case ND_Message:
      {
        /* Receive a message from a player */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
        char* Text = Socket->ReceiveString();
        if (Text == NULL)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Player " << PlayerId << " as sent a message" << std::endl;
#endif
        /* Find the player */
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player != NULL)
        {
          /* Store information on the message */
          MessageInfo* Message = new MessageInfo;
          Message->PlayerName = Player->Name;
          Message->Message = Text;
          /* Notify the interface */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(MessageReceived,0),(LPARAM)Message);
        }
        else
          delete[] Text;
        break;
      }
      case ND_Move:
      {
        /* Receive a move from a player */
        long Data = Socket->ReceiveInteger();
        if (Data == -1)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : A player has made a move" << std::endl;
#endif
        /* Notify the interface */
        PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(MoveReceived,0),Data);
        break;
      }
      case ND_Name:
      {
        /* Receive a remote player's name */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
        char* PlayerName = Socket->ReceiveString();
        if (PlayerName == NULL)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Player " << PlayerId << " as changed his name to " << PlayerName << std::endl;
#endif
        /* Find the player */
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player != NULL)
        {
          /* Update the player's name */
          Player->Name = PlayerName;
          /* Notify the interface to update the player's name */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerNameChanged,0),(LPARAM)Player);
        }
        /* Clean up */
        delete[] PlayerName;
        break;
      }
      case ND_NetworkRequest:
      {
        /* Receive a request from the server */
        NetworkRequestType Request = (NetworkRequestType)Socket->ReceiveInteger();
        if (Request == -1)
          return false;
        switch (Request)
        {
          case GameData:
          {
            /* Notify the interface to send the game data */
            PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(GameDataRequested,0),0);
            break;
          }
          default:
            break;
        }
        break;
      }
      case ND_Notification:
      {
        /* Receive a notification from the server */
        long Notification = Socket->ReceiveInteger();
        if (Notification == -1)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Notification " << Notification << std::endl;
#endif
        switch (Notification)
        {
          case JoinedRoom:
          {
            InRoom = true;
            /* Add the local player to the observer list */
            Observers.Add(LocalPlayer);
            break;
          }
          case LeftRoom:
          {
            InRoom = false;
            /* Delete the players informations */
            if (BlackPlayer != NULL)
            {
              if (BlackPlayer != LocalPlayer)
                delete BlackPlayer;
              BlackPlayer = NULL;
            }
            if (WhitePlayer != NULL)
            {
              if (WhitePlayer != LocalPlayer)
                delete WhitePlayer;
              WhitePlayer = NULL;
            }
            while (Observers.Size() > 0)
            {
              PlayerInfo* Player = Observers.Remove();
              if (Player != LocalPlayer)
                delete Player;
            }
            break;
          }
          case GameStarted:
          {
            /* Reset the ready flags */
            if (BlackPlayer != NULL)
              BlackPlayer->Ready = false;
            if (WhitePlayer != NULL)
              WhitePlayer->Ready = false;
            break;
          }
          default:
            break;
        }
        /* Notify the interface */
        PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(NotificationReceived,Notification),0);
        break;
      }
      case ND_PlayerId:
      {
        /* Receive the local player's id */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : My player id is " << PlayerId << std::endl;
#endif
        /* Store the local player's id */
        LocalPlayer->PlayerId = PlayerId;
        /* Connection is completed */
        Connected = true;
        /* Notify the interface that the connection is established */
        PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(ConnectionSucceeded,0),0);
        break;
      }
      case ND_PlayerJoined:
      {
        /* Receive a remote player's id */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
        char* PlayerName = Socket->ReceiveString();
        if (PlayerName == NULL)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Player " << PlayerId << " as joined" << std::endl;
#endif
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player == NULL)
        {
          /* Store information for the remote player */
          Player = new PlayerInfo;
          Player->PlayerId = PlayerId;
          Player->Name = PlayerName;
          Player->Ready = false;
          Player->Time = 0;
          /* Add the remote player to the list */
          Observers.Add(Player);
          /* Notify the interface that the player joined */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerJoined,0),(LPARAM)Player);
        }
        break;
      }
      case ND_PlayerLeft:
      {
        /* Receive a remote player's id */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Player " << PlayerId << " has left" << std::endl;
#endif
        char* PlayerName = NULL;
        /* Remove the player */
        if (WhitePlayer != NULL && WhitePlayer->PlayerId == (unsigned int)PlayerId)
        {
          PlayerName = new char[sizeof(WhitePlayer->Name)];
          strcpy(PlayerName,WhitePlayer->Name.c_str());
          delete WhitePlayer;
          WhitePlayer = NULL;
        }
        else if (BlackPlayer != NULL && BlackPlayer->PlayerId == (unsigned int)PlayerId)
        {
          PlayerName = new char[sizeof(BlackPlayer->Name)];
          strcpy(PlayerName,BlackPlayer->Name.c_str());
          delete BlackPlayer;
          BlackPlayer = NULL;
        }
        else
        {
          PlayerInfo* Observer = Observers.GetFirst();
          while (Observer != NULL && Observer->PlayerId != (unsigned int)PlayerId)
            Observer = Observers.GetNext();
          if (Observer != NULL)
          {
            PlayerName = new char[sizeof(Observer->Name)];
            strcpy(PlayerName,Observer->Name.c_str());
            Observers.Remove(Observer);
            delete Observer;
          }
        }
        /* Notify the interface that a player left */
        PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerLeft,0),(LPARAM)PlayerName);
        break;
      }
      case ND_PlayerReady:
      {
        /* Receive a remote player's id */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
#ifdef DEBUG
        /* Output to log */
        std::cout << "Received Data : Player " << PlayerId << " is ready" << std::endl;
#endif
        /* Find the player */
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player != NULL)
        {
          /* Update the player */
          Player->Ready = true;
          /* Notify the interface to update the players information */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerStateChanged,0),(LPARAM)Player);
        }
        break;
      }
      case ND_PlayerRequest:
      {
        /* Receive a request from a remote player */
        PlayerRequestType Request = (PlayerRequestType)Socket->ReceiveInteger();
        if (Request == -1)
          return false;
        switch (Request)
        {
          case DrawRequest:
          {
#ifdef DEBUG
            /* Output to log */
            std::cout << "Received Data : Opponent requests a draw" << std::endl;
#endif
            /* Notify the interface to display the request */
            PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(RequestReceived,Request),0);
            break;
          }
          case TakebackRequest:
          {
#ifdef DEBUG
            /* Output to log */
            std::cout << "Received Data : Opponent requests to take back his move" << std::endl;
#endif
            /* Notify the interface to display the request */
            PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(RequestReceived,Request),0);
            break;
          }
          default:
            break;
        }
        break;
      }
      case ND_PlayerTime:
      {
        /* Receive the server's time */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
        long Time = Socket->ReceiveInteger();
        if (Time == -1)
          return false;
        /* Output to log */
#ifdef DEBUG
        std::cout << "Received Data : Player " << PlayerId << " time changed to " << Time << std::endl;
#endif
        /* Find the player */
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player != NULL)
        {
          /* Update the player */
          Player->Time = Time;
          /* Notify the interface to update the players information */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerTimeChanged,0),(LPARAM)Player);
        }
        break;
      }
      case ND_PlayerType:
      {
        /* Receive a player's id and type */
        long PlayerId = Socket->ReceiveInteger();
        if (PlayerId == -1)
          return false;
        PlayerType Type = (PlayerType)Socket->ReceiveInteger();
        if (Type == -1)
          return false;
        /* Output to log */
#ifdef DEBUG
        std::cout << "Received Data : Player " << PlayerId << " has changes his type to " << Type << std::endl;
#endif
        /* Find the player */
        PlayerInfo* Player = FindPlayer(PlayerId);
        if (Player != NULL)
        {
          /* Change the player's type */
          if (Player == WhitePlayer)
            WhitePlayer = NULL;
          else if (Player == BlackPlayer)
            BlackPlayer = NULL;
          else
            Observers.Remove(Player);
          if (Type == BlackPlayerType)
            BlackPlayer = Player;
          else if (Type == WhitePlayerType)
            WhitePlayer = Player;
          else if (Type == ObserverType)
            Observers.Add(Player);
          Player->Ready = false;
          /* Notify the interface to update the player */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerTypeChanged,0),(LPARAM)Player);
          /* Notify the interface to update the player */
          PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PlayerStateChanged,0),(LPARAM)Player);
        }
        break;
      }
      case ND_PromoteTo:
      {
        /* Receive a chess piece type */
        long Type = Socket->ReceiveInteger();
        if (Type == -1)
          return false;
        /* Notify the interface to promote the last played piece */
        PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(PromotionReceived,0),(LPARAM)Type);
        break;
      }
      case ND_RoomInfo:
      {
        /* Receive information on one of the server's room */
        long RoomId = Socket->ReceiveInteger();
        if (RoomId == -1)
          return false;
        char* RoomName = Socket->ReceiveString();
        if (RoomName == NULL)
          return false;
        long RoomLocked = Socket->ReceiveInteger();
        if (RoomLocked == -1)
          return false;
        long PlayerCount = Socket->ReceiveInteger();
        if (PlayerCount == -1)
          return false;
        /* Store information on the room */
        RoomInfo* Room = new RoomInfo;
        Room->Name = RoomName;
        Room->RoomId = RoomId;
        Room->Locked = (bool)RoomLocked;
        Room->PlayerCount = PlayerCount;
        /* Notify the interface to update the room list */
        PostMessage(hWindow,WM_UPDATEINTERFACE,MAKEWPARAM(RoomInfoReceived,0),(LPARAM)Room);
        delete[] RoomName;
        break;
      }
      default:
        return false;
    }
  }
}

unsigned long __stdcall GameClient::ThreadEntry(void* arg)
{
  GameClient* Client = (GameClient*)arg;
  if (Client->OpenConnection())
  {
    Client->ReceiveData();
    Client->CloseConnection();
  }
  Client->hThread = NULL;
  return 0;
}
