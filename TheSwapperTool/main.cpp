#include "TSTool.h"

int RunAndWaitprocess(const std::string_view& dir_path, const std::string_view& file_name, const std::span<const std::string_view>& args)
{
	STARTUPINFO si{};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi{};
	std::string cmdline = file_name.data();
	for (auto& a : args) {
		cmdline += std::string(" ") + a.data();
	}
	std::vector<char> cmdline_buf;
	cmdline_buf.resize(cmdline.size() + 1);
	memcpy(cmdline_buf.data(), cmdline.data(), cmdline.size());
	if (!CreateProcess(fs::path(dir_path).append(file_name).string().c_str(), cmdline_buf.data(), NULL, NULL, FALSE, 0, NULL, dir_path.data(), &si, &pi))
	{
		return -1;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code = 0;
	if (!GetExitCodeProcess(pi.hProcess, &exit_code))
	{
		return -1;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return (int)exit_code;
}

int main(int argc, const char** args)
{
	if (argc < 3) {
		printf("invalid arguments");
		return -1;
	}

	std::setlocale(LC_CTYPE, "kor");

	int result = -1;

	fs::path orig_path = args[1];
	fs::path patch_path = args[2];

	TSTool().GenerateKrChars(patch_path, ".en", fs::path(patch_path).append("krchars.txt"));

	for (auto& t : { "default","subtitle" }) {
		auto arg1 = stdext::strformat<128>("-c %s.bmfc", t);
		auto arg2 = stdext::strformat<128>("-o %s", t);
		auto arg3 = R"(-t krchars.txt)";
		RunAndWaitprocess(patch_path.string(), R"(bmfont64.exe)",std::array<std::string_view, 3>{ arg1, arg2, arg3 });
	}

	TSTool().GenerateKrLevels(orig_path, patch_path, ".en");

	TSTool().GenerateKrFontImg(orig_path,
		fs::path(patch_path).append("default.fnt"),
		fs::path(patch_path).append("subtitle.fnt"),
		fs::path(patch_path).append("fonts"));

	return result;
}