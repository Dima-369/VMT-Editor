# Workflow

Here are some workflow examples for different types of materials.

## Concrete 

`LightmappedGeneric` shader.

Textures required: diffuse, normal.

1. Drag both diffuse and normal into VMT Editor window to automatically convert them.

2. Set the _Surface_ to `Concrete`.

3. Add _Specular_ group.

4. Set the _Amount_ color slightly orange to mimic the sun color.

5. Set the _Glossiness_ to around `10-25`.

6. Load up the material in game, then adjust the _Mask contrast_ amount until you get a more pronounced specular effect. You can also adjust the _Amount_ to compensate for darkening.

7. If the concrete normal map is very rough, add the _Reflection_ group, check _Use cubemap_ and set the _Tint_ to `0`. This makes the specular affected by the normal map.

## Shiny wood floor

`LightmappedGeneric` shader.

Textures required: diffuse, normal, specular.

1. Drag your textures into the VMT Editor window to convert them. They should get automatically sorted into _Diffuse_, _Bump_, and _Bump alpha_ slots.

2. Set the _Surface_ to `Wood`.

3. Add _Reflection_ group.

4. Check _Use cubemap_ and _Bump alpha mask_.

5. Set _Fresnel_ to around `0.95`. 

6. Load up the material in game, build cubemaps, then adjust the _Tint_ brightness to get the material to fit in the environment.

## Car paint

`VertexLitGeneric` shader.

Textures required: diffuse, normal, specular (optional), tint mask (optional).

For best results make the car paint part of the model its own material. 

?> Make the diffuse based on a white car so you can easily make more variants with _Color_. 

1. Drag your textures into the VMT Editor window to convert them. They should get automatically sorted into _Diffuse_, _Bump_, and _Bump alpha_ slots. Drag the tint mask into _Diffuse alpha_ (optional).

2. Add _Color_ group. Check _Diffuse alpha mask_ (optional).

3. Add _Reflection_ group.

4. Check _Use cubemap_ and _Bump alpha mask_.

5. Set _Tint_ to around `0.5` and _Fresnel_ to `1`.

6. Add _Specular_ group.

7. Set _Fresnel_ to `0.2 0.6 1.0`, _Boost_ to `4` and _Glossiness_ to `50`.

8. Decide on the color of your car. This method requires you to make separate materials for each car color instead of tinting the model in Hammer. 

?> You can use online car configurators to find realistic colors. 

9. Set both the _Color 2_ and specular _Tint_ to that color. This creates a metallic looking highlight. 

10. Load up the material in game, build cubemaps, then adjust specular _Boost_ and reflection _Tint_ to better match the environment if necessary.






