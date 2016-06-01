/*
* Ressource.h - A collection of resources like icons and dialog definition.
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
#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif
#define IDAPPLY                         0010

#define IDR_MAINACCELERATORS            0100

/* Menu item identifiers */
#define IDS_MENU_GAME                   1100
#define IDS_MENU_GAME_LOCAL             1101
#define IDS_MENU_GAME_NETWORK           1102
#define IDS_MENU_GAME_PAUSE             1103
#define IDS_MENU_GAME_RESUME            1104
#define IDS_MENU_GAME_TAKEBACKMOVE      1105
#define IDS_MENU_GAME_OFFERDRAW         1106
#define IDS_MENU_GAME_RESIGN            1107
#define IDS_MENU_GAME_EXIT              1108

#define IDS_MENU_EDIT                   1200
#define IDS_MENU_EDIT_LOADGAME          1201
#define IDS_MENU_EDIT_SAVEGAME          1202
#define IDS_MENU_EDIT_LOADPOSITION      1203
#define IDS_MENU_EDIT_SAVEPOSITION      1204
#define IDS_MENU_EDIT_COPYGAME          1205
#define IDS_MENU_EDIT_PASTEGAME         1206
#define IDS_MENU_EDIT_COPYPOSITION      1207
#define IDS_MENU_EDIT_PASTEPOSITION     1208
#define IDS_MENU_EDIT_PREFERENCES       1209

#define IDS_MENU_VIEW                   1300
#define IDS_MENU_VIEW_PLAYERS           1301
#define IDS_MENU_VIEW_CAPTURES          1302
#define IDS_MENU_VIEW_MOVES             1303
#define IDS_MENU_VIEW_HISTORY           1304
#define IDS_MENU_VIEW_SWITCH            1305

#define IDS_MENU_HELP                   1500
#define IDS_MENU_HELP_HELP              1501
#define IDS_MENU_HELP_UPDATE            1502
#define IDS_MENU_HELP_ABOUT             1503

/* List view column headers identifiers */
#define IDS_GAMEROOMLIST_COL1           1900
#define IDS_GAMEROOMLIST_COL2           1901
#define IDS_GAMEROOMLIST_COL3           1902
#define IDS_GAMEHISTORYLIST_COL1        1910
#define IDS_GAMEHISTORYLIST_COL2        1911
#define IDS_GAMEHISTORYLIST_COL3        1912
#define IDS_GAMEHISTORYLIST_COL4        1913
#define IDS_GAMEHISTORYLIST_COL5        1914
#define IDS_GAMEHISTORYLIST_COL6        1915
#define IDS_GAMEHISTORYLIST_COL7        1916
#define IDS_PLAYERLIST_COL1             1920
#define IDS_PLAYERLIST_COL2             1921

/* Dialog item identifiers */
#define IDD_ABOUT                      10000
#define IDC_APPLICATIONNAME            10001
#define IDC_COPYRIGHT                  10002
#define IDC_URL                        10003

#define IDD_INPUT                      10100
#define IDC_INPUTLBL                   10101
#define IDC_INPUT                      10102

#define IDD_PREFERENCES                10200
#define IDC_AUTOUPDATE                 10201
#define IDC_AUTOINVERTVIEW             10202
#define IDC_ALWAYSONTOP                10203
#define IDC_ALWAYSVISIBLE              10204
#define IDC_PROMOTETOQUEEN             10205
#define IDC_THEMELBL                   10206
#define IDC_THEME                      10207
#define IDC_MANAGETHEMES               10208
#define IDC_CHESSSETLBL                10209
#define IDC_CHESSSET                   10210
#define IDC_SHOWCOORDINATES            10211
#define IDC_SHOWLASTMOVE               10212
#define IDC_SHOWINVALIDMOVES           10213
#define IDC_MOVELISTNOTATIONLBL        10214
#define IDC_MOVELISTNOTATION           10215

#define IDD_THEMESDIALOG               10400
#define IDC_THEMESLBL                  10401
#define IDC_THEMES                     10402
#define IDC_NEWTHEME                   10403
#define IDC_DELETETHEME                10404
#define IDC_BORDERLBL                  10405
#define IDC_BORDERCOLOR                10406
#define IDC_WHITESQUARELBL             10407
#define IDC_WHITESQUARESCOLOR          10408
#define IDC_BLACKSQUARELBL             10409
#define IDC_BLACKSQUARECOLOR           10410
#define IDC_COORDINATESLBL             10411
#define IDC_COORDINATESCOLOR           10412
#define IDC_LASTMOVELBL                10413
#define IDC_LASTMOVECOLOR              10414
#define IDC_WHITEPIECESLBL             10415
#define IDC_WHITEPIECESCOLOR           10416
#define IDC_BLACKPIECESLBL             10417
#define IDC_BLACKPIECESCOLOR           10418
#define IDC_WHITEPIECESSTYLELBL        10419
#define IDC_WHITEPIECESSTYLE           10420
#define IDC_BLACKPIECESSTYLELBL        10421
#define IDC_BLACKPIECESSTYLE           10422
#define IDC_WHITEPIECESOUTLINELBL      10423
#define IDC_WHITEPIECESOUTLINECOLOR    10424
#define IDC_BLACKPIECESOUTLINELBL      10425
#define IDC_BLACKPIECESOUTLINECOLOR    10426
#define IDC_CURRENTSQUARELBL           10427
#define IDC_CURRENTSQUARECOLOR         10428
#define IDC_SELECTEDSQUARELBL          10429
#define IDC_SELECTEDSQUARECOLOR        10430
#define IDC_VALIDMOVELBL               10431
#define IDC_VALIDMOVECOLOR             10432
#define IDC_INVALIDMOVELBL             10433
#define IDC_INVALIDMOVECOLOR           10434

#define IDD_LOCALGAME                  10500
#define IDC_WHITEPLAYERNAMELBL         10501
#define IDC_WHITEPLAYERNAME            10502
#define IDC_COMPUTERWHITE              10503
#define IDC_BLACKPLAYERNAMELBL         10504
#define IDC_BLACKPLAYERNAME            10505
#define IDC_COMPUTERBLACK              10506
#define IDC_GAMETYPELBL                10507
#define IDC_GAMETYPE                   10508

#define IDD_NETWORKGAME                10600
#define IDC_PLAYERNAMELBL              10601
#define IDC_PLAYERNAME                 10602
#define IDC_GAMEROOMS                  10603
#define IDC_GAMEROOMLIST               10604
#define IDC_NEWGAMEROOM                10605
#define IDC_REFRESHGAMEROOMS           10606

#define IDD_GAMEHISTORY                10700
#define IDC_GAMEHISTORY                10701
#define IDC_GAMEHISTORYLIST            10702
#define IDC_DELETEGAME                 10703

#define IDD_PROMOTION                  10800
#define IDC_PROMOTION                  10801
#define IDC_ROOK                       10802
#define IDC_KNIGHT                     10803
#define IDC_BISHOP                     10804
#define IDC_QUEEN                      10805
