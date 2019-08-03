<img src="readme/splash.svg" width="100%" />

---
<p align="center">
<a href="https://travis-ci.org/vicr123/theSlate"><img src="https://img.shields.io/travis/vicr123/theslate/blueprint?label=Linux%2C%20macOS&style=for-the-badge" alt="Travis CI Build Status" /></a>
<a href="https://ci.appveyor.com/project/vicr123/theslate"><img src="https://img.shields.io/appveyor/ci/vicr123/theslate/blueprint?label=Windows&style=for-the-badge" alt="AppVeyor Build Status" /></a>
<img src="https://img.shields.io/github/license/vicr123/theslate?style=for-the-badge" />
</p>

theSlate is a cross platform text editor, designed to be fast and integrate well with your system of choice.

[Website](https://vicr123.com/theslate)

---

# Dependencies
- Qt 5
  - Qt Core
  - Qt GUI
  - Qt Widgets
  - Qt WebEngine
  - Qt SVG
- [the-libs](https://github.com/vicr123/the-libs)
- [KSyntaxHighlighting](https://api.kde.org/frameworks/syntax-highlighting/html/index.html)

# Get
If you're using a supported operating system, we may have binaries available:

| System | Package |
|-------------------|---------------------------------------------------------------------------------------------------------|
| Windows Installer | [theSlate Installer](https://github.com/vicr123/theInstaller/releases/download/continuous/theSlate.exe) |
| Windows Portable | [Zip Archive on GitHub Releases](https://github.com/vicr123/theSlate/releases) |
| macOS | [Application Bundle on GitHub Releases](https://github.com/vicr123/theSlate/releases) |
| Arch Linux | `theslate` on the AUR |
| Other Linux | [AppImage on GitHub Releases](https://github.com/vicr123/theSlate/releases) |

For more information, visit the [website](https://vicr123.com/theslate/download.html).

## Build
Run the following commands in your terminal. 
```
qmake
make
```

## Install
On Linux, run the following command in your terminal (with superuser permissions)
```
make install
```

On macOS, drag the resulting application bundle (`theSlate.app` or `theSlate Blueprint.app`, depending on which branch you're building) into your Applications folder

# Contributing
Thanks for your interest in theSlate! Check out the [CONTRIBUTING.md](CONTRIBUTING.md) file to get started and see how you can help!

---

> © Victor Tran, 2019. This project is licensed under the GNU General Public License, version 3, or at your option, any later version.
> 
> Check the [LICENSE](LICENSE) file for more information.