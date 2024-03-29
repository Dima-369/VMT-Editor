# VMT Editor ![version](https://img.shields.io/badge/version-1.3.11-blue.svg)

[Official website](https://dima-369.github.io/VMT-Editor/)

[User Guide](https://dima-369.github.io/VMT-Editor/documentation/#/)

Visual editor for .vmt files, written in C++ and Qt.

## Download

- [Latest release](https://github.com/Dima-369/VMT-Editor/releases/latest)
- [All releases](https://github.com/Dima-369/VMT-Editor/releases)

The Windows releases include all dependencies, just extract the zip and launch the editor!

## Features

- Simple and fast VMT creation with parameters separated in groups
- Parameter and texture validation for existing VMT files
- Support for VMT templates
- Automatic conversion of bitmaps to VTF, including smart alpha channel removal and renaming
- Automatic sorting of dropped bitmaps
- One click reconversion of all bitmaps used in the VMT
- Support for combining diffuse or normal map with a custom alpha channel - used for specular or reflection masks
- Batch VTF and VMT creation
- Blend tool texture generation, including support for 4 way blend and blend modulate
- Support for latest CS:GO parameters such as layer blend

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

You can open the example VMTs in the `misc/vmt` folder to check if everything works correctly.

## Screenshots

![](https://github.com/Dima-369/VMT-Editor/raw/master/screenshots/1.png)
![](https://github.com/Dima-369/VMT-Editor/raw/master/screenshots/2.png)
![](https://github.com/Dima-369/VMT-Editor/raw/master/screenshots/3.png)
![](https://github.com/Dima-369/VMT-Editor/raw/master/screenshots/4.png)
