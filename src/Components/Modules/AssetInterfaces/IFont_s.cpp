#include <STDInclude.hpp>
#include "IFont_s.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.hpp>

namespace Assets
{
	namespace
	{
		int PackFonts(const uint8_t* data, std::vector<uint16_t>& charset, Game::Glyph* glyphs, float pixel_height, unsigned char* pixels, int pw, int ph, int yOffset)
		{
			stbtt_fontinfo f;
			f.userdata = NULL;

			if (!stbtt_InitFont(&f, data, 0))
				return -1;

			std::memset(pixels, 0, pw * ph);

			int x = 1, y = 1, bottom_y = 1;

			float scale = stbtt_ScaleForPixelHeight(&f, pixel_height);

			int i = 0;

			for (auto& ch : charset)
			{
				int advance, lsb, x0, y0, x1, y1, gw, gh;

				int g = stbtt_FindGlyphIndex(&f, ch);

				stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
				stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);

				gw = x1 - x0;
				gh = y1 - y0;

				if (x + gw + 1 >= pw)
				{
					// Advance to next row
					y = bottom_y;
					x = 1;
				}

				if (y + gh + 1 >= ph)
				{
					// Check if we have ran out of the room
					return -i;
				}

				stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw, gh, pw, scale, scale, g);

				auto& glyph = glyphs[i++];

				glyph.letter = ch;
				glyph.s0 = x / static_cast<float>(pw);
				glyph.s1 = (x + gw) / static_cast<float>(pw);
				glyph.t0 = y / static_cast<float>(ph);
				glyph.t1 = (y + gh) / static_cast<float>(ph);
				glyph.pixelWidth = static_cast<char>(gw);
				glyph.pixelHeight = static_cast<char>(gh);
				glyph.x0 = static_cast<char>(x0);
				glyph.y0 = static_cast<char>(y0 + yOffset);
				glyph.dx = static_cast<char>(std::roundf(scale * advance));

				// Advance to next col
				x = x + gw + 1;

				// Expand bottom of current row if current glyph is bigger
				if (y + gh + 1 > bottom_y)
				{
					bottom_y = y + gh + 1;
				}
			}

