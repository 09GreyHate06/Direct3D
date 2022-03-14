#include "d3dcpch.h"
#include "Loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

namespace d3dcore::utils
{

	ImageData LoadImageFile(const std::string& filename, bool flipImageYOnLoad, int reqComponents)
	{
		ImageData imageData = {};
		stbi_set_flip_vertically_on_load(flipImageYOnLoad);
		imageData.pixels = stbi_load(filename.c_str(), &imageData.width, &imageData.height, &imageData.nrComponents, reqComponents);

		if (!imageData.pixels)
			D3DC_CORE_LOG_WARN("Failed to load image file: {0}", filename);

		return imageData;
	}

	void FreeImageData(ImageData& data)
	{
		stbi_image_free(data.pixels);
		data.pixels = nullptr;
	}

	std::string LoadTextFile(const std::string& filename)
	{
		std::string result;
		std::ifstream in(filename, std::ios::in | std::ios::binary);

		if (!in)
		{
			D3DC_CORE_LOG_WARN("Failed to load text file: {0}", filename);
			return result;
		}

		in.seekg(0, std::ios::end);
		result.resize(static_cast<uint32_t>(in.tellg()));
		in.seekg(0, std::ios::beg);
		in.read(result.data(), result.size());
		in.close();

		return result;
	}

}