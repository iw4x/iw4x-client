namespace Utils
{
	std::string GetMimeType(std::string url);
	std::string ParseChallenge(std::string data);

	template <typename T> void Merge(std::vector<T>* target, T* source, size_t length)
	{
		if (source)
		{
			for (size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T> void Merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto &entry : source)
		{
			target->push_back(entry);
		}
	}
}
