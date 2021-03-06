#pragma once

#pragma warning( disable : 4101)

#ifdef _DEBUG
#define M_IS_DEBUG true
#else
#define M_IS_DEBUG false
#endif

#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include <array>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <span>
#include <string>
#include <algorithm>
#include <memory>
#include <clocale>
#include <future>
#include <atomic>
#include <locale>
#include <codecvt>

namespace stdext
{
	template<typename T, typename F>
	inline T* FindPtr(const std::span<std::shared_ptr<T>>& container, F func)
	{
		auto it = std::find_if(container.begin(), container.end(), func);
		if (it == container.end()) {
			return nullptr;
		}

		return &(*(*it));
	}
	template<typename T, typename F>
	inline T* FindPtr(const std::span<T>& container, F func)
	{
		auto it = std::find_if(container.begin(), container.end(), func);
		if (it == container.end()) {
			return nullptr;
		}

		return &(*it);
	}
}


#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
# endif
# ifndef VC_EXTRALEAN
#   define VC_EXTRALEAN
# endif
# ifndef NOMINMAX
#   define NOMINMAX
# endif
#include <windows.h>

class MemReader {
public:
	const char*& pos;
	MemReader(const char*& _pos) : pos(_pos) {}

	template<typename T>
	T Read()
	{
		T out;
		out = *((T*)pos);
		pos += sizeof(T);
		return out;
	}

	template<typename T>
	void Read(T& out)
	{
		out = *((T*)pos);
		pos += sizeof(T);
	}

	template<typename T>
	void ReadArray(std::size_t len, T* out)
	{
		memcpy(out, pos, sizeof(T) * len);
		pos += sizeof(T) * len;
	}
	template<typename T, int N>
	void ReadArray(std::array<T, N>& out)
	{
		ReadArray(N, &out[0]);
	}
	template<typename T, int N>
	std::array<T, N> ReadArray()
	{
		std::array<T, N> out;
		ReadArray(N, &out[0]);
		return out;
	}
	template<typename T>
	void ReadVector(std::vector<T>& out)
	{
		ReadArray(out.size(), &out[0]);
	}
	template<typename T>
	std::vector<T> ReadVector(std::size_t size)
	{
		std::vector<T> out;
		out.resize(size);
		ReadArray(size, &out[0]);
		return out;
	}
};

class MemWriter {
public:
	char*& pos;
	MemWriter(char*& _pos) : pos(_pos) {}

	template<typename T>
	void Write(const T& v)
	{
		*((T*)pos) = v;
		pos += sizeof(T);
	}
	template<typename T>
	void WriteArray(T* v, std::size_t len)
	{
		memcpy(pos, v, sizeof(T) * len);
		pos += sizeof(T) * len;
	}
	template<typename T, int N>
	void WriteArray(const std::array<T, N>& v)
	{
		WriteArray(&v[0], N);
	}
	template<typename T>
	void WriteArray(const std::vector<T>& v)
	{
		WriteArray(&v[0], v.size());
	}
};

inline void ReadFile(const fs::path& path, void* buffer)
{
	std::ifstream ifs(path,std::ios::binary);
	std::streambuf* pbuf = ifs.rdbuf();
	pbuf->pubseekoff(0, ifs.beg);
	pbuf->sgetn((char*)buffer, fs::file_size(path));
}

template <typename FUNC>
inline void RecursiveDirWork(const fs::path path, FUNC f)
{
	for (auto i : fs::recursive_directory_iterator{ path }) {
		if (i.is_directory()) {
			RecursiveDirWork(i,f);
		}
		else if (i.is_regular_file()) {
			f(i);
		}
	}
}

class uintvar {
public:
	uintvar(uint64_t value) {
		int highest_bit = 0;
		std::bitset<sizeof(uint64_t) * 8> bs(value);
		for (int i = sizeof(uint64_t) * 8 - 1; i >= 0; i--) {
			if (bs[i]) {
				highest_bit = i;
				break;
			}
		}
		int highest_byte = highest_bit / 7 + 1;
		bytes.resize(highest_byte);
		for (int i = 0; i < highest_byte; i++) {
			auto v = (value >> (highest_byte - 1 - i) * 7) & 0x7F;
			bytes[i] = (uint8_t)(v | 0x80);
		}
		bytes.back() &= 0x7F;
	}
	std::vector<uint8_t> bytes;
};

template<typename T>
inline std::string ValueToHexString(T v, bool include_header = true) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << v;
	return ((include_header) ? "0x" : "") + std::string(stream.str());
}
template<typename T>
inline std::wstring ValueToHexWString(T v, bool include_header = true) {
	std::wstringstream stream;
	stream << std::setfill(L'0') << std::setw(sizeof(T)*2)  << std::hex << v;
	return ((include_header) ? L"0x" : L"") + std::wstring(stream.str());
}

std::string BytesToHexString(const std::span<uint8_t>& arr) {
	static const char* digits = "0123456789ABCDEF";
	auto hex_len = arr.size();
	std::string rc(hex_len * 2, '0');
	for (size_t i = 0; i < hex_len; ++i) {
		rc[i * 2 + 0] = digits[arr[i] >> 4 & 0x0f];
		rc[i * 2 + 1] = digits[arr[i] >> 0 & 0x0f];
	}
	return "0x" + rc;
}

