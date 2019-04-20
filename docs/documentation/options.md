# Options

Open options to change how VMT Editor behaves. You can specify your own texture suffixes and enable auto renaming. You can select your preferred mipmap filter. You can also add VMT Editor to .vmt files context menu and change .vmt and .vtf file icons.

## Texture Conversion

### Remove alpha channel when not needed in vmt
VMT Editor can automatically save textures as DXT1 when there's no parameters that would use the texture's alpha channel detected (such as `$transparency 1` or `$normalalphaenvmapmask 1`)

### Replace Substance suffixes when converting textures
When converting textures, VMT Editor can automatically replace the suffixes such as *_diffuse*, *_normal* and *_specular* to those specified under _Custom suffixes_.

### Name converted VTFs after VMT filename
Automatically renames the converted textures with the name of the current open material + specified suffix. 

?> For example if the current material is called `concrete01.vmt` the with suffixes for Diffuse being ` ` and Normal `_normal`, the converted textures would be `concrete01.vtf` and `concrete01_normal.vtf` regardless of the source files' names.

## Mipmap filters

Here you can select your prefered mipmap filter and mipmap sharpen filter. 

_No mipmap sharpen filter on normal maps_ disables the sharpen filter when converting a normal map, which helps eliminate the "normal map getting stronger with distance" effect.

By default textures are saved with _DXT1_ or _DXT5_ compression, which sometimes results in noticeable artifacts in normal maps. You can disable that with _Uncompressed normal maps_ to force uncompressed _BGR888_ / _BGRA8888_ format. Keep in mind this makes the files four times larger.

## Shader groups

By clicking on _Edit shaders..._ you can enable or disable shaders from appearing in the _Shader_ dropdown list. You can also select which groups are active when selecting a shader and add custom shaders not included in VMT Editor. Keep in mind not all the parameter groups are avaiable for custom shaders.