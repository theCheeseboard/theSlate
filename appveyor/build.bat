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
copy release\theslate.exe deploy
cd deploy
windeployqt theslate.exe
