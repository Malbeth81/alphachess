#include "inputdialog.h"

#include "../resource.h"

static const char* DlgTitle;
static const char* DlgPrompt;
static char* DlgValue;

// WINAPI FUNCTIONS ------------------------------------------------------------

static BOOL __stdcall InputDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      /* Process control messages */
      switch (LOWORD(wParam))
      {
        case IDOK:
        {
          GetWindowText(GetDlgItem(hDlg, IDC_INPUT), DlgValue, MAX_PATH);
          SendMessage(hDlg, WM_CLOSE, 1, 0);
          break;
        }
        case IDCANCEL:
        {
          SendMessage(hDlg, WM_CLOSE, 0, 0);
          break;
        }
      }
      return TRUE;
    }
    case WM_CLOSE:
    {
      EndDialog(hDlg, wParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      SetWindowText(hDlg, DlgTitle);
      SetWindowText(GetDlgItem(hDlg, IDC_INPUTLBL), DlgPrompt);
      SetWindowText(GetDlgItem(hDlg, IDC_INPUT), DlgValue);
      return TRUE;
    }
  }
  return FALSE;
}

// Public functions ------------------------------------------------------------

char* InputDialog(HWND hParent, const char* Title, const char* Prompt, const char* DefaultValue)
{
  HINSTANCE Instance = (hParent != NULL ? (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE) : GetModuleHandle(NULL));
  DlgTitle = Title;
  DlgPrompt = Prompt;
  DlgValue = new char[MAX_PATH];
  strcpy(DlgValue, DefaultValue);
  if (DialogBox(Instance, MAKEINTRESOURCE(IDD_INPUT), hParent, (DLGPROC)InputDialogProc) > 0)
    return DlgValue;
  else
    return NULL;
}
