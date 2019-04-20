# Interface

The interface of VMT Editor is split into three basic parts:

- The Parameters, Proxies and Additional Parameters tabs
- VMT Preview
- Message Log

## Parameters tab

The _Parameters_ tab is where the most of the main functionality of the VMT Editor lies. 

?> How to use it is covered in [Material Creation](mmaterial.md).

### Shader

Here you select the shader, depending on the use of your material. Most of the time this would be either _LightmappedGeneric_ for brush textures and decals, _WorldVertexTransition_ for displacement blend materials or _VertexLitGeneric_ for model materials. Next to the groupbox are buttons to quickly select the appropriate shader.

### Toolbar

Toolbar that contains shortcuts for the most used parameter groups. Other groups can be found under _Add..._ menu item, or you can add any missing parameters under the _Additional Parameters_ tab.

## Proxies tab

Edit proxies or other nested parameters with syntax validation and autocompletion. Requires `Proxies {}` base group.

## Additional Parameters tab

Here you can add all the parameters VMT Editor doesn't have native support for. Parameters usually require `$` or `%` in front of the parameter name.

## VMT Preview

VMT Preview contains the preview of the .vmt file that will be exported. You can also write your own parameters directly in the preview and click the _Parse_ button to save them.

You can also paste parameters from clipboard directly on the VMT Editor window and they will be automatically parsed.

## Tools

Tools menu contains some useful actions that may speed up your workflow.

_Reconvert All_ will run the reconvert action on all textures in the open material.

_Create Blend Tool Texture_ automatically creates a "split" tool texture for Hammer texture browser, based on current loaded textures.

_Refresh VMT Preveiw_ and _Parse VMT Preview_ behave the same as buttons on the bottom of the _VMT Preview_.

_Refresh Material In-Game_ sends a command to current open game to refresh the current open material. May crash the game.

It also contains the  [Convert to VTF](converting.md#convert-to-vtf-dialog) dialog and [Batch VMT](batch.md) dialog.