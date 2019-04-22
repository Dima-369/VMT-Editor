# Parameter groups

Parameter groups consists of all the related parameters for the specified material property, however only the parameters you modify will get added to the .vmt file. 

## Texture

Texture groups is the core group of most materials. It contains the material _Diffuse_ (or albedo or base color) and _Bump_ which is either a normal map or a ssbump map.

!> You cannot use ssbump on models.

_Surface_ dropdown determines the physical properties of the materials, like what sound it makes or how bullets penetrate it.

If you're using a blend material you have an option to specify a second set of _Diffuse_ and _Bump_ textures, however only second _Diffuse_ is required for a blend material. You also have an option to specify a _Blend Modulate_ texture, which controls the border between the two blended textures.

?> CS:GO can use grayscale blend textures, provided you use the new _Layer Blend_ functions.

## Detail

 Adds a detail texture over the whole material, scales higher than 1 mean smaller than diffuse and lower than 1 mean larger than diffuse.
 
_Amount_ controls the visibility of the detail texture.

You can select the detail blend mode with the dropdown next to the _Amount_ slider. Normal behaves like overlay in Photoshop.

!> Not all blend modes works in all games. CS:GO only supports blend modes 0, 7, 9, 10, 11, 12.

## Transparency

_Opacity_ controls the overall opacity of material, _Translucent_ and _Alpha test_ use diffuse alpha channel.

?> Translucent is smooth but has sorting issues. Alpha test solves those issues, but has hard edges - useful for materials like foliage or chain link fences. _Alpha to coverage_ allows MSAA to work with alpha test.

You can control alpha test the cutoff point with threshold. 

_Additive_ doesn't need an alpha channel, black is see through and white is solid (like Photoshop). 

Check _Decal_ if you plan to use the material as decal or overlay.

## Color

Tints the material. _Color_ is used for brush materials and _Color 2_ is used for model materials.

_Mask_ is a greyscale mask that controls which areas of the material get tinted (white) and which ones don't (black). Usually used with model tinting in Hammer.

!> You can't use a mask texture if you use a bump map. It's recommended to place the mask into the alpha channel of the diffuse texture and use the _Diffuse alpha mask_ checkbox.

## Transform

Texture and Bump Transform allow you to independently change the scale and position of diffuse and bump textures.

_Centes_ specifies the center coordinate of the scaling and rotation. _Scale_ controls the number of tiles, so a larger value means a smaler scale. 

## Lighting

_Light warp_ is a one dimensional texture that allows you to "gradient map" the lighting.

!> Looks like _Light warp_ does not work in CS:GO.

_Reflectivity_ allows you to override the value that VRAD uses when calculating the light bounces. Useful for making materials bounce more light.

_Seamless scale_ makes material use triplanar mapping, useful for very distorted displacements. 

!> _Seamless scale_ does not support normal maps, only ssbump.

## Flags

_Tool texture_ is a texture that displays in Hammer texture browser.

_Keywords_ help you find the texture in the Hammer texture browser.

_Ignore Z_ makes the material render on top of everything.

_No fog_ stops the material from reacting to environment fog.

_Disable CSM lookup_ stops the material from receiving dynamic shadows in CS:GO. Can improve performance with very dense foliage.

## Specular

Enables phong specular reflections on the material. Group looks different depending on selected shader.

### LightmappedGeneric specular

!> Only works in CS:GO

_Amount_ controls the color as well as the brightness of the specular highlight. Last spinbox brightens and darkens the whole material.

?> Specular highlight does not change depending on the light color so it's a good idea to tint the specular with your sun color.

_Glosiness_ controls the size or exponent of the specular highlight. Higher means smaller highlight.

With _Mask contrast_ and _Mask brightness_ you can adjust the contrast of the specular mask. 

?> Behaves like a ratio, so a value of `[4 2]` is the same as `[2 1]`

Diffuse luminance is used as default for the specular mask, but you can use alpha channel of either diffuse or bump with _Diffuse alpha mask_ or _Bump alpha mask_. This setting is shared with the _Reflection_ group since you can only have one mask on brushes.

!> Specular highlight ignores normal map if the material doesn't have a cubemap specified. If you only want specular reflections you can enable cubemap and set the tint to 0.

### VertexLitGeneric specular

_Fresnel_ controls the specular highlight brightness when looking at the material from different angles of incidence. F0 means looking directly at the material, while 90 is looking completely parallel to the material. 

