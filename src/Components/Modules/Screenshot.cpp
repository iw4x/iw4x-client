#include <STDInclude.hpp>

#include "GSC/Script.hpp"

#include "Window.hpp"

#include "Screenshot.hpp"

#define OK(x) SUCCEEDED(lastError = x)
#define PRINT_LAST_ERROR() Components::Logger::PrintError(Game::CON_CHANNEL_GFX, Game::R_ErrorDescription(lastError));

namespace Components
{
	bool Screenshot::R_GetFrontBufferData(size_t width, size_t height, char* dstPixel)
	{
		HRESULT lastError{};

		const auto device = *Game::dx_ptr;

		bool success = false;

		IDirect3DSurface9* surface{};
		if (OK(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE::D3DBACKBUFFER_TYPE_MONO, &surface)))
		{
			D3DSURFACE_DESC description{};
			surface->GetDesc(&description);

			IDirect3DSurface9* offscreenSurface{};

			if (OK(device->CreateOffscreenPlainSurface(
				description.Width,
				description.Height,
				description.Format,
				D3DPOOL::D3DPOOL_SYSTEMMEM,
				&offscreenSurface,
				NULL
			)))
			{
				if (OK(device->GetFrontBufferData(0, offscreenSurface)))
				{
					D3DLOCKED_RECT rectangle{};
					RECT source{};
					source.right = description.Width;
					source.bottom = description.Height;
					if (OK(offscreenSurface->LockRect(&rectangle, &source, D3DLOCK_READONLY)))
					{
						size_t targetPosition = 0;
						size_t sourcePosition = 0;

						const auto sourcePixel = reinterpret_cast<const char*>(rectangle.pBits);

						for (size_t y = 0; y < description.Height; y++)
						{
							for (size_t x = 0; x < description.Width; x++)
							{
								sourcePosition = y * rectangle.Pitch + x * 4;

								std::memcpy(&dstPixel[targetPosition], &sourcePixel[sourcePosition], 4);
								targetPosition += 4;
							}
						}

						if (OK(offscreenSurface->UnlockRect()))
						{
							// cool
							success = true;
						}
						else
						{
							PRINT_LAST_ERROR();
						}
					}
					else
					{
						PRINT_LAST_ERROR();
					}

				}
				else
				{
					PRINT_LAST_ERROR();
				}

				offscreenSurface->Release();
			}
			else
			{
				PRINT_LAST_ERROR();
			}

			surface->Release();
		}
		else
		{
			PRINT_LAST_ERROR();
		}

		return success;
	}

	bool Screenshot::GetTextureData(IDirect3DTexture9* texture, OUT uint32_t& width, OUT uint32_t& height, OUT std::string& destinationRGBA)
	{
		bool success = false;
		HRESULT lastError{};

		D3DSURFACE_DESC description{};
		if (OK(texture->GetLevelDesc(0, &description)))
		{
			if (description.Format != D3DFORMAT::D3DFMT_A8R8G8B8)
			{
				// Sorry, need to use the DirectXTex library for that stuff ;/
				return false;
			}

			const auto device = *Game::dx_ptr;

			D3DLOCKED_RECT rectangle{};
			RECT source{};
			source.right = description.Width;
			source.bottom = description.Height;

			destinationRGBA = std::string(description.Width * description.Height * 4, '\0');

			if (OK(texture->LockRect(0, &rectangle, &source, D3DLOCK_READONLY)))
			{
				size_t targetPosition = 0;
				size_t sourcePosition = 0;

				const auto sourcePixel = reinterpret_cast<const char*>(rectangle.pBits);

				for (size_t y = 0; y < description.Height; y++)
				{
					for (size_t x = 0; x < description.Width; x++)
					{
						sourcePosition = y * rectangle.Pitch + x * 4;

						std::memcpy(&destinationRGBA[targetPosition], &sourcePixel[sourcePosition], 4);
						targetPosition += 4;
					}
				}

				if (OK(texture->UnlockRect(0)))
				{
					// cool
					success = true;
				}
				else
				{
					PRINT_LAST_ERROR();
				}
			}
			else
			{
				PRINT_LAST_ERROR();
			}
		}
		else
		{
			PRINT_LAST_ERROR();
		}

		return success;
	}

	void Screenshot::SaveTextureToFile(const std::string& name, IDirect3DTexture9* texture)
	{
		std::string rgba{};
		uint32_t width{};
		uint32_t height{};
		if (GetTextureData(texture, width, height, rgba))
		{
			WriteTarga(name, width, height, rgba);
		}
	}

	bool Screenshot::WriteTarga(const std::string& name, size_t width, size_t height, const std::string& rgba)
	{
		Utils::Memory::Allocator allocator{};

		uint8_t header[18]{};

		// Write TGA Header
		header[2] = 2; // True Color RGBA

		header[12] = width & 255;
		header[13] = width >> 8;
		header[14] = height & 255;
		header[15] = height >> 8;
		header[16] = 32;
		header[17] = 0x20;


		const auto dataLength = rgba.size() + ARRAYSIZE(header);
		const auto buff = allocator.allocateArray<char>(dataLength);

		std::memcpy(buff, header, ARRAYSIZE(header));
		std::memcpy(&buff[ARRAYSIZE(header)], rgba.data(), rgba.size());

		const std::string outBuffer(buff, dataLength);
		return Utils::IO::WriteFile(name, outBuffer);
	}

	void Screenshot::TakeScreenshot()
	{
		const auto& videoConfig = Game::cls->vidConfig;
		const auto v_displayWidth = videoConfig.displayWidth;
		const auto v_displayHeight = videoConfig.displayHeight;

		Utils::Memory::Allocator allocator{};
		const auto dataLength = v_displayWidth * v_displayHeight * 4;
		const auto buff = allocator.allocateArray<char>(dataLength);

		if (R_GetFrontBufferData(v_displayWidth, v_displayHeight, buff))
		{
			const std::string outBuffer(buff, dataLength);

			Utils::IO::CreateDir("screenshots");
			WriteTarga(Utils::String::VA("screenshots/screenshot_%lld.tga", std::time(nullptr)), v_displayWidth, v_displayHeight, outBuffer);
			Components::Logger::Print("Screenshot success!");
		}
		else {
			Components::Logger::Print("Screenshot failure!");
		}
	}

	Screenshot::Screenshot()
	{
		Components::Command::Add("screenshot", []() {
			TakeScreenshot();
		});

		Components::GSC::Script::AddFunction("screenshot", []() {
			TakeScreenshot();
		});

		UIScript::Add("screenshot", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info) {
			TakeScreenshot();
		});
	}
}


#undef PRINT_LAST_ERROR
#undef OK
