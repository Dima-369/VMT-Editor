# VMT Editor

Visual editor for the VMT filetype, used by Source Engine 1, written in C++ and Qt.

Includes texture preview, parameter validation and generally tries to help in editing VMTs.

![version](https://img.shields.io/badge/version-1.0.1-blue.svg)

## Downloads

Windows binary releases are provided on Github's release page.

We dropped the InnoSetup binary, as it only adds the Windows Explorer integration to open VMT files and Start menu shortcuts, and most seem to dislike the installation process.

## OS Support

Windows is primarily supported because we rely on `VTFCmd.exe` (see link below) to convert VTFs to images files, so they can be used in the preview.

But you can compile on macOS or Linux for yourself, and pretty much everything besides the texture preview will work.

## How to compile yourself

We are using Qt 5.6 and MinGW 5.3.0 for compiling (but newer versions should work as well).

- Download the community edition setup of [Qt](https://www.qt.io/)
- Install Qt Creator and MinGW (Qt setup handles this)
- Clone this project and open `VMT_Editor.pro` in Qt Creator
- (optional) Place DevIL.dll, VTFCmd.exe and VTFLib.dll in the executable folder (from [Nem's tools](http://nemesis.thewavelength.net/index.php?c=177)) for texture preview

Or skip the GUIs and compile through qmake directly.

You can open the example VMTs in the `misc/vmt` folder to check how everything looks for you.

## Screenshots

![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/1.png)
![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/2.png)
![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/3.png)
![](https://github.com/Gira-X/VMT-Editor/raw/master/screenshots/4.png)