?> Surfaces in real life are shinier when looking at them from the side and less shiny when looking diretly perpendicular at the surface. A good starting value for non-metallic materials is `[0.1 0.5 1]`.

_Boost_ controls the brightness of the specular highlight.

_Glossiness_ controls the size or exponent of the specular highlight. Higher means smaller highlight. You can also specify a glossiness map which overrides the _Glossiness_ slider.

?> Glossiness map uses only red channel. You can use green channel of this map as a metallic map, together with diffuse tint.

_Diffuse tint boost_ only works when you have _Glossiness green channel diffuse tint_ enabled. Controls the amount of tint the specular highlight gets from the diffuse color. Useful for colored metallic looking highlights. 

_Tint_ controls the amount of color tint for the specular highlight. Useful for single color metallic materials without a glossiness map.

_Warp_ allows you to remap the specular values with a gradient texture. Useful for iridescent effects.

By default, specular uses the alpha channel of bump for the mask. You can instead use the alpha of the diffuse with _Diffuse alpha mask_.

?> Unline brush specular, you can use a separate map for both specular and cubemap reflections.

_Half lambert_ only works on dynamic models and "stretches" the lighting around the whole model.

## Reflection

Reflection groups controls the static cubemap reflections.

You can either use the level cubemaps with _Use cubemap_ or specify your own cubemap in the _Reflection_ text field. 

?> You can use the `environment maps/metal_generic_001 - 006` to fake blurred reflections

For the mask you can either specify a texture or use the alpha channel of either diffuse or bump with _Diffuse alpha mask_ or _Bump alpha mask_.

!> You cannot use a separate texture if you're using a bump map.

?> In CS:GO you can also use the red channel of the color mask texture. Only avaiable on models.

_Tint_ controls the overall brightness and color of the reflection.

_Fresnel_ on brushes makes the reflection darker when looking straight at the surface. Useful for realistic looking reflections. 

_Fresnel_ on models controls the amount of specular _Fresnel ranges_ and _Diffuse tint_ that get applied to the reflection.

_Light influence_ makes the reflection get brighter or darker, depending on the lighting. Useful when using a custom cubemap.

_Anisotropy_ stretches the reflecion downwards.

!> _Saturation_, _Contrast_, _Light influence_ and _Anisotropy_ don't work on models with a normal map.

## Self illumination

Adding self illumination makes the material emissive.

It uses the alpha channel of diffuse for the mask. You can also specify your own texture or use specular map alpha channel.

_Tint_ controls the brightness and color of the light emission.

## Normal blend

!> Only works in CS:GO. Brush only.

Allows you to blend a detail normal map on top of existing normal map. 

## Layer blend

!> Only works in CS:GO. Blend only.

Enable the effects with _Enable new blend_. Also requires a blend modulate texture.

_Layer 1 tint_ and _Layer 2 tint_ allow you to tint each blend layer separately.

?> You can use this to tint the same diffuse texture different colors and blend between them.

_Blend softness_ controls the softness of the border between the layers. Smaller means harder border.

_Border strength_, _Border softness_, _Border offset_ and _Border tint_ allow you to create a colored border around the blend point of the two layers.

## Decal

!> Only works in CS:GO. Model only.

Allows you to specify a texture that uses the model's second UV set to apply decals over the diffuse.

## Tree sway

!> Does not work with a normal map.

Simulates foliage swaying in the wind. Effect is based around model origin.

_Height_ and _Radius_ control the distance in world units at which the effect is fully on.

_Start height_ and _Start radius_ control the portion of the height or radius at which the effects starts to fade in (0-1).

_Strength_ controls how much the tree sways.

_Speed_ controls how quickly the tree sways.

_Falloff_ controls the falloff of the sway, higher means the core of the tree is more stable.

_Scrumble strength_ controls how much the leaves move.

_Scrumble speed_ controls how quickly the leaves move.

_Scrumble frequency_ controls the size of the sine wave applied to scrumble motion. Usually 10-25.

_Scrumble falloff_ controls the falloff of the scrumble motion, higher means the core of the tree is more stable.

_High wind multiplier_ controls the multiplier of the motion at wind speeds. Starts _Lerp start_ and is fully on at _Lerp end_.

## Rim light

Adds a rim light effect around the models that is always visible.

_Exponent_ controls the size and _Boost_ controls the brightness.

## 4 Way Blend

!> Only works in CS:GO

Allows you to blend 4 materials instead of just regular two. Each texture has its own independent scale and blend controls.

_Blend value_ of each texture controls the sharpness of the blend transition.

_Blend range_ controls the texture luminance values that are used for the blend transition. _Blend factor_ controls the amount of this blend.

!> 4 Way Blends can only use two bump maps and two surface materials. Which surface material is used is still controlled by the alpha channel of the displacement.

## Water

?> Only avaiable when Water shader is selected

Base group of the Water shader, here you can specify basic properties of the water material.

_Normal_ is the normal map that's used for the water surface, can be static or animated.

?> Most water materials use a static normal map and add animation in shader with a flowmap.

_Bottom material_ is used to specify the material that appears when you go under water. Usually has the same parameters as the top material, except with no reflection, bottom material and _Bottom_ checked.

_Force expensive_ and _Force cheap_ are deprecated in newer games, though most water materials still use _Force expensive_.

_Bumpframe_ allows you to specify the start frame of the animated normal map. Usually animated with proxies.

?> Use the _Animated texture_ proxy
```Proxy
AnimatedTexture
{
animatedtexturevar $normalmap
animatedtextureframenumvar $bumpframe
animatedtextureframerate 30
}
```

## Water - Flow

Allows you to add an animated flow effect to the water by using a single static flowmap texture.

!> When using _Flow_, the normal map scale is controlled by _Normal map size_ in world units. The texture scale in Hammer instead controls the size of the flowmap. You can use _Flow debug_ to see the flowmap in Hammer and align it to your water surface. This means that the material scale in Hammer is usually very big, around `10 - 20`

_Flowmap_ is a texture that specifies the direction of the water flow. Red channel is X direction with 127 being still and green channel is Y direction with 127 being still. 

_World UV scale_ is the scale multiplier of the flowmap. Usually 1.

_UV scroll distance_ is the relative amount the water moves in a single cycle.

_Time scale_ is the multiplier of flow cycle speed.

_Time interval_ is the duration of a single cycle in seconds.

_Bump strength_ controls the amount of normal map.

_Noise scale_ controls the scale of the _Noise texture_ that is used to break up the water repetition. Usually very small.

## Water - Fog

Controls the underlying color of the water.

?> This means the darkest color the water will be. To achieve realistic results use dark (blue) fog color, usually around 20 brightness. Brighter colors result in murkier looking water.  

Water will fade into the specified _Fog color_ after number of units specifies in _Fog start_ and _Fog end_

Check _Volumetric fog_ to enable this effect. 

Use _Lightmap fog_ for the water to be able to have shadows cast on it.

_Flashlight tint_ controls how much the flashlight affects the water color.

## Water - Reflection

Controls the water reflections.

?> Check _Real time reflection_ to enable the effect.

_Tint_ controls the color of the reflection. 

?> To achieve good looking results the reflection tint should not be very saturated and depending on the fog color should be darker if the fog is brighter. 

_Amount_ contrils the distortion amount applied to the reflection, with `0` looking like completely still water.

!> _Reflect 2D skybox_ is needed for water to reflect the skybox in CS:GO

_Only skybox_ only reflects the 3D skybox. Works only in L4D2.

!> Use `$reflect3dskybox 1` (additional parameter) for the water to reflect 3D sky in CS:GO.

_Reflect entities_ makes the water reflect all point entities, including prop_statics. Wihtout it water only reflects brushes and displacements.

_Only marked entities_ makes the water reflect only point entities that have the _Render in cheap reflections_ set to `1` in Hammer.

_Texture_ allows you to specify a static texture that will be visible as a reflection. Rarely useful.

?> If you don't want to have a real time reflection, add a normal _Reflection_ group and specify a cubemap. Water _Reflection_ group still controls the tint though.

## Water - Refraction

Controls the distortion when looking through the water surface. 

_Tint_ controls the tint of the refraction. Only visible before water fog is fully opaque.

?> Use a different refraction tint than fog color to have the effect of water being a different color in the shallow areas. 

_Amount_ controls the distortion amount.

## Water - Scroll

An older effect for animating the water surface. Overlays the normal map on itself and scrolls it into two directions specified. 

!> _Scroll_ cannot be used together with _Flow_.

## Refract

Base group of refract shader. 

Allows you to specify two normal maps that get overlayed together. 

?> Use the `Scroll` proxy to scroll the normal maps in different directions. 

_Tint_ controls the color of the surface. Can also specify a texture.

_Amount_ controls the distortion amount.

_Blur_ controls the amount of blur behind the surface. 