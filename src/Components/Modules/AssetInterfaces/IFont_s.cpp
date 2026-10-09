#include "IFont_s.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.hpp>

namespace Assets
{
	namespace
	{
		int PackFonts(const uint8_t* data, std::vector<uint16_t>& charset, Game::Glyph* glyphs, float pixel_height, float glyphScale, unsigned char* pixels, int pw, int ph, int yOffset)
		{
			stbtt_fontinfo f;
			f.userdata = NULL;

			if (!stbtt_InitFont(&f, data, 0))
				return -1;

			std::memset(pixels, 0, pw * ph);

			int x = 1, y = 1, bottom_y = 1;

			// Fonts with tall ascenders (e.g. Arabic) come out small when fitted to the line height, glyphScale enlarges them
			float scale = stbtt_ScaleForPixelHeight(&f, pixel_height) * glyphScale;

			int i = 0;

			for (auto& ch : charset)
			{
				int advance, lsb, x0, y0, x1, y1, gw, gh;

				int g = stbtt_FindGlyphIndex(&f, ch);

				stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
				stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);

				gw = x1 - x0;
				gh = y1 - y0;

				// Glyph metrics are stored as signed chars
				constexpr auto maxMetric = std::numeric_limits<char>::max();
				if (gw > maxMetric || gh > maxMetric || std::roundf(scale * advance) > maxMetric || x0 < -128 || y0 + yOffset < -128)
				{
					Components::Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Glyph {} is too large for the font metrics, reduce the size or glyphScale\n", ch);
				}

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

	namespace
	{
		struct FontMaterials
		{
			Game::GfxImage* image;
			Game::Material* material;
			Game::Material* glowMaterial;
		};

		// Copies of the stock font materials that use the named texture
		FontMaterials CreateFontMaterials(const std::string& name, const std::string& imageName, Utils::Memory::Allocator* allocator)
		{
			auto* image = allocator->allocate<Game::GfxImage>();
			std::memcpy(image, Game::DB_FindXAssetHeader(Game::ASSET_TYPE_IMAGE, "gamefonts_pc").image, sizeof(Game::GfxImage));
			image->name = allocator->duplicateString(imageName);

			auto* material = allocator->allocate<Game::Material>();
			std::memcpy(material, Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, "fonts/gamefonts_pc").material, sizeof(Game::Material));

			auto* textureTable = allocator->allocate<Game::MaterialTextureDef>();
			std::memcpy(textureTable, material->textureTable, sizeof(Game::MaterialTextureDef));

			material->textureTable = textureTable;
			material->textureTable->u.image = image;
			material->info.name = allocator->duplicateString(name);

			auto* glowMaterial = allocator->allocate<Game::Material>();
			std::memcpy(glowMaterial, Game::DB_FindXAssetHeader(Game::ASSET_TYPE_MATERIAL, "fonts/gamefonts_pc_glow").material, sizeof(Game::Material));

			glowMaterial->textureTable = material->textureTable;
			glowMaterial->info.name = allocator->duplicateString(std::format("{}_glow", name));

			Game::XAssetHeader tmpHeader;

			tmpHeader.image = image;
			Components::AssetHandler::StoreTemporaryAsset(Game::ASSET_TYPE_IMAGE, tmpHeader);

			tmpHeader.material = material;
			Components::AssetHandler::StoreTemporaryAsset(Game::ASSET_TYPE_MATERIAL, tmpHeader);

			tmpHeader.material = glowMaterial;
			Components::AssetHandler::StoreTemporaryAsset(Game::ASSET_TYPE_MATERIAL, tmpHeader);

			return {image, material, glowMaterial};
		}
	}