			return bottom_y;
		}
	}

	void IFont_s::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		const auto* asset = header.font;

		if (asset->material)
		{
			builder->loadAsset(Game::ASSET_TYPE_MATERIAL, asset->material);
		}

		if (asset->glowMaterial)
		{
			builder->loadAsset(Game::ASSET_TYPE_MATERIAL, asset->glowMaterial);
		}
	}

	void IFont_s::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File fontDefFile(std::format("{}.json", name));
		Components::FileSystem::File fontFile(std::format("{}.ttf", name));

		if (!fontDefFile.exists() || !fontFile.exists())
		{
			return;
		}

		nlohmann::json fontDef;
		try
		{
			fontDef = nlohmann::json::parse(fontDefFile.getBuffer());
		}
		catch (const nlohmann::json::parse_error& ex)
		{
			Components::Logger::Error(Game::ERR_FATAL, "JSON Parse Error: {}. Font {} is invalid\n", ex.what(), name);
			return;
		}

		auto w = fontDef["textureWidth"].get<int>();
		auto h = fontDef["textureHeight"].get<int>();
			
		auto size = fontDef["size"].get<int>();
		auto yOffset = fontDef["yOffset"].get<int>();

		auto* pixels = builder->getAllocator()->allocateArray<uint8_t>(w * h);

		// Setup assets
		const auto* texName = builder->getAllocator()->duplicateString(Utils::String::VA("if_%s", name.data() + 6 /* skip "fonts/" */));
		const auto* fontName = builder->getAllocator()->duplicateString(name);
		const auto* glowMaterialName = builder->getAllocator()->duplicateString(Utils::String::VA("%s_glow", name.data()));

		auto* image = builder->getAllocator()->allocate<Game::GfxImage>();
		std::memcpy(image, Game::DB_FindXAssetHeader(Game::ASSET_TYPE_IMAGE, "gamefonts_pc").image, sizeof(Game::GfxImage));

		image->name = texName;

		auto* material = builder->getAllocator()->allocate<Game::Material>();
		std::memcpy(material, Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, "fonts/gamefonts_pc").material, sizeof(Game::Material));

		auto textureTable = builder->getAllocator()->allocate<Game::MaterialTextureDef>();
		std::memcpy(textureTable, material->textureTable, sizeof(Game::MaterialTextureDef));

		material->textureTable = textureTable;
		material->textureTable->u.image = image;
		material->info.name = fontName;

		auto* glowMaterial = builder->getAllocator()->allocate<Game::Material>();
		std::memcpy(glowMaterial, Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, "fonts/gamefonts_pc_glow").material, sizeof(Game::Material));

		glowMaterial->textureTable = material->textureTable;
		glowMaterial->info.name = glowMaterialName;

		std::vector<std::uint16_t> charset;

		if (fontDef["charset"].is_array())
		{
			nlohmann::json::array_t charsetArray = fontDef["charset"];
			for (auto& ch : charsetArray)
			{
				charset.push_back(static_cast<std::uint16_t>(ch.get<int>()));
			}

			// order matters
			std::ranges::sort(charset);

			for (std::uint16_t i = 32; i < 128; i++)
			{
				if (std::ranges::find(charset, i) == charset.end())
				{
					Components::Logger::Error(Game::ERR_FATAL, "Font {} missing codepoint {}", name.data(), i);
				}
			}
		}
		else
		{
			for (std::uint16_t i = 32; i < 128; i++)
			{
				charset.push_back(i);
			}
		}

		auto* font = builder->getAllocator()->allocate<Game::Font_s>();

		font->fontName = fontName;
		font->pixelHeight = size;
		font->material = material;
		font->glowMaterial = glowMaterial;
		font->glyphCount = static_cast<int>(charset.size());
		font->glyphs = builder->getAllocator()->allocateArray<Game::Glyph>(charset.size());

		// Generate glyph data
		int result = PackFonts(reinterpret_cast<const uint8_t*>(fontFile.getBuffer().data()), charset, font->glyphs, static_cast<float>(size), pixels, w, h, yOffset);

		if (result == -1)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Truetype font {} is broken", name);
		}
		else if (result < 0)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Texture size of font {} is not enough", name);
		}
		else if(h - result > size)
		{
			Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Texture of font {} have too much left over space: {}\n", name, h - result);
		}

		header->font = font;

		// Save generated materials
		Game::XAssetHeader tmpHeader;

		tmpHeader.image = image;
		Components::AssetHandler::StoreTemporaryAsset(Game::ASSET_TYPE_IMAGE, tmpHeader);

		tmpHeader.material = material;
		Components::AssetHandler::StoreTemporaryAsset(Game::ASSET_TYPE_MATERIAL, tmpHeader);

		tmpHeader.material = glowMaterial;
		Components::AssetHandler::StoreTemporaryAsset(Game::ASSET_TYPE_MATERIAL, tmpHeader);

		// Save generated image
		Utils::IO::CreateDir("userraw\\images");
			
		int fileSize = w * h * 4;
		int iwiHeaderSize = static_cast<int>(sizeof(Game::GfxImageFileHeader));

		Game::GfxImageFileHeader iwiHeader =
		{
			{ 'I', 'W', 'i' },
			/* version */
			8,
			/* flags */
			2,
			/* format */
			Game::IMG_FORMAT_BITMAP_RGBA,
			0,
			/* dimensions(x, y, z) */
			{ static_cast<short>(w), static_cast<short>(h), 1 },
			/* fileSizeForPicmip (mipSize in bytes + sizeof(GfxImageFileHeader)) */
			{ fileSize + iwiHeaderSize, fileSize, fileSize, fileSize }
		};

		std::string outIwi;
		outIwi.resize(fileSize + sizeof(Game::GfxImageFileHeader));

		std::memcpy(outIwi.data(), &iwiHeader, sizeof(Game::GfxImageFileHeader));

		// Generate RGBA data
		auto* rgbaPixels = outIwi.data() + sizeof(Game::GfxImageFileHeader);

		for (auto i = 0; i < w * h * 4; i += 4)
		{
			rgbaPixels[i + 0] = static_cast<char>(255);
			rgbaPixels[i + 1] = static_cast<char>(255);
			rgbaPixels[i + 2] = static_cast<char>(255);
			rgbaPixels[i + 3] = static_cast<char>(pixels[i / 4]);
		}

		Utils::IO::WriteFile(std::format("userraw\\images\\{}.iwi", texName), outIwi);
	}

	void IFont_s::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::Font_s, 24);
		AssertSize(Game::Glyph, 24);

		auto* buffer = builder->getBuffer();
		auto* asset = header.font;
		auto* dest = buffer->dest<Game::Font_s>();

		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->fontName)
		{
			buffer->saveString(asset->fontName);
			Utils::Stream::ClearPointer(&dest->fontName);
		}

		dest->material = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->material).material;
		dest->glowMaterial = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->glowMaterial).material;

		if (asset->glyphs)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->glyphs, asset->glyphCount);
			Utils::Stream::ClearPointer(&dest->glyphs);
		}

		buffer->popBlock();
	}
}
