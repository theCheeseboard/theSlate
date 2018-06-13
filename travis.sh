
    git clone https://github.com/vicr123/the-libs.git
    cd the-libs
    qmake
    make
    sudo make install INSTALL_ROOT=/
    cd ..
    qmake
    make
    make install INSTALL_ROOT=appdir
    wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod a+x linuxdeployqt-continuous-x86_64.AppImage
    "./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs"
    "./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage"