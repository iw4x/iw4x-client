#include "STDInclude.hpp"

namespace Components
{
	std::queue<Toast::UIToast> Toast::Queue;
	std::mutex Toast::Mutex;

	void Toast::Show(const char* image, const char* title, const char* description, int length)
	{
		Toast::Mutex.lock();
		Toast::Queue.push({ image, title, description, length, 0 });
		Toast::Mutex.unlock();
	}

	void Toast::Draw(UIToast* toast)
	{
#pragma warning(push)
#pragma warning(disable: 4244)
		if (!toast) return;

		int width = Renderer::Width();
		int height = Renderer::Height();
		int slideTime = 100;

		int duration = toast->Length;
		int startTime = toast->Start;

		int aCorners = 0; // Adjust the corners. They seem to have a 1px border
		int cornerSize = 15;
		int bHeight = 74;
		int bWidth = 200;

		int imgDim = 60;

		Game::Material* circle = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "circle").material;
		Game::Material* white = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "white").material;
		Game::Material* image = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, toast->Image.data()).material;
		Game::Font* font = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_FONT, "fonts/normalFont").font;
		Game::vec4_t color = { 0, 0, 0, 0.7f };
		Game::vec4_t wColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		if (Game::Com_Milliseconds() < startTime || (startTime + duration) < Game::Com_Milliseconds()) return;

		// Fadein stuff
		else if (Game::Com_Milliseconds() - startTime < slideTime)
		{
			height /= 5;
			height *= 4;

			int diffH = Renderer::Height() / 5;
			int diff = Game::Com_Milliseconds() - startTime;
			double scale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
			diffH *= scale;
			height += diffH;
		}

		// Fadeout stuff
		else if (Game::Com_Milliseconds() - startTime > (duration - slideTime))
		{
			height /= 5;
			height *= 4;

			int diffH = Renderer::Height() / 5;
			int diff = (startTime + duration) - Game::Com_Milliseconds();
			double scale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
			diffH *= scale;
			height += diffH;
		}

		else
		{
			height /= 5;
			height *= 4;
		}

		height += bHeight / 2 + aCorners - cornerSize;

		// Calculate width data
		int iOffset = (bHeight - imgDim) / 2 + aCorners;
		int iOffsetLeft = iOffset * 2;
		int titleSize = Game::R_TextWidth(toast->Title.data(), 0x7FFFFFFF, font);
		int descrSize = Game::R_TextWidth(toast->Desc.data(), 0x7FFFFFFF, font);
		bWidth = iOffsetLeft * 3 + imgDim + std::max(titleSize, descrSize);

		// Make stuff divisible by 2
		// Otherwise there are overlapping images
		// and I'm too lazy to figure out the actual problem :P
		bWidth += (bWidth % 2);
		bHeight += (bHeight % 2);

		// Corners
		Game::R_AddCmdDrawStretchPic(width / 2 - bWidth / 2, height - bHeight / 2, cornerSize, cornerSize, 0, 0, 0.5f, 0.5f, color, circle);                           // Top-Left
		Game::R_AddCmdDrawStretchPic(width / 2 + bWidth / 2 - cornerSize, height - bHeight / 2, cornerSize, cornerSize, 0.5f, 0, 0, 0.5f, color, circle);              // Top-Right
		Game::R_AddCmdDrawStretchPic(width / 2 - bWidth / 2, height + bHeight / 2 - cornerSize, cornerSize, cornerSize, 0, 0.5f, 0.5f, 0, color, circle);              // Bottom-Left
		Game::R_AddCmdDrawStretchPic(width / 2 + bWidth / 2 - cornerSize, height + bHeight / 2 - cornerSize, cornerSize, cornerSize, 0.5f, 0.5f, 0, 0, color, circle); // Bottom-Right

		// Border
		Game::R_AddCmdDrawStretchPic(width / 2 - bWidth / 2 + cornerSize, height - bHeight / 2 + aCorners, bWidth - cornerSize * 2, cornerSize - aCorners, 0, 0, 1.0f, 1.0f, color, white);    // Top
		Game::R_AddCmdDrawStretchPic(width / 2 - bWidth / 2 + cornerSize, height + bHeight / 2 - cornerSize, bWidth - cornerSize * 2, cornerSize - aCorners, 0, 0, 1.0f, 1.0f, color, white);  // Bottom
		Game::R_AddCmdDrawStretchPic(width / 2 - bWidth / 2 + aCorners, height - bHeight / 2 + cornerSize, cornerSize - aCorners, bHeight - cornerSize * 2, 0, 0, 1.0f, 1.0f, color, white);   // Left
		Game::R_AddCmdDrawStretchPic(width / 2 + bWidth / 2 - cornerSize, height - bHeight / 2 + cornerSize, cornerSize - aCorners, bHeight - cornerSize * 2, 0, 0, 1.0f, 1.0f, color, white); // Right

																																																					// Center
		Game::R_AddCmdDrawStretchPic(width / 2 - (bWidth - cornerSize * 2) / 2, height - (bHeight - cornerSize * 2) / 2, bWidth - cornerSize * 2, bHeight - cornerSize * 2, 0, 0, 1.0f, 1.0f, color, white);

		// Image
		Game::R_AddCmdDrawStretchPic(width / 2 - bWidth / 2 + iOffsetLeft, height - bHeight / 2 + iOffset, imgDim, imgDim, 0, 0, 1.0f, 1.0f, wColor, image);

		// Text
		int leftText = width / 2 - bWidth / 2 - cornerSize + iOffsetLeft * 2 + imgDim;
		int rightText = width / 2 + bWidth / 2 - cornerSize - aCorners - iOffsetLeft;
		Game::R_AddCmdDrawText(toast->Title.data(), 0x7FFFFFFF, font, leftText + (rightText - leftText) / 2 - titleSize / 2 + cornerSize, height - bHeight / 2 + cornerSize * 2 + 7, 1.0f, 1.0f, 0, wColor, 0); // Title
		Game::R_AddCmdDrawText(toast->Desc.data(), 0x7FFFFFFF, font, leftText + (rightText - leftText) / 2 - descrSize / 2 + cornerSize, height - bHeight / 2 + cornerSize * 2 + 33, 1.0f, 1.0f, 0, wColor, 0); // Description
#pragma warning(pop)
	}

	void Toast::Handler()
	{
		if (Toast::Queue.empty()) return;

		Toast::Mutex.lock();

		Toast::UIToast* toast = &Toast::Queue.front();

		// Set start time
		if (!toast->Start)
		{
			toast->Start = Game::Com_Milliseconds();
		}

		if ((toast->Start + toast->Length) < Game::Com_Milliseconds())
		{
			Toast::Queue.pop();
		}
		else
		{
			Toast::Draw(toast);
		}

		Toast::Mutex.unlock();
	}

	Toast::Toast()
	{
		Renderer::OnFrame(Toast::Handler);

		Command::Add("testtoast", [] (Command::Params)
		{
			Toast::Show("specialty_nuke", "Test", "This is a test toast", 3000);
		});
	}

	Toast::~Toast()
	{
		Toast::Queue = std::queue<Toast::UIToast>();
	}
}
