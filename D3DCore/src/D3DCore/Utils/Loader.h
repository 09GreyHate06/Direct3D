#pragma once

namespace d3dcore::utils
{
	struct ImageData
	{
		uint8_t* pixels;
		int width;
		int height;
		int nrComponents;
	};

	ImageData LoadImageFile(const std::string& filename, bool flipImageYOnLoad, int reqComponents);
	void FreeImageData(ImageData& data);

	std::string LoadTextFile(const std::string& filename);
}