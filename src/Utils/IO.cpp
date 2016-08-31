#include "STDInclude.hpp"

namespace Utils
{
	namespace IO
	{
		bool FileExists(std::string file)
		{
			return std::ifstream(file).good();
		}

		void WriteFile(std::string file, std::string data)
		{
			std::ofstream stream(file, std::ios::binary);

			if (stream.is_open())
			{
				stream.write(data.data(), data.size());
				stream.close();
			}
		}

		std::string ReadFile(std::string file)
		{
			std::string buffer;

			if (FileExists(file))
			{
				std::streamsize size = 0;
				std::ifstream stream(file, std::ios::binary);
				if (!stream.is_open()) return buffer;

				stream.seekg(0, std::ios::end);
				size = stream.tellg();
				stream.seekg(0, std::ios::beg);

				if (size > -1)
				{
					buffer.clear();
					buffer.resize((uint32_t)size);

					stream.read(const_cast<char*>(buffer.data()), size);
				}

				stream.close();
			}

			return buffer;
		}

		void CreateDirectory(std::string dir)
		{
			char opath[MAX_PATH];
			char *p;
			size_t len;

			strncpy_s(opath, dir.data(), sizeof(opath));
			len = strlen(opath);

			if (opath[len - 1] == L'/')
			{
				opath[len - 1] = L'\0';
			}

			for (p = opath; *p; p++)
			{
				if (*p == L'/' || *p == L'\\')
				{
					*p = L'\0';

					if (_access(opath, 0))
					{
						_mkdir(opath);
					}

					*p = L'\\';
				}
			}

			if (_access(opath, 0))
			{
				_mkdir(opath);
			}
		}
	}
}
