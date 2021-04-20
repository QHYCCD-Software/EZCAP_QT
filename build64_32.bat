cd C:\Jenkins\workspace\EZCAP_QT
cd ..\make-qt-64

set path=C:\Qt\Qt5.12.9\Tools\mingw730_64\bin;%path%
set path=C:\Qt\Qt5.12.9\5.12.9\mingw73_64\bin;%path%

"C:\Qt\Qt5.12.9\Tools\QtCreator\bin\jom.exe" -f Makefile.Release clean
"C:\Qt\Qt5.12.9\5.12.9\mingw73_64\bin\qmake.exe" C:\Jenkins\workspace\EZCAP_QT\EZCAP.pro -spec win32-g++
C:/Qt/Qt5.12.9/Tools/mingw730_64/bin/mingw32-make.exe  release -j8




cd C:\Jenkins\workspace\EZCAP_QT
cd ..\make-qt-32

set path=C:\Qt\Qt5.12.9\Tools\mingw730_32\bin;%path%
set path=C:\Qt\Qt5.12.9\5.12.9\mingw73_32\bin;%path%

"C:\Qt\Qt5.12.9\Tools\QtCreator\bin\jom.exe" -f Makefile.Release clean
"C:\Qt\Qt5.12.9\5.12.9\mingw73_32\bin\qmake.exe" C:\Jenkins\workspace\EZCAP_QT\EZCAP.pro -spec win32-g++
C:/Qt/Qt5.12.9/Tools/mingw730_32/bin/mingw32-make.exe  release -j8



pause
pause


echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\*"
echo y | del "C:\Jenkins\workspace\make-qt-32\release\log\*.log"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer\Output\*.exe"
copy "C:\Jenkins\workspace\make-qt-32\release\*.*" "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\"

echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\*.o"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\*.cpp"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\*.h"
xcopy "C:\Jenkins\workspace\make-qt-32\release\platforms\*" "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\platforms\" /E /Y

if not exist C:\SoftwareSVN\sdk_publish\QHYCCD_SDK_CrossPlatform\build32\src\Release\qhyccd.dll exit 3
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer\Output\*.exe"
copy C:\SoftwareSVN\sdk_publish\QHYCCD_SDK_CrossPlatform\build32\src\Release\qhyccd.dll "C:\Jenkins\workspace\EZCAP_QT\installer\{source}\EZCAP_Qt\"

set innoSetupCompile32="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
set srcName="Installer_mail_Qt.iss"


cd /d C:\Jenkins\workspace\EZCAP_QT\installer
%innoSetupCompile32% /cc %srcName%

copy "C:\Jenkins\workspace\EZCAP_QT\installer\Output\*.exe" "C:\SoftwareSVN\QHY_SVN_Root\ZBH_WorkSpace\QHYCCDAllInOne\{input}\ezcap_qt\"
copy "C:\Jenkins\workspace\EZCAP_QT\installer\Output\*.exe" "D:\ftpRoot\ezcap_qt\"


pause



echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*"
echo y | del "C:\Jenkins\workspace\make-qt-64\release\log\*.log"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\Output\*.exe"
copy "C:\Jenkins\workspace\make-qt-64\release\*.*" "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\"

echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*.o"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*.cpp"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*.h"
xcopy "C:\Jenkins\workspace\make-qt-64\release\platforms\*" "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\platforms\" /E /Y

if not exist C:\SoftwareSVN\sdk_publish\QHYCCD_SDK_CrossPlatform\build64\src\Release\qhyccd.dll exit 6
copy C:\SoftwareSVN\sdk_publish\QHYCCD_SDK_CrossPlatform\build64\src\Release\qhyccd.dll "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\"


set innoSetupCompile32="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
set srcName="Installer_mail_Qt.iss"


cd /d C:\Jenkins\workspace\EZCAP_QT\installer_x64
%innoSetupCompile32% /cc %srcName%

copy "C:\Jenkins\workspace\EZCAP_QT\installer_x64\Output\*.exe" "C:\SoftwareSVN\QHY_SVN_Root\ZBH_WorkSpace\QHYCCDAllInOne\{input}\ezcap_qt\"
copy "C:\Jenkins\workspace\EZCAP_QT\installer_x64\Output\*.exe" "D:\ftpRoot\ezcap_qt\"
pause