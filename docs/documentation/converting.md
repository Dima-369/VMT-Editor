# Converting textures

One of the main features of VMT Editor is converting source texture files into .vtf files used by Source engine materials.

There are two main ways of doing this - automatically when importing textures or by opening the _Convert to VTF_ dialog.

## Automatic conversion

To use automatic conversion you can either:

- Drag and drop the texture directly into the text field
- Browse to the texture file

?> You can also drag and drop multiple textures onto the VMT Editor window (not in any text field) and they will be automatically sorted into their appropriate slots depending on their suffix. Works for diffuse, normal, specular, glossiness and tint mask.

Textures will be converted by the settings specified in the [Options](options.md#texture-conversion). It uses DXT1 for non alpha textures and DXT5 for textures with an alpha channel. 

After conversion textures will be copied into the directory of the currently open material. If material is not yet saved, they will be copied after the first save.

If you've made changes to the source texture you can reconvert by clicking on the two arrows at the end of the text field, or reconvert all the textures by pressing F9.

You can downscale the texture by half by clicking on the scale button at the end of the text field.

If you want to access more options for converting the texture you can click on the three lines button and it will be opened in the _Convert to VTF_ dialog.

!> Keep in mind the settings are not saved, so you'll have to open the dialog again if you want to reconvert the texture.

You can also add alpha channels to diffuse and bump textures. Just drag or browse to your mask in the text field that appears once you've added a texture to the diffuse or bump slot. 

?> For example, this is useful if you have separate diffuse, normal and specular textures. Since you're not able to use a separate spacular map if you're already using a normal, you can add the specular map to the alpha channel of the normal and enable _Bump alpha mask_.

!> Combining only works with PNG and non RLE compressed TGA files.

## Convert to VTF dialog

Convert to VTF dialog behaves similarly to VTFEdit import dialog with a few key differences.

?> Renaming settings from options are preserved.

You can convert multiple textures with the same settings at once by clicking on the _Add..._ button and selecting your textures. 

You can specify the output directory in the _Output_ slot. Empty means it will use the source texture location.

Click on the _Convert_ button to convert all the textures in the list. A checkmark appears next to converted textures.

You can select a template to quickly access common settings. 

?> If you open an image with VMT Editor, it will open in the _Convert to VTF_ dialog. 