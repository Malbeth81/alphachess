/*
* PromotionDialog.h - A dialog that allows to choose between 4 piece type.
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
#ifndef PROMOTIONDIALOG_H_
#define PROMOTIONDIALOG_H_

#include "../chess/chessboard.h"
#include "../resource.h"
#include "../system.h"
#include "../theme.h"
#include <custombutton.h>
#include <winutils.h>


ChessPieceType ShowPromotionDialog(HINSTANCE Instance, HWND hParent, ChessSet* Set);

#endif
