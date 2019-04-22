# Material creation

Creating materials consists of adding parameter groups and modifying elements within those groups. Most shaders already have the default _Texture_ parameter group visible.

You can add additional parameter groups with the _Add..._ menu or by using the toolbar on the left side of the window.

Parameter groups consists of all the related parameters for the specified material property, however only the parameters you modify will get added to the .vmt file. 

?> What each group does is described in the [Parameter Groups](groups.md) section.

## Text fields

Text fields are used to specify different texture files. If you already have .vtf files you can either browse directly to them or type the path into the text field. 

?> If the file is located outside of the currently selected game directory it will be copied to the location of current saved material.

!> Textures located in the .vpk or .gcf files will show up as missing, but they will work in game.

Text fields can also convert your source texture files into .vtf files, different methods are described in the [Converting Textures](converting.md) section.

## Sliders

Sliders are used to control most of the scalar parameters. Though it may seem like they only go up to a certain value, you can manually type the value into the spinbox and the slider will adjust its range. 

## Colors

Most color selectors are paired with a slider that you can use to control the color's brightness. This also allows the color values to go above `[1 1 1]`. 

?> Colors may look different after saving and opening a material, especially with darker colors. VMT Editor splits the brightness and color part when loading a material, but the end result should be the same.

?> Color is automatically converted from sRGB to linear for certaion parameters like `$color`.