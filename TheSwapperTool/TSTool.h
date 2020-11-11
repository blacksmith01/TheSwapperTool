#pragma once

#include "Common.hpp"
#include "BMFont.hpp"

struct ResText
{
	fs::path file_path;
	std::unordered_map<std::string, std::string> texts;
	std::unordered_map<std::string, std::wstring> wtexts;
};

struct ResFont
{
	static constexpr wchar_t DefaultChars1[] = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¥§©«°·»¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜßàáâãäåæçèéêëìíîïñòóôõö÷øùúûüÿ€ŠšŒœŸ–—‘’‚‛“”„";
	static constexpr wchar_t DefaultChars2[] = L"";
	static constexpr wchar_t DefaultChars3[] = L"";

	struct Node
	{
		std::string ch;
		wchar_t wch;
		uint32_t x;
		uint32_t y;
		uint32_t w;
		uint32_t h;
	};

	fs::path font_file_path;
	fs::path prop_file_path;

	std::vector<std::shared_ptr<Node>> nodes;

	uint32_t img_width;
	uint32_t img_height;
	std::vector<uint8_t> img_data;

	static wchar_t GetDefaultChar(std::size_t offset)
	{
		std::array<std::size_t, 3> lens = { _countof(DefaultChars1), _countof(DefaultChars2), _countof(DefaultChars3) };
		if (offset < lens[0]) {
			return DefaultChars1[offset];
		}
		else if (offset < lens[0] + lens[1]) {
			return DefaultChars2[offset - lens[0]];
		}
		else if (offset < lens[0] + lens[1] + lens[2]) {
			return DefaultChars2[offset - lens[0] - lens[1]];
		}

		throw std::exception("!char pos");
	}

	void GetMaxWidthHeight(uint32_t& width, uint32_t& height)
	{
		width = 0;
		height = 0;
		for (auto& n : nodes) {
			width = std::max(n->w, width);
			height = std::max(n->h, height);
		}
	}

	std::string GeneratePropertyString(bool include_max_height = true)
	{
		std::string str;
		str.reserve(1 + nodes.size() * 4 * 5);

		if (include_max_height) {
			uint32_t width, height;
			GetMaxWidthHeight(width, height);
			str += std::to_string(height);
		}

		for (auto& n : nodes) {
			str += stdext::strformat<256>("\r%d\r%d\r%d\r%d", n->h, n->w, n->x, n->y);
		}

		return str;
	}
};

struct ResLevel
{
	fs::path file_path;
	std::vector<std::string> lines;
};
struct ResLevelText
{
	std::map<int, std::wstring> texts;
};

class TSTool
{
public:
	TSTool();

public:
	bool GenerateKrChars(const fs::path& patch_path, std::string_view language, const fs::path& dst_file_path);
	bool GenerateKrLevels(const fs::path& orig_path, const fs::path& patch_path, std::string_view language);
	bool GenerateKrFontImg(const fs::path& orig_path, const fs::path default_bmf_path, const fs::path subtitle_bmf_path, const fs::path& output_path);

private:
	void LoadLevels(const fs::path& dir_path);
	std::shared_ptr<ResLevel> LoadLevel(const fs::path& path);

	void LoadTranslations(const fs::path& dir_path, std::string_view language);
	std::shared_ptr<ResText> LoadTranslation(const fs::path& path);
	std::string FindTranslation(std::string_view key);

	void LoadFonts(const fs::path& dir_path);
	std::shared_ptr<ResFont> LoadFont(const fs::path& path);
	void LoadFontProperty(const fs::path& path, std::vector<std::shared_ptr<ResFont::Node>>& nodes);
	
private:
	std::vector<std::shared_ptr<ResText>> texts;
	std::vector<std::shared_ptr<ResFont>> fonts;
	std::vector<std::shared_ptr<ResLevel>> levels;

	std::map<std::string, ResLevelText> leveltexts_linked;
	std::map<std::string, ResLevelText> leveltexts_fixed;
};