	void IFont_s::LoadGlyphTable(Game::XAssetHeader* header, const std::string& name, const nlohmann::json& fontDef, Components::ZoneBuilder::Zone* builder)
	{
		auto* allocator = builder->getAllocator();
		const auto& glyphDefs = fontDef["glyphs"];

		// The texture is used as it is, it has to be in images/<image>.iwi
		const auto materials = CreateFontMaterials(name, fontDef["image"].get<std::string>(), allocator);

		auto* font = allocator->allocate<Game::Font_s>();
		font->fontName = allocator->duplicateString(name);
		font->pixelHeight = fontDef["pixelHeight"].get<int>();
		font->material = materials.material;
		font->glowMaterial = materials.glowMaterial;
		font->glyphCount = static_cast<int>(glyphDefs.size());
		font->glyphs = allocator->allocateArray<Game::Glyph>(glyphDefs.size());

		for (std::size_t i = 0; i < glyphDefs.size(); ++i)
		{
			const auto& def = glyphDefs[i];
			auto& glyph = font->glyphs[i];

			glyph.letter = def["letter"].get<std::uint16_t>();
			glyph.x0 = static_cast<char>(def["x0"].get<int>());
			glyph.y0 = static_cast<char>(def["y0"].get<int>());
			glyph.dx = static_cast<char>(def["dx"].get<int>());
			glyph.pixelWidth = static_cast<char>(def["pixelWidth"].get<int>());
			glyph.pixelHeight = static_cast<char>(def["pixelHeight"].get<int>());
			glyph.s0 = def["s0"].get<float>();
			glyph.t0 = def["t0"].get<float>();
			glyph.s1 = def["s1"].get<float>();
			glyph.t1 = def["t1"].get<float>();
		}

		header->font = font;
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
		if (!fontDefFile.exists())
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

		// Fonts dumped from the game come with their glyph table and texture
		if (fontDef.contains("glyphs"))
		{
			LoadGlyphTable(header, name, fontDef, builder);
			return;
		}

		Components::FileSystem::File fontFile(std::format("{}.ttf", name));
		if (!fontFile.exists())
		{
			return;
		}

		// Without an explicit size, match the line height of the stock font this one replaces
		int size;
		if (fontDef.contains("size"))
		{
			size = fontDef["size"].get<int>();
		}
		else if (fontDef.contains("baseFont"))
		{
			const auto baseFontName = fontDef["baseFont"].get<std::string>();
			const auto* baseFont = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_FONT, baseFontName.data()).font;
			if (baseFont == nullptr)
			{
				Components::Logger::Error(Game::ERR_FATAL, "Base font {} of font {} was not found", baseFontName, name);
				return;
			}

			size = baseFont->pixelHeight;
		}
		else
		{
			Components::Logger::Error(Game::ERR_FATAL, "Font {} needs a size or a baseFont", name);
			return;
		}

		auto yOffset = fontDef.value("yOffset", 0);
		auto glyphScale = fontDef.value("glyphScale", 1.0f);

		// Console fonts draw text above the y position. Sit on the stock font's baseline and match its
		// capital height, so the replacement lines up with the menus made for the stock font.
		if (fontDef.value("alignToBaseFont", false) && fontDef.contains("baseFont"))
		{
			auto* baseFont = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_FONT, fontDef["baseFont"].get<std::string>().data()).font;
			const auto* baseCapital = baseFont ? Game::R_GetCharacterGlyph(baseFont, 'A') : nullptr;

			stbtt_fontinfo info{};
			int left, bottom, right, top;
			if (baseCapital && stbtt_InitFont(&info, reinterpret_cast<const uint8_t*>(fontFile.getBuffer().data()), 0) && stbtt_GetCodepointBox(&info, 'A', &left, &bottom, &right, &top))
			{
				const auto capitalHeight = static_cast<float>(top - bottom) * stbtt_ScaleForPixelHeight(&info, static_cast<float>(size));
				glyphScale = static_cast<float>(baseCapital->pixelHeight) / capitalHeight * fontDef.value("capitalScale", 1.0f);
				yOffset += baseCapital->y0 + baseCapital->pixelHeight;
			}
		}

		// Setup assets
		const auto texName = std::format("if_{}", name.substr(6 /* skip "fonts/" */));
		const auto materials = CreateFontMaterials(name, texName, builder->getAllocator());

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

		font->fontName = materials.material->info.name;
		font->pixelHeight = size;
		font->material = materials.material;
		font->glowMaterial = materials.glowMaterial;
		font->glyphCount = static_cast<int>(charset.size());
		font->glyphs = builder->getAllocator()->allocateArray<Game::Glyph>(charset.size());

		// Generate glyph data, growing the texture until every glyph fits when no size is given
		const auto autoTextureSize = !fontDef.contains("textureWidth") || !fontDef.contains("textureHeight");
		auto w = autoTextureSize ? 256 : fontDef["textureWidth"].get<int>();
		auto h = autoTextureSize ? 256 : fontDef["textureHeight"].get<int>();

		std::vector<uint8_t> pixels;
		int result;
		while (true)
		{
			pixels.resize(w * h);
			result = PackFonts(reinterpret_cast<const uint8_t*>(fontFile.getBuffer().data()), charset, font->glyphs, static_cast<float>(size), glyphScale, pixels.data(), w, h, yOffset);

			if (!autoTextureSize || result >= 0 || h >= 4096)
			{
				break;
			}

			if (w > h) h *= 2;
			else w *= 2;
		}

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
