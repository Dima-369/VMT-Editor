#include "shader.h"

#include "utilities.h"
#include "messagebox.h"

Shader::Shader(const QString& name, bool enabled, QVector< Groups > groups) :
	name(name),
	enabled(enabled),
	groups(groups)
{

}

bool Shader::operator== (const Shader& other) const
{
	return (this->name == other.name) && (this->enabled == other.enabled) && (this->groups == other.groups);
}

QString Shader::delegateText( Groups index ) {

	switch(index)
	{
	case 0: return "Texture > Base Texture";

	case 1: return "Texture > Base Texture 2";

	case 2: return "Texture > Transparency";

	case 3: return "Texture > Detail Texture";

	case 4: return "Texture > Color";

	case 5: return "Texture > Other";

	case 6: return "Shading > Phong";

	case 7: return "Shading > Reflection";

	case 8: return "Shading > Self Illumination";

	case 9: return "Shading > Rim Light";

	case 10: return "Water";

	case 11: return "Water > Flowmap";

	case 12: return "Water > Reflection";

	case 13: return "Water > Refraction";

	case 14: return "Water > Fog";

	case 15: return "Other > Scroll";

	case 16: return "Other > Base Texture - Texture Transforms";

	case 17: return "Other > Bumpmap - Texture Transforms";

	case 18: return "Other > Miscellaneous";

	case 19: return "Custom > Refract";

	case 20: return "Custom > Patch";

	case 21: return "Custom > Sprite";

	case 22: return "Custom > UnlitTwoTexture";

	case 23: return "Texture > Base Texture 3";

	case 24: return "Texture > Base Texture 4";

	case 25: return "Texture > Phong Brush";

	default:

		MsgBox::warning(NULL, "Wrong parameter passed!",
						"static QString delegateText( Groups index )\n\nParameter: index: " + Str(index) + " is not valid!");

		return "";
	}
}

QString Shader::convert( Shaders index ) {

	switch(index)
	{
	case 0: return "Cable";

	case 1: return "Decal";

	case 2: return "DecalModulate";

	case 3: return "Infected";

	case 4: return "LightmappedGeneric";

	case 5: return "Modulate";

	case 6: return "MonitorScreen";

	case 7: return "Patch";

	case 8: return "Predator";

	case 9: return "Refract";

	case 10: return "Shattered Glass";

	case 11: return "Sprite";

	case 12: return "SpriteCard";

	case 13: return "UnlitGeneric";

	case 14: return "UnlitTwoTexture";

	case 15: return "VertexLitGeneric";

	case 16: return "Water";

	case 17: return "WorldVertexTransition";

	case 18: return "Custom";

	case 19: return "Lightmapped_4WayBlend";

	default:

		MsgBox::warning(NULL, "Wrong parameter passed!",
						"static QString convertShaders( Shaders index )\n\nParameter: index: " + Str(index) + " is not valid!");

		return "";
	}
}


Shader::Shaders Shader::convert(const QString &text)
{
	#define CHECK(s, e) if (lower == s) return e;

	const QString lower = text.toLower();
	CHECK("cable", S_Cable)
	CHECK("decal", S_Decal)
	CHECK("decalmodulate", S_DecalModulate)
	CHECK("infected", S_Infected)
	CHECK("lightmappedgeneric", S_LightmappedGeneric)
	CHECK("modulate", S_Modulate)
	CHECK("monitorscreen", S_MonitorScreen)
	CHECK("patch", S_Patch)
	CHECK("predator", S_Predator)
	CHECK("refract", S_Refract)
	CHECK("shattered glass", S_ShatteredGlass)
	CHECK("sprite", S_Sprite)
	CHECK("spritecard", S_SpriteCard)
	CHECK("unlitgeneric", S_UnlitGeneric)
	CHECK("unlittwotexture", S_UnlitTwoTexture)
	CHECK("vertexlitgeneric", S_VertexLitGeneric)
	CHECK("water", S_Water)
	CHECK("worldvertextransition", S_WorldVertexTransition)
	CHECK("lightmapped_4wayblend", S_Lightmapped_4WayBlend)
	return S_Custom;
}

int Shader::convertGroup( QString text )
{
	if     (text == "Texture > Base Texture")						{ return 0; }
	else if(text == "Texture > Base Texture 2")						{ return 1; }
	else if(text == "Texture > Transparency")						{ return 2; }
	else if(text == "Texture > Detail Texture")						{ return 3; }
	else if(text == "Texture > Color")								{ return 4; }
	else if(text == "Texture > Other")								{ return 5; }
	else if(text == "Shading > Phong")								{ return 6; }
	else if(text == "Shading > Reflection")							{ return 7; }
	else if(text == "Shading > Self Illumination")					{ return 8; }
	else if(text == "Shading > Rim Light")							{ return 9; }
	else if(text == "Water")										{ return 10; }
	else if(text == "Water > Flowmap")								{ return 11; }
	else if(text == "Water > Reflection")							{ return 12; }
	else if(text == "Water > Refraction")							{ return 13; }
	else if(text == "Water > Fog")									{ return 14; }
	else if(text == "Other > Scroll")								{ return 15; }
	else if(text == "Other > Base Texture - Texture Transforms")	{ return 16; }
	else if(text == "Other > Bumpmap - Texture Transforms")			{ return 17; }
	else if(text == "Other > Miscellaneous")						{ return 18; }
	else if(text == "Custom > Refract")								{ return 19; }
	else if(text == "Custom > Patch")								{ return 20; }
	else if(text == "Custom > Sprite")								{ return 21; }
	else if(text == "Custom > UnlitTwoTexture")						{ return 22; }
	else if(text == "Texture > Base Texture 3")						{ return 23; }
	else if(text == "Texture > Base Texture 4")						{ return 24; }
	else if(text == "Shading > PhongBrush")                         { return 25; }
	else															{ return -1; }
}
