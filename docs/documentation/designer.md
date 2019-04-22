# Substance Designer shader

Substance Designer shader is designed to emulate both `LightmappedGeneric` and `VertexLitGeneric` shaders.

## Previewing

To add a texture to the shader preview, right click on any node in the graph and select _View in 3D view_, then select the appropriate slot.

!> At least one light needs to be active for the specular highlight to work.

Shader behaves differently depending on what shader it's emulating.

_LightmappedGeneric_ mode can only use one map for envmap and specular mask, which is used by the slot `specularmask`. 
Specular highlight can additionally be masked with modulated diffuse, depending on the _Mask Contrast_.
Specular highlight has no fresnel.

_VertexLitGeneric_ mode can use different maps for `envmapmask` and `specularmask`, and can make use of `Glossiness` and `Metallic` maps. 

?> You can use the same node as both `envmapmask` and `specularmask`.

Metallic map only tints the highlight with the color of the diffuse. You need to pack the metallic map in the green channel of glossiness map for export and use in source.
Envmap fresnel is dependent on both _Fresnel ranges_ and `Metallic` map.

## Settings

First switch changes the shader between `LightmappedGeneric` and `VertexLitGeneric` mode.

_Enable Environment Map_ enables and disables cubemap reflections.

_Envmap Tint_ behaves the same as [Reflection](groups.md#reflection) group _Tint_ or `$envmaptint`.

_Envmap Blur_ is a setting that you can use to fake lower cubemap sizes. Only used in Designer preview.

_Envmap Fresnel_ is the same as [Reflection](groups.md#reflection) group _Fresnel_ or `$fresnelreflection` or `$envmapfresnel` depending on selected shader.

_Specular Diffuse Mask Contrast_ is the same as [Specular](groups.md#lightmappedgeneric-specular) group _Mask contrast_ or `$phongmaskcontrastbrightness`.

_Specular Tint_ is the same as [Specular](groups.md#lightmappedgeneric-specular) group _Amount_ and _Tint_ or `$phongamount` and `$phongtint`.

_Specular Fresnel Ranges_ is the same as [Specular](groups.md#vertexlitgeneric-specular) group _Fresnel_ or `$phongfresnelranges`.

_Specular Glossiness_ is the same as [Specular](groups.md#vertexlitgeneric-specular) group _Glossiness_ or `$phongexponent`.

_Specular Boost_ is the same as the [Specular](groups.md#vertexlitgeneric-specular) group _Amount_ and _Boost_ or `$phongamount` and `$phongboost`.

_Use Glossiness Map_ switches between the specified `Glossiness` input or the _Specular Glossiness_ slider.

_Use Metallic Map_ enables the `Metallic` input. To use it in the VMT it needs to be packed into the green channel of the `Glossiness` map.

_Metallic Boost_ boosts the specular in the parts masked with the `Metallic`. Same as [Specular](groups.md#vertexlitgeneric-specular) group _Diffuse tint boost_ or `$phongalbedoboost `.

