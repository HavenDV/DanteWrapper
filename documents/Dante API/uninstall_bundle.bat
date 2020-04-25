@ECHO off

SET BUNDLE_UPGRADE_CODE=%1
SET GOT_UNINSTALL_STRING=0

ECHO "Deleting Wix Bundle with UpgradeCode %BUNDLE_UPGRADE_CODE%"


FOR /F "delims=" %%A in ('uninstall_bundle.exe -p %%BUNDLE_UPGRADE_CODE%%') DO (
	SET GOT_UNINSTALL_STRING=1
	SET UNINSTALL_STRING=%%A
)

:Process
IF NOT %GOT_UNINSTALL_STRING% EQU 1 (
	 ECHO "Bundle not found"
	 EXIT
) ELSE (
	 ECHO "Deleting Bundle, command is '%UNINSTALL_STRING%'"
	 %UNINSTALL_STRING%
)
