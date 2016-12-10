# VMT Editor

Visual editor for the VMT filetype, used by Source Engine 1, written in C++ and Qt.

Includes texture preview, parameter validation and generally tries to help in editing VMTs.

![version](https://img.shields.io/badge/version-1.0.1-blue.svg)

## Downloads

You can download a very recent version from here: [VMT Editor 1.01](http://gortnar.com/vmt/VMT_Editor_1_01.7z)

We dropped the InnoSetup binary, as it only adds the Windows Explorer integration to open VMT files and Start menu shortcuts and most seem to dislike the installation process.

## OS support

The texture preview tool we are using to convert VTFs to PNGs is apparently Windows only, so this is the only platform which we release binaries for.

If this does not bother you, you can compile on macOS or Linux and use the editor on other platforms as well.

## How to compile yourself

We are using Qt 5.6.0 for compiling.

- Download the community edition setup of [Qt](https://www.qt.io/)
- Install Qt Creator (Qt setup can handle this)
- Install MinGW (Qt setup can handle this as well)
- Clone this project and open `VMT_Editor.pro` in Qt Creator
- (optional) Place DevIL.dll, VTFCmd.exe and VTFLib.dll in the executable folder (from [Nem's tools for texture preview](http://nemesis.thewavelength.net/index.php?c=177))

Or skip the GUIs and compile through qmake directly.

You can open the example VMTs in the `Misc/vmt` folder to check how everything looks for you.

## Screenshots

![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/1.png)
![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/2.png)
![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/3.png)
![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/4.png)
