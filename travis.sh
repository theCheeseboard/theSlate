if [ $STAGE = "script" ]; then
  if [ $TRAVIS_OS_NAME = "linux" ]; then
    echo "[TRAVIS] Preparing build environment"
    source /opt/qt510/bin/qt510-env.sh
    echo "[TRAVIS] Building and installing the-libs"
    git clone https://github.com/vicr123/the-libs.git
    cd the-libs
    qmake
    make
    sudo make install INSTALL_ROOT=/
    cd ..
    echo "[TRAVIS] Running qmake"
    qmake
    echo "[TRAVIS] Building project"
    make
    echo "[TRAVIS] Installing into appdir"
    make install INSTALL_ROOT=appdir
    echo "[TRAVIS] Getting linuxdeployqt"
    wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod a+x linuxdeployqt-continuous-x86_64.AppImage
    echo "[TRAVIS] Building AppImage"
    ./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs
    ./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage
  else
    echo "[TRAVIS] Building for macOS"
  fi
elif [ $STAGE = "before_install" ]; then
  if [ $TRAVIS_OS_NAME = "linux" ]; then
    wget -O ~/vicr12345.gpg.key https://vicr123.com/repo/apt/vicr12345.gpg.key
    sudo apt-key add ~/vicr12345.gpg.key
    sudo add-apt-repository 'deb https://vicr123.com/repo/apt/ubuntu bionic main'
    sudo add-apt-repository -y ppa:beineri/opt-qt-5.10.0-xenial
    sudo apt-get update -qq
    sudo apt-get install qt510-meta-minimal qt510x11extras qt510tools qt510translations qt510svg qt510websockets xorg-dev libxcb-util0-dev libgl1-mesa-dev
  else
    echo "[TRAVIS] Preparing to build for macOS"
  fi
elif [ $STAGE = "after_success" ]; then
  if [ $TRAVIS_OS_NAME = "linux" ]; then
    echo "[TRAVIS] Publishing AppImage"
    wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
    bash upload.sh theSlate*.AppImage*
  fi
fi
