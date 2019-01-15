if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)


set QTDIR=C:\Qt\5.11\msvc2017_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

git clone https://github.com/vicr123/the-libs.git
cd the-libs
git checkout blueprint
qmake the-libs.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/contemporary-theme.git
cd contemporary-theme
qmake Contemporary.pro "CONFIG+=release"
nmake release
cd ..

git clone git://anongit.kde.org/extra-cmake-modules.git
cd extra-cmake-modules
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. -G "NMake Makefiles"
nmake
nmake install
cd ..\..

git clone git://anongit.kde.org/syntax-highlighting.git
cd syntax-highlighting
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. -G "NMake Makefiles"
nmake
nmake install
cd ..\..

qmake theSlate.pro "CONFIG+=release"
nmake release
mkdir deploy
mkdir deploy\styles
mkdir deploy\filebackends
mkdir deploy\translations
mkdir deploy\ColorDefinitions
mkdir deploy\ColorDefinitions\themes
copy "contemporary-theme\release\Contemporary.dll" deploy\styles
copy slate\release\theslate.exe deploy
copy slate\translations\*.qm deploy\translations
copy slate\ColorDefinitions\themes\* deploy\ColorDefinitions\themes
copy FileBackends\LocalFileBackend\release\LocalFileBackend.dll deploy\filebackends
copy FileBackends\HttpBackend\release\HttpBackend.dll deploy\filebackends
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
copy "C:\OpenSSL-Win64\bin\openssl.exe" deploy
copy "C:\OpenSSL-Win64\bin\libeay32.dll" deploy
copy "C:\OpenSSL-Win64\bin\ssleay32.dll" deploy
copy "C:\OpenSSL-Win64\bin\openssl.cfg" deploy
copy "C:\Program Files (x86)\KSyntaxHighlighting\bin\KF5SyntaxHighlighting.dll" deploy
cd deploy
windeployqt theslate.exe -network
