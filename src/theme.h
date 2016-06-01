/*
* Theme.h
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
#ifndef THEME_H_
#define THEME_H_

#include <string>
#include <winutils.h>
#include <windows.h>

using namespace std;

enum ChessPieceStyle {Plain, Outline, Both};

struct ChessSet
{
  string FileName;
  string FontName;
  string Letters;
};

class ChessBoardTheme
{
public:
  string Name;
  COLORREF BorderColor;
  COLORREF CoordinatesColor;
  COLORREF WhiteSquaresColor;
  COLORREF BlackSquaresColor;
  COLORREF LastMovedColor;
  COLORREF WhitePiecesColor;
  COLORREF BlackPiecesColor;
  COLORREF WhitePiecesOutlineColor;
  COLORREF BlackPiecesOutlineColor;
  COLORREF CurrentSquareColor;
  COLORREF SelectedSquareColor;
  COLORREF ValidMoveColor;
  COLORREF InvalidMoveColor;
  ChessPieceStyle WhitePiecesStyle;
  ChessPieceStyle BlackPiecesStyle;

  ChessBoardTheme()
  {
    BorderColor = 0x606060;
    CoordinatesColor = 0xFFFFFF;
    WhiteSquaresColor = 0xA0A0A0;
    BlackSquaresColor = 0x808080;
    LastMovedColor = 0xDD4030;
    WhitePiecesColor = 0xEEEEEE;
    BlackPiecesColor = 0x222222;
    WhitePiecesOutlineColor = 0x222222;
    BlackPiecesOutlineColor = 0xEEEEEE;
    CurrentSquareColor = 0x00FFFF;
    SelectedSquareColor = 0xFF0000;
    ValidMoveColor = 0x00FF00;
    InvalidMoveColor = 0x0000FF;
    WhitePiecesStyle = Plain;
    BlackPiecesStyle = Plain;
  }

  void LoadFromFile(string Filename)
  {
    char Section[] = "Theme";
    /* Get the theme's name from the file name */
    Name = Filename.substr(Filename.find_last_of("\\")+1, Filename.find_last_of("."));
    /* Load the theme info from the file */
    BorderColor = GetPrivateProfileInt(Section, "BorderColor", 0, Filename.c_str());
    CoordinatesColor = GetPrivateProfileInt(Section, "CoordinatesColor", 0, Filename.c_str());
    WhiteSquaresColor = GetPrivateProfileInt(Section, "WhiteSquaresColor", 0, Filename.c_str());
    BlackSquaresColor = GetPrivateProfileInt(Section, "BlackSquaresColor", 0, Filename.c_str());
    LastMovedColor = GetPrivateProfileInt(Section, "LastMovedColor", 0, Filename.c_str());
    WhitePiecesColor = GetPrivateProfileInt(Section, "WhitePiecesColor", 0, Filename.c_str());
    BlackPiecesColor = GetPrivateProfileInt(Section, "BlackPiecesColor", 0, Filename.c_str());
    WhitePiecesOutlineColor = GetPrivateProfileInt(Section, "WhitePiecesOutlineColor", 0, Filename.c_str());
    BlackPiecesOutlineColor = GetPrivateProfileInt(Section, "BlackPiecesOutlineColor", 0, Filename.c_str());
    CurrentSquareColor = GetPrivateProfileInt(Section, "CurrentSquareColor", 0, Filename.c_str());
    SelectedSquareColor = GetPrivateProfileInt(Section, "SelectedSquareColor", 0, Filename.c_str());
    ValidMoveColor = GetPrivateProfileInt(Section, "ValidMoveColor", 0, Filename.c_str());
    InvalidMoveColor = GetPrivateProfileInt(Section, "InvalidMoveColor", 0, Filename.c_str());
    WhitePiecesStyle = (ChessPieceStyle)GetPrivateProfileInt(Section, "WhitePiecesStyle", 0, Filename.c_str());
    BlackPiecesStyle = (ChessPieceStyle)GetPrivateProfileInt(Section, "BlackPiecesStyle", 0, Filename.c_str());
  }

  void SaveToFile(string Filename)
  {
    char Section[] = "Theme";
    /* Save the theme info to the file */
    WritePrivateProfileInteger(Section, "BorderColor", BorderColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "CoordinatesColor", CoordinatesColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "WhiteSquaresColor", WhiteSquaresColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "BlackSquaresColor", BlackSquaresColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "LastMovedColor", LastMovedColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "WhitePiecesColor", WhitePiecesColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "BlackPiecesColor", BlackPiecesColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "WhitePiecesOutlineColor", WhitePiecesOutlineColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "BlackPiecesOutlineColor", BlackPiecesOutlineColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "CurrentSquareColor", CurrentSquareColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "SelectedSquareColor", SelectedSquareColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "ValidMoveColor", ValidMoveColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "InvalidMoveColor", InvalidMoveColor, Filename.c_str());
    WritePrivateProfileInteger(Section, "WhitePiecesStyle", WhitePiecesStyle, Filename.c_str());
    WritePrivateProfileInteger(Section, "BlackPiecesStyle", BlackPiecesStyle, Filename.c_str());
  }
};

#endif
