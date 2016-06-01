#ifndef INPUTDIALOG_H_
#define	INPUTDIALOG_H_

#include <windows.h>

char* InputDialog(HWND hWindow, const char* Title, const char* Prompt, const char* DefaultValue = "");

#endif
