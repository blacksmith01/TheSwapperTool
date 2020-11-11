#include "TSTool.h"

TSTool::TSTool()
{
	leveltexts_linked.emplace("area1_obstacle", ResLevelText{ { {920, L"tuto_grab"} } });
	leveltexts_linked.emplace("beginning", ResLevelText{ { {5134, L"tuto_jump"}, {5514, L"tuto_mouse"} } });
	leveltexts_linked.emplace("console_tutorial", ResLevelText{ { {200, L"tuto_consoles"} } });
	leveltexts_linked.emplace("intro_pod_eject_planet", ResLevelText{ { {141, L"intro_life_pod_ejected"} } });
	leveltexts_linked.emplace("planet_ending_v2", ResLevelText{ { {2791, L"tuto_ending"}, {2806, L"tuto_ending2"} } });
	leveltexts_linked.emplace("puzzle0", ResLevelText{ { {480, L"tuto_map"}, {507, L"tuto_clone_collecting"}, {748, L"tuto_cloning1"}, {762, L"tuto_cloning2"} } });
	leveltexts_linked.emplace("puzzle0b", ResLevelText{ { {449, L"tuto_clones_amount"} } });
	leveltexts_linked.emplace("puzzle1", ResLevelText{ { {557, L"tuto_clone_collecting"} } });
	leveltexts_linked.emplace("puzzle2", ResLevelText{ { {67, L"tuto_swapping"} } });
	leveltexts_linked.emplace("puzzle4", ResLevelText{ { {275, L"tuto_blue_light"} } });
	leveltexts_linked.emplace("puzzle5_new", ResLevelText{ { {232, L"tuto_red_light"} } });
	leveltexts_linked.emplace("swapperdevice_room", ResLevelText{ { {161, L"tuto_cloning1"}, {176, L"tuto_cloning2"} } });

	leveltexts_fixed.emplace("hub", ResLevelText{ { {3010, L"섹터 1"}, {3325, L"섹터 2"} } });
	leveltexts_fixed.emplace("intro_planet", ResLevelText{ { {110, L"테세우스 연구 정거장, 행성 코리 V"} } });
}

void TSTool::LoadLevels(const fs::path& dir_path)
{
	for (auto i : fs::recursive_directory_iterator{ dir_path }) {
		if (i.is_regular_file() && fs::path(i).extension() == ".lvl") {

			auto filename = fs::path(i).stem().string();
			if (!leveltexts_linked.contains(filename) && !leveltexts_fixed.contains(filename))
				continue;
			levels.push_back(LoadLevel(i));
		}
	}
}

std::shared_ptr<ResLevel> TSTool::LoadLevel(const fs::path& path)
{
	auto res = std::make_shared<ResLevel>();
	res->file_path = path;

	std::ifstream is(path);
	std::string line;
	while (std::getline(is, line))
	{
		std::istringstream iss(line);
		res->lines.push_back(line);
	}

	return res;
}

void TSTool::LoadTranslations(const fs::path& dir_path, std::string_view language)
{
	for (auto i : fs::recursive_directory_iterator{ dir_path }) {
		if (i.is_regular_file() && fs::path(i).extension() == language) {
			texts.push_back(LoadTranslation(i));
		}
	}
}

std::shared_ptr<ResText> TSTool::LoadTranslation(const fs::path& path)
{
	inipp::Ini<char> ini;
	std::ifstream is(path);
	ini.parse(is);

	auto res = std::make_shared<ResText>();

	res->file_path = path;

	for (auto& p : ini.sections["translations"]) {
		res->texts.emplace(p.first, p.second);
		res->wtexts.emplace(p.first, utf8_to_wcs(p.second));
	}

	return res;
}

std::string TSTool::FindTranslation(std::string_view key)
{
	for (auto& t : texts) {
		auto it = t->texts.find(key.data());
		if (it != t->texts.end()) {
			return it->second;
		}
	}
	return "";
}

void TSTool::LoadFonts(const fs::path& dir_path)
{
	for (auto i : fs::recursive_directory_iterator{ dir_path }) {
		if (i.is_regular_file() && fs::path(i).extension() == ".png") {
			fonts.push_back(LoadFont(i));
		}
	}
}

std::shared_ptr<ResFont> TSTool::LoadFont(const fs::path& path)
{
	auto filename = path.stem().string();

	auto res = std::make_shared<ResFont>();
	res->font_file_path = path;
	res->prop_file_path = path.parent_path().append(filename + "#properties#.ini");

	LoadFontProperty(res->prop_file_path, res->nodes);

	PngReader::ReadARGB(path, res->img_width, res->img_height, res->img_data);

	return res;
}

