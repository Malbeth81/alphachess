  SetCompressor lzma
# Variables
  !define ProgramName "alphachess"
  !define ProgramTitle "AlphaChess 4"

# Configuration
  Name "${ProgramTitle}.0.1"
  OutFile "..\AlphaChessSetup.exe"
  InstallDir "$PROGRAMFILES\${ProgramTitle}"
  InstallDirRegKey HKLM "Software\${ProgramTitle}" "Install Dir"
  XPStyle On
  
# Modern UI Configuration
  !include "MUI.nsh"
#  !define MUI_ICON "images\install.ico"
#  !define MUI_UNICON "images\uninstall.ico"
#  !define MUI_HEADERIMAGE
#  !define MUI_HEADERIMAGE_BITMAP "images\install.bmp"
#  !define MUI_HEADERIMAGE_UNBITMAP "images\uninstall.bmp"
#  !define MUI_HEADERIMAGE_RIGHT
#  !define MUI_WELCOMEFINISHPAGE_BITMAP "images\welcome.bmp"
  !define MUI_ABORTWARNING
  !define MUI_WELCOMEPAGE_TITLE_3LINES
  !define MUI_FINISHPAGE_RUN "$INSTDIR\${ProgramName}.exe"
  
  # Installer steps
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "License.txt"
#  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  # Uninstaller steps
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
  
  # Installer options
  !insertmacro MUI_RESERVEFILE_LANGDLL
  !insertmacro MUI_LANGUAGE "English"
#  !insertmacro MUI_LANGUAGE "French"

# Begin Section
Section "AlphaChess" SecMain
SectionIn RO
  ;Add files
  SetOutPath "$INSTDIR"
  SetOverWrite On
  File "${ProgramName}.exe"
  File "${ProgramName}.ini"
  File "license.txt"
  
  SetOutPath "$INSTDIR\fonts"
  SetOverWrite On
  File "fonts\ALFOCHES.TTF"
  File "fonts\Alpha.ttf"
  File "fonts\AVENFONT.TTF"
  File "fonts\Berlin.ttf"
  File "fonts\CASEFONT.TTF"
  File "fonts\chess1.ttf"
  File "fonts\Chess-7.TTF"
  File "fonts\CONDFONT.TTF"
  File "fonts\HARLFONT.TTF"
  File "fonts\KINGFONT.TTF"
  File "fonts\LEIPFONT.TTF"
  File "fonts\LINEFONT.TTF"
  File "fonts\LUCEFONT.TTF"
  File "fonts\MAGNFONT.TTF"
  File "fonts\MARKFONT.TTF"
  File "fonts\MARRFONT.TTF"
  File "fonts\MAYAFONT.TTF"
  File "fonts\MERIFONT.TTF"
  File "fonts\MILLFONT.TTF"
  File "fonts\montreal.ttf"
  File "fonts\MOTIFONT.TTF"
  File "fonts\MVALFONT.TTF"
  File "fonts\stchess.ttf"
  File "fonts\TT_USUAL.TTF"

  SetOutPath "$INSTDIR\games"
  SetOverWrite Off
  
  SetOutPath "$INSTDIR\themes"
  SetOverWrite Off
  File "themes\Deep Blue Hollow.ini"
  File "themes\Deep Blue Solid.ini"
  File "themes\Deep Green Hollow.ini"
  File "themes\Deep Green Solid.ini"
  File "themes\Deep Grey Hollow.ini"
  File "themes\Deep Grey Solid.ini"
  File "themes\Deep Red Hollow.ini"
  File "themes\Deep Red Solid.ini"
  File "themes\Light Blue Hollow.ini"
  File "themes\Light Blue Solid.ini"
  File "themes\Light Green Hollow.ini"
  File "themes\Light Green Solid.ini"
  File "themes\Light Grey Hollow.ini"
  File "themes\Light Grey Solid.ini"
  File "themes\Light Red Hollow.ini"
  File "themes\Light Red Solid.ini"

  ; Create start-menu items
  CreateShortCut "$SMPROGRAMS\${ProgramTitle}.lnk" "$INSTDIR\${ProgramName}.exe" "" "$INSTDIR\${ProgramName}.exe" 0
  CreateShortCut "$DESKTOP\${ProgramTitle}.lnk"    "$INSTDIR\${ProgramName}.exe" "" "$INSTDIR\${ProgramName}.exe" 0

  ; Write installation information to the registry
  WriteRegStr   HKLM "Software\${ProgramTitle}" "Install Dir"      "$INSTDIR"
  WriteRegDWORD HKLM "Software\${ProgramTitle}" "Install Language" $LANGUAGE

  ;Write uninstall information to the registry
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramTitle}" "DisplayIcon"     "$INSTDIR\${ProgramName}.exe,0"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramTitle}" "DisplayName"     "${ProgramTitle}"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramTitle}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramTitle}" "ModifyPath"      "$INSTDIR"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramTitle}" "NoModify"        1

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

#SubSection "Languages" SecLang
   
#Section "English" SecLangEn
#  ;Add files
#  SetOutPath "$INSTDIR"
#  SetOverWrite On
#  File "AChessEN.lang"
#SectionEnd

#Section "Français" SecLangFr
#  ;Add files
#  SetOutPath "$INSTDIR"
#  SetOverWrite On
#  File "AChessFR.lang"
#SectionEnd

#SubSectionEnd

# Installer Functions
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
;  MessageBox MB_OK "Warning!! If you have NOT installed version 3.1.4 then BEFORE you continue with this installation you MUST save any game you want to preserve from your history otherwise they will be deleted!"
FunctionEnd

# Uninstaller Section
Section "Uninstall"
  ;Delete Files And Directory
  Delete "$INSTDIR\themes\*.*"
  RmDir "$INSTDIR"
  
  Delete "$INSTDIR\fonts\*.*"
  RmDir "$INSTDIR"
  
  Delete "$INSTDIR\*.*"
  #RmDir "$INSTDIR" ; Because of games

  ;Delete Shortcuts
  Delete "$SMPROGRAMS\${ProgramTitle}.lnk"
  Delete "$DESKTOP\${ProgramTitle}.lnk"

  ;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${ProgramTitle}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${ProgramTitle}"
SectionEnd

#Uninstaller Functions
Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd
