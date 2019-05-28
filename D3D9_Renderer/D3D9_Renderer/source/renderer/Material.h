#pragma once

#include <d3d9.h>

#include"d3d9/Shader.h"

namespace renderer
{
	class Material
	{
	public:
		Material();
		~Material();

		enum class TextureType
		{
			Diffuse,
			Normal
		};
        inline void SetPath(std::string path) { m_difftexturePath = path; }
		void SetTexture(TextureType texType, IDirect3DTexture9* texture);
		IDirect3DTexture9* GetTextureOfType(TextureType texType);
		IDirect3DTexture9** GetPtrToTextureOfType(TextureType texType);

	private:
        std::string m_difftexturePath;
		IDirect3DTexture9* m_diffuseTexture;
		IDirect3DTexture9* m_normalTexture;
	};
}