void TSTool::LoadFontProperty(const fs::path& path, std::vector<std::shared_ptr<ResFont::Node>>& nodes)
{
	auto prop_buffer = ::ReadText(path);
	auto buffer_size = (std::size_t)prop_buffer.size();
	auto pos = prop_buffer.find("\n", 0);

	std::vector<int> temp;
	while (pos != std::string::npos && pos < buffer_size) {
		auto next_pos = prop_buffer.find("\n", pos + 1);

		temp.push_back(atoi(prop_buffer.substr(pos + 1, next_pos - pos - 1).c_str()));
		if (temp.size() >= 4) {
			auto n = std::make_shared<ResFont::Node>();
			n->ch = ResFont::GetDefaultChar(nodes.size());
			n->h = temp[0];
			n->w = temp[1];
			n->x = temp[2];
			n->y = temp[3];
			nodes.push_back(n);
			temp.clear();
		}

		pos = next_pos;
	}
}

bool TSTool::GenerateKrChars(const fs::path& patch_path, std::string_view language, const fs::path& dst_file_path)
{
	LoadTranslations(fs::path(patch_path).append("translations"), language);

	// kor
	wchar_t scope_min = 0xAC00;
	wchar_t scope_max = 0xD7AF;

	std::set<wchar_t> ansimap;
	std::set<wchar_t> kormap;
	for (auto& res : texts) {
		for (auto& t : res->wtexts) {
			for (auto wch : t.second) {
				if ((wch & 0xFF00) == 0) {
					ansimap.insert(wch);
				}
				else if (wch >= scope_min && wch <= scope_max) {
					kormap.insert(wch);
				}
			}
		}
	}
	for (auto& p : leveltexts_fixed) {
		for (auto& t : p.second.texts) {
			for (auto& wch : t.second) {
				if (wch >= scope_min && wch <= scope_max) {
					kormap.insert(wch);
				}
			}
		}
	}

	std::wstring ansimap_string; ansimap_string.reserve(ansimap.size());
	std::wstring kormap_string; kormap_string.reserve(kormap.size());
	for (auto& p : ansimap) {
		ansimap_string += p;
	}
	for (auto& p : kormap) {
		kormap_string += p;
	}

	std::wofstream wof;
	wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	wof.open(dst_file_path.wstring().c_str());
	wof << ansimap_string;
	wof << kormap_string;
	wof.close();

	if (wof.bad()) {
		return false;
	}

	return true;
}

bool TSTool::GenerateKrLevels(const fs::path& orig_path, const fs::path& patch_path, std::string_view language)
{
	LoadTranslations(fs::path(patch_path).append("translations"), language);
	LoadLevels(fs::path(orig_path).append("#sp#levels_sp"));

	for (auto& lt : leveltexts_linked) {
		auto ltname = lt.first;
		auto it = std::find_if(levels.begin(), levels.end(), [&ltname](auto& elm) { return elm->file_path.stem().string() == ltname; });
		if (it == levels.end())
			continue;
		auto& lines = (*it)->lines;
		for (auto t : lt.second.texts) {
			if (t.first >= lines.size())
				continue;
			auto str = FindTranslation(wcs_to_utf8(t.second));
			if (str.empty())
				continue;
			lines[t.first - 1] = stdext::strformat<256>("Text %s", str.data());
		}
	}
	for (auto& lt : leveltexts_fixed) {
		auto ltname = lt.first;
		auto it = std::find_if(levels.begin(), levels.end(), [&ltname](auto& elm) { return elm->file_path.stem().string() == ltname; });
		if (it == levels.end())
			continue;
		auto& lines = (*it)->lines;
		for (auto t : lt.second.texts) {
			if (t.first >= lines.size())
				continue;
			lines[t.first - 1] = stdext::strformat<256>("Text %s", wcs_to_utf8(t.second).c_str());
		}
	}

	auto dst_dir_path = fs::path(patch_path).append("#sp#levels_sp");
	if (!fs::exists(dst_dir_path)) {
		fs::create_directory(dst_dir_path);
	}
	for (auto& lv : levels) {
		std::ofstream os(fs::path(dst_dir_path).append(lv->file_path.filename().string()));
		for (auto& l : lv->lines) {
			os << l << std::endl;
		}
	}

	return true;
}

