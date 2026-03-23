#pragma once

namespace Components
{
	class Screenshot : public Component
	{

	public:
		Screenshot();
		static void SaveTextureToFile(const std::string& name, IDirect3DTexture9* texture);

	private:
		static bool R_GetFrontBufferData(size_t width, size_t height, char* dstPixel);
		static void TakeScreenshot();
		static bool GetTextureData(IDirect3DTexture9* texture, OUT uint32_t& width, OUT uint32_t& height, OUT std::string& destinationRGBA);
		static bool WriteTarga(const std::string& name, size_t width, size_t height, const std::string& rgba);
	};
}