std::vector<uint8_t> HexStringToBytes(const std::string& hex) {
	std::vector<uint8_t> bytes;

	std::size_t hex_len = hex.size();
	std::size_t start_idx = 0;
	if (hex_len > 2 && hex.compare(0, 2, "0x") == 0) {
		start_idx = 2;
	}
	for (std::size_t i = start_idx; i < hex_len; i += 2) {
		std::string byteString = hex.substr(i, 2);
		uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}


inline std::wstring utf8_to_wcs(const std::string& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(src);
}
inline std::string wcs_to_utf8(const std::wstring& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(src);
}

inline std::wstring mbs_to_wcs(const std::string& src)
{
	if (src.empty()) {
		return L"";
	}

	std::size_t converted{};
	std::vector<wchar_t> dest(src.size() + 1, L'\0');
	if (::mbstowcs_s(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE) != 0) {
		throw std::system_error{ errno, std::system_category() };
	}
	dest.resize(std::char_traits<wchar_t>::length(dest.data()));
	dest.shrink_to_fit();
	return std::wstring(dest.begin(), dest.end());
}
inline std::wstring mbs_to_wcs(const std::string& src, const std::wstring& err_default)
{
	try {
		return mbs_to_wcs(src);
	}
	catch (const std::exception& e) {
		return err_default;
	}
}

inline std::string wcs_to_mbs(const std::wstring& src)
{
	if (src.empty()) {
		return "";
	}

	std::size_t converted{};
	std::vector<char> dest(src.size() * sizeof(wchar_t) + 1, '\0');
	if (::wcstombs_s(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE) != 0) {
		throw std::system_error{ errno, std::system_category() };
	}
	dest.resize(std::char_traits<char>::length(dest.data()));
	dest.shrink_to_fit();
	return std::string(dest.begin(), dest.end());
}
inline std::string wcs_to_mbs(const std::wstring& src, const std::string& err_default)
{
	try {
		return wcs_to_mbs(src);
	}
	catch (const std::exception& e) {
		return err_default;
	}
}

inline std::string mbs_to_utf8(const std::string& src) {
	try {
		return wcs_to_utf8(mbs_to_wcs(src));
	}
	catch (const std::exception& e) {
		return src;
	}
}

inline std::string utf8_to_mbs(const std::string& src)
{
	try {
		return wcs_to_mbs(utf8_to_wcs(src));
	}
	catch (const std::exception& e) {
		return src;
	}
}

inline bool sjis_valid(const std::string& src)
{
	try {
		mbs_to_wcs(src);
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

#include "../include/rapidxml/rapidxml.hpp"
#include "../include/rapidxml/rapidxml_utils.hpp"
#include "../include/rapidxml/rapidxml_print.hpp"

template<typename T>
inline const char* RapidXmlString(rapidxml::xml_document<char>& doc, const T& v) {
	return doc.allocate_string(std::to_string(v).c_str());
}
template<typename T>
inline const wchar_t* RapidXmlString(rapidxml::xml_document<wchar_t>& doc, const T& v) {
	return doc.allocate_string(std::to_wstring(v).c_str());
}

inline const char* RapidXmlString(rapidxml::xml_document<char>& doc, const std::string& v) {
	return doc.allocate_string(v.c_str());
}
inline const wchar_t* RapidXmlString(rapidxml::xml_document<wchar_t>& doc, const std::wstring& v) {
	return doc.allocate_string(v.c_str());
}

#include "../include/libpng/include/png.h"

#ifdef _DEBUG
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpng16d.lib")
#else
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng16.lib")
#endif


class PngReader {
public:
	static void Read(fs::path file_path, std::vector<uint8_t>& data)
	{
		uint32_t width;
		uint32_t height;
		png_byte color_type;
		png_byte bit_depth;
		return Read(file_path, width, height, color_type, bit_depth, data);
	}

	static void Read(fs::path file_path, uint32_t& width, uint32_t& height, png_byte& color_type, png_byte& bit_depth, std::vector<uint8_t>& data)
	{
		char header[8];

		FILE* fp;
		fopen_s(&fp, file_path.string().c_str(), "rb");
		fread(header, 1, 8, fp);
		auto png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		auto info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));

		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);
		png_read_info(png_ptr, info_ptr);

		width = png_get_image_width(png_ptr, info_ptr);
		height = png_get_image_height(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		auto number_of_passes = png_set_interlace_handling(png_ptr);
		png_read_update_info(png_ptr, info_ptr);

		setjmp(png_jmpbuf(png_ptr));

		auto row_size = png_get_rowbytes(png_ptr, info_ptr);
		data.resize(width * height);

		std::vector<uint8_t> row_bytes;
		row_bytes.resize(row_size);
		for (uint32_t y = 0; y < height; y++) {
			png_read_row(png_ptr, row_bytes.data(), nullptr);
			if (color_type == PNG_COLOR_TYPE_GRAY) {
				for (uint32_t i = 0; i < width; i++) {
					data[y * width + i] = row_bytes[i];
				}
			}
			else if (color_type == PNG_COLOR_TYPE_RGB) {
				for (uint32_t i = 0; i < width * 3; i += 3) {
					data[y * width + i / 3] = row_bytes[i];
				}
			}
		}
		fclose(fp);
	}
};

#include "zlib/zlib.h"