bool TSTool::GenerateKrFontImg(const fs::path& orig_path, const fs::path default_bmf_path, const fs::path subtitle_bmf_path, const fs::path& output_path)
{
	LoadFonts(fs::path(orig_path).append("fonts"));

	std::array<BMFont, 2> bmfs;
	for (int i = 0; i < 2; i++) {
		if (!BMFontXmlReader::Read(i == 0 ? default_bmf_path : subtitle_bmf_path, bmfs[i])) {
			return false;
		}
	}

	std::array<std::vector<uint32_t>, 2> bmf_img_widths;
	std::array<std::vector<uint32_t>, 2> bmf_img_heights;
	std::array<std::vector<std::vector<uint8_t>>, 2> bmf_img_buffers;

	for (int i = 0; i < 2; i++) {
		for (auto& p : bmfs[i].pages) {
			bmf_img_widths[i].push_back({});
			bmf_img_heights[i].push_back({});
			bmf_img_buffers[i].push_back({});
			PngReader::ReadARGB(default_bmf_path.parent_path().append(p.file), bmf_img_widths[i].back(), bmf_img_heights[i].back(), bmf_img_buffers[i].back());
		}
	}

	for (auto& res_font : fonts) {
		auto bmf_idx = res_font->font_file_path.stem().string() == "GeosansLight" ? 0 : 1;
		auto& bmf = bmfs[bmf_idx];

		std::vector<BMFont::Char*> ansi_chars;
		std::vector<BMFont::Char*> kr_chars;
		for (auto& ch : bmf.chars) {
			if (ch.id <= UINT8_MAX) {
				ansi_chars.push_back(&ch);
			}
			else {
				kr_chars.push_back(&ch);
			}
		}

		auto kr_char_total = (int)kr_chars.size();
		auto min_yoffset = bmf.GetCharMinYoffset();
		auto ch_max_width = bmf.GetCharMaxWidthForRender();
		auto ch_max_height = bmf.common.lineHeight;

		{
			uint32_t org_max_width, org_max_height;
			res_font->GetMaxWidthHeight(org_max_width, org_max_height);
			ch_max_height = org_max_height;
		}

		auto wcount = res_font->img_width / ch_max_width;
		auto hcount = kr_char_total / wcount + ((kr_char_total % wcount) > 0 ? 1 : 0);

		auto new_img_height = res_font->img_height + (hcount * ch_max_height);
		new_img_height = ((new_img_height / 1024) + ((new_img_height % 1024) > 0 ? 1 : 0)) * 1024;
		auto new_img_size = res_font->img_width * new_img_height * 4;

		std::vector<uint8_t> dst_buf(new_img_size, 0);
		if (bmf_idx == 1) {
			for (auto i = 0; i < new_img_size; i += 4) {
				dst_buf[i + 3] = 0xFF;
			}
		}

		memcpy(dst_buf.data(), res_font->img_data.data(), res_font->img_data.size());

		auto property = std::to_string(ch_max_height) + res_font->GeneratePropertyString(false);

		property.reserve(property.size() + kr_char_total * 4 * 5);

		for (int i = 0; i < kr_char_total; i++) {
			auto bmf_ch = kr_chars[i];
			auto xoffset_mod = std::max(bmf_ch->xoffset + (int)bmf.info.padding[3], 0);
			auto yoffset_mod = std::max(bmf_ch->yoffset - min_yoffset, 0);
			auto ch_wsize = std::max(xoffset_mod + bmf_ch->width, bmf_ch->xadvance + bmf.info.padding[1] + bmf.info.padding[3]);
			auto ch_hsize = yoffset_mod + bmf_ch->height;

			auto& src_buf = bmf_img_buffers[bmf_idx][bmf_ch->page];

			auto dst_xoffset = (i % wcount) * ch_max_width;
			auto dst_yoffset = res_font->img_height + ((i / wcount) * ch_max_height);

			for (int y = 0; y < bmf_ch->height; y++) {
				for (int x = 0; x < bmf_ch->width; x++) {
					auto src_xoffset = bmf_ch->x + x;
					auto src_yoffset = bmf_ch->y + y;

					auto dst_pos = (dst_yoffset + yoffset_mod + y) * res_font->img_width + dst_xoffset + xoffset_mod + x;
					auto src_pos = src_yoffset * bmf_img_widths[bmf_idx][bmf_ch->page] + src_xoffset;

					for (int c = 0; c < 4; c++)
						dst_buf[dst_pos * 4 + c] = src_buf[src_pos * 4 + c];
				}
			}

			property += stdext::strformat<256>("\r%d\r%d\r%d\r%d", 111, std::min(ch_wsize + 10, ch_max_width), dst_xoffset, dst_yoffset);
		}

		PngWriter::WriteARGB(dst_buf, res_font->img_width, new_img_height, fs::path(output_path).append(res_font->font_file_path.filename().string()));
		std::ofstream(fs::path(output_path).append(res_font->prop_file_path.filename().string())).write(property.c_str(), property.size());
	}

	return true;
}