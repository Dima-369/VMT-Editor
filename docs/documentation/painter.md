# Substance Painter shader

Substance Painter shader is designed to emulate the look of `VertexLitGeneric` shader.

Select the `source` shader in the _Shader Settings_ rollout. 

?> Add a Fill layer on top of your stack with _Diffuse_ and _Specular_ channels with your AO texture. Set them to multiply to get nicer looking reflections and more defined shapes.

## Channels

Shader uses the following Painter channels:

- Diffuse    (sRGB8)
- Normal     (RGB16F)
- Height     (L16F)
- Specular   (L16/L8)
- Glossiness (L16/L8)
- Metallic   (L16/L8)
- User0      (L16/L8) (Envmap mask)
- User1      (L16/L8) (Color mask)

Not all materials need all channels so use only the ones you need. Keep in mind each additional channel means more texture space.

You can add or remove the channels in the _Texture Set Settings_ rollout. 

## Templates

There's two templates included: _Source base_ and _Source extended_.

_Source base_ only includes the most used channels - Diffuse, Normal, Height and Specular.

_Source extended_ includes all the avaiable channels.

!> If the model appears black after opening a template, reselect the shader in the _Shader Settings_ rollout.

## Export presets

There are several export presets that depend on the use case:

- _Source All_ exports all the channels, with `Envmap mask` in `Diffuse` alpha and `Specular` in `Normal` alpha.
- _Source Base_ exports `Diffuse` with `Specular` alpha
- _Source Base+Spec_ exports `Diffuse` with separate `Specular`
- _Source Normal_ exports `Diffuse`, and `Normal` with `Specular` Alpha
- _Source Normal+Tint_ exports `Diffuse` with `Color Mask` alpha and `Normal` with `Specular` Alpha

You can edit or make your own eport presets by clicking on the _Configuration_ tab in the _Export textures_ window.

## Shader Settings

_Common parameters_ is a default Painter group and has no effects on the shader.

_Diffuse Color_ is the same as [Color](groups.md#color) or `$color2`.

_Envmap Tint_ behaves the same as [Reflection](groups.md#reflection) group _Tint_ or `$envmaptint`.

_Envmap Fresnel_ behaves the same as [Reflection](groups.md#reflection) group _Fresnel_ or `$envmapfresnel`.

_Use Specular Channel_ makes the envmap reflection use the _Specular_ Painter channel. If left unchecked it uses the _User0_ channel instead.

_Enable Envmap_ enables and disables envmap reflections.

_Fresnel Ranges_ behaves the same as [Specular](groups.md#vertexlitgeneric-specular) group _Fresnel_ or `$phongfresnelranges`.

_Specular Boost_ behaves the same as the [Specular](groups.md#vertexlitgeneric-specular) group _Boost_ or `$phongboost`. Boost values may vary from Source shader to a real material.

_Specular Glossiness_ behaves the same as [Specular](groups.md#vertexlitgeneric-specular) group _Glossiness_ or `$phongexponent`.

_Use Glossiness Channel_ switches between using the _Glossiness_ channel input or the _Specular Glossiness_ slider.

_Use Metallic Channel_ enables the _Metallic_ channel input. To use it in the VMT it needs to be packed into the green channel of the `Glossiness` map.

_Metallic Boost_ boosts the specular in the parts masked with the `Metallic`. Same as [Specular](groups.md#vertexlitgeneric-specular) group _Diffuse tint boost_ or `$phongalbedoboost `.

_Light Color_ changes the color of the light shining on the model in the preview.

## Gloss wizard

Gloss wizard is a Painter filter included with the Source shader. It automatically creates the _Specular_, _Glossiness_ and _Envmap mask_ channels from either _Roughness_ or _Specular_ channel information.

Drag the wizard on the top of your layer stack, but below AO if you have any. Note that your project needs to have the _Specular_, _Glossiness_ and _User0_ (Envmap mask) channels for it to work.

Select _Use Roughness_ to use the _Rougness_ channel information. Otherwise it will use the _Specular_ channel.

_Specular Adjust_ adjusts the curve of the _Specular_ channel.

_Gloss Curve Adjust_ adjusts the curve of the _Glossiness_ channel.

_Envmap Mask Cutoff_ adjusts at which value the envmap reflections start to fade in.

