if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)


set QTDIR=C:\Qt\5.11\msvc2017_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
git clone https://github.com/vicr123/the-libs.git
cd the-libs
git checkout blueprint
qmake the-libs.pro
nmake release
nmake install
cd ..
qmake theSlate.pro
nmake release
mkdir deploy
mkdir deploy\syntaxhighlighting
mkdir deploy\filebackends
mkdir deploy\translations
copy slate\release\theslate.exe deploy
copy slate\translations\*.qm deploy\translations
copy SyntaxHighlightingPlugins\release\SyntaxHighlightingPlugins.dll deploy\syntaxhighlighting
copy FileBackends\LocalFileBackend\release\LocalFileBackend.dll deploy\filebackends
copy FileBackends\HttpBackend\release\HttpBackend.dll deploy\filebackends
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
cd deploy
windeployqt theslate.exe
