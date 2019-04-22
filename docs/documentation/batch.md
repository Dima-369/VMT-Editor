# Batch material creation

You can use VMT Editor to quickly batch create multiple .vmt files.

!> In order to use this function you need to have the VTF files ready in the game materials directory. You can convert your textures with [Convert to VTF](converting.md#convert-to-vtf-dialog) dialog first.

1. Create your base material that has all the parameters you want to have in your batched files. This also includes adding textures.

2. Open the _Batch VMT_ dialog from the _Tools_ menu.

3. Click on add and select only diffuse textures. VMT Editor automatically adds the normal, specular, glossiness and other textures based on the texture suffixes in your base material.

?> This means that if your diffuse is named `floor01` and your bump `floor01_normal`, the suffix for bump will be set as `_normal`. Batched material for `metal01` will then have `metal01_normal` inserted into the bumpmap slot. 

?> If your diffuse is named `floor01` and your bump `ground01_normal`, all the batched VMT files will have `gorund01_normal` inserted into bumpmap slot.

!> Unfortunately, this doesn't work if your diffuse already has a suffix, as it compares all the texture names with diffuse. 

4. Press on _Batch_ to create materials for all the textures selected in the list.

?> You can use _Batch VMT_ to quickly create materials for all skybox sides. If you're using half height sides you have to make the top and bottom materials separately. 

