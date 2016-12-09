#ifndef SHADER_H
#define SHADER_H

#include <QString>
#include <QVector>
#include <QMap>

struct Shader {

	enum Shaders {
		S_Cable = 0,
		S_Decal, // 1
		S_DecalModulate, // 2
		S_Infected, // 3
		S_LightmappedGeneric, // 4
		S_Modulate, // 5
		S_MonitorScreen, // 6
		S_Patch, // 7
		S_Predator, // 8
		S_Refract, // 9
		S_ShatteredGlass, // 10
		S_Sprite, // 11
		S_SpriteCard, // 12
		S_UnlitGeneric, // 13
		S_UnlitTwoTexture, // 14
		S_VertexLitGeneric, // 15
		S_Water, // 16
		S_WorldVertexTransition, // 17
		S_Custom, // 18
		S_Lightmapped_4WayBlend // 19
	};

	enum Groups {

		G_Base_Texture = 0,
		G_Base_Texture2, // 1
		G_Transparency, // 2
		G_Detail, // 3
		G_Color, // 4
		G_Other, // 5
		G_Phong, // 6
		G_Reflection, // 7
		G_Self_Illumination, // 8
		G_Rim_Light, // 9
		G_Water, // 10
		G_Flowmap, // 11
		G_WaterReflection, // 12
		G_Refraction, // 13
		G_Fog, // 14
		G_Scroll, // 15
		G_Base_Texture_Texture_Transforms, // 16
		G_Bumpmap_Texture_Transforms, // 17
		G_Miscellaneous, // 18
		G_Refract, // 19
		G_Patch, // 20
		G_Sprite, // 21
		G_UnlitTwoTexture, // 22
		G_Base_Texture3, // 23
		G_Base_Texture4, // 24
		G_PhongBrush // 25
	};

	//----------------------------------------------------------------------------------------//

	Shader (const QString& name = "", bool enabled = false, QVector< Groups > groups = QVector< Groups >());

	QString name;

	bool enabled;

	QVector< Groups > groups;

	//----------------------------------------------------------------------------------------//

	bool operator== (const Shader& other) const;

	//----------------------------------------------------------------------------------------//

	static QString delegateText( Groups index );

	static QString convert( Shaders index );

	/*!
	 * Converts the passed string to a Shader enum value.
	 *
	 * The casing does not matter as long as the shader is know.
	 * If the shader is not known, S_Custom is returned.
	 */
	static Shader::Shaders convert(const QString &text);

	static int convertGroup( QString text );
};

#endif // SHADER_H
