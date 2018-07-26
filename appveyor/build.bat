git clone https://github.com/vicr123/the-libs.git
cd the-libs
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
