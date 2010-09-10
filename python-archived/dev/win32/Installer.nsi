# NSIS Installer Tempalte
# by AZ Huang

# ${PROG_NAME} NSIS Script
!define PROG_NAME "Gummi"
!define PROG_NAME_LOW "gummi"
!define PUBLISHER "Wei-Ning Huang"
!define VERSION "svn"

# include modern UI2
!include MUI2.nsh

Outfile "${PROG_NAME}-${VERSION}-setup.exe"
Name "${PROG_NAME}"
InstallDir "$PROGRAMFILES\${PROG_NAME}"
SetCompressor lzma
RequestExecutionLevel admin

# MUI2 settings
!define MUI_ICON "${PROG_NAME_LOW}_64x64.ico"
!define MUI_ABORTWARNING

# Installer pages
Var SMFolder

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PROG_NAME_LOW}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $SMFolder
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PROG_NAME}.exe"
!insertmacro MUI_PAGE_FINISH

# Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

# set Languages
!insertmacro MUI_LANGUAGE "English"
 
# install section
Section
# Workaround for shortcut deletion bug on Vista/7
# See http://nsis.sourceforge.net/Shortcuts_removal_fails_on_Windows_Vista
# for more information
SetShellVarContext all
setOutPath $INSTDIR
File /r 'dist\*'

# Create uninstaller
WriteUninstaller $INSTDIR\uninstall.exe

# Create shortcuts
CreateSHortCut "$DESKTOP\${PROG_NAME}.lnk" \
	"$INSTDIR\${PROG_NAME}.exe"
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	CreateDirectory "$SMPROGRAMS\$SMFolder"
	CreateSHortCut "$SMPROGRAMS\$SMFolder\${PROG_NAME}.lnk" \
		"$INSTDIR\${PROG_NAME}.exe"
	CreateShortCut "$SMPROGRAMS\$SMFolder\Uninstall.lnk" \
		"$INSTDIR\Uninstall.exe"
!insertmacro MUI_STARTMENU_WRITE_END

# Write Registry
WriteRegStr HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}" \
	"DisplayIcon" "$INSTDIR\${PROG_NAME}.exe"
WriteRegStr HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}" \
	"DisplayName" "${PROG_NAME}"
WriteRegStr HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}" \
	"DisplayVersion" "${VERSION}"
WriteRegStr HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}" \
	"Publisher" "${PUBLISHER}"
WriteRegStr HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}" \
	"UninstallString" "$\"$INSTDIR\uninstall.exe$\""
WriteRegStr HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}" \
	"URLInfoAbout" "http://github.com/Aitjcize/${PROG_NAME}"
SectionEnd

# uninstall section
Section "uninstall"
# Workaround for shortcut deletion bug on Vista/7
# See http://nsis.sourceforge.net/Shortcuts_removal_fails_on_Windows_Vista
# for more information
SetShellVarContext all

rmdir /r $INSTDIR
Delete "$DESKTOP\${PROG_NAME}.lnk"
!insertmacro MUI_STARTMENU_GETFOLDER Application $SMFolder
rmdir /r "$SMPROGRAMS\$SMFolder"
DeleteRegKey HKLM \
"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}"

DeleteRegKey /ifempty HKCU "${PROG_NAME}"
sectionEnd
