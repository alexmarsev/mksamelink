#ifndef UNICODE
#	define UNICODE
#endif
#ifndef _UNICODE
#	define _UNICODE
#endif

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/crc.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using namespace std;

wostream& operator<<(wostream& wos, const vector<wstring>& vec){
	copy(vec.begin(), vec.end(), ostream_iterator<wstring, wchar_t>(wos, L" ")); 
	return wos;
}

char chunk[32768];

int compute_checksum(const fs::path& path) {
	int res;
	fs::ifstream file(path, ios::binary);
	file.exceptions(ifstream::badbit);
	if (file.is_open()) {
		boost::crc_32_type src32;
		while (!file.eof()) {
			file.read(chunk, sizeof(chunk));
			src32.process_bytes(chunk, (size_t)file.gcount());
		}
		file.close();
		res = src32.checksum();
	} else {
		throw exception(string("cannot open ").append(path.string()).c_str());
	}
	return res;
}

fs::path make_relative_path(const fs::path& from, const fs::path& to) {
	fs::path res;
	auto from_it = from.begin();
	auto to_it = to.begin();
	if (*from_it == *to_it) {
		for (; *from_it == *to_it; ++from_it, ++to_it);
		++from_it;
		for (; from_it != from.end(); ++from_it) res /= "..";
		for (; to_it != to.end(); ++to_it) res /= *to_it;
	} else {
		res = to;
	}
	return res;
}

void symlink_same_files(const set<fs::path>& files,
	const set<fs::path>& existing_symlink_targets, bool interactive = false)
{
	// TODO implement interactive mode
	fs::path target;
	for (auto it = files.cbegin(); it != files.cend(); ++it) {
		if (existing_symlink_targets.count(*it)) {
			target = *it;
			break;
		}
	}
	if (target.empty()) target = *files.begin();
	for_each(files.cbegin(), files.cend(), [&target](fs::path path) {
		if (path != target) {
			fs::path relative_path = make_relative_path(path, target);
			wcout << path.wstring() << L" -> " << relative_path.wstring() << endl;
			fs::remove(path);
			fs::create_symlink(relative_path, path);
		}
	});
}

int wmain(int argc, wchar_t* argv[]) {
	bool interactive = false;
	bool show_help = false;
	size_t min_filesize = 4096;
	vector<wstring> input_dirs;

	po::options_description opdp("options");
	opdp.add_options()
		("help,h", po::bool_switch(&show_help), "print usage information")
		("minsize,m", po::value(&min_filesize), "minimum file size")
		//("interactive,i", po::bool_switch(&interactive), "interactive mode")
	;

	po::options_description opd;
	opd.add(opdp);
	opd.add_options()("input", po::wvalue(&input_dirs), "input");
	po::positional_options_description opp;
	opp.add("input", -1);

	po::variables_map vm;
	try {
		po::store(po::wcommand_line_parser(argc, argv).
			options(opd).positional(opp).run(), vm);
	} catch (const exception& e) {
		cout << e.what() << endl;
		return 1;
	}
	vm.notify();

	if (show_help) {
		cout << "mksamelink [options] [dir ...]" << endl << opdp;
		return 0;
	}

	if (input_dirs.empty()) {
		cout << "no directories specified" << endl;
		return 1;
	}

	cout << "interactive: " << (interactive ? "yes" : "no") << endl;
	cout << "min filesize: " << min_filesize << endl;
	wcout << L"directories: " << input_dirs << endl;

	map<pair<int, uintmax_t>, set<fs::path>> found_files;
	set<fs::path> symlink_targets;

	for (auto dirs_it = input_dirs.cbegin(); dirs_it != input_dirs.cend(); ++dirs_it) {
		fs::recursive_directory_iterator it(*dirs_it);
		fs::recursive_directory_iterator end;
		for (; it != end; ++it) {
			try {
				fs::path path = it->path();
				if (fs::is_symlink(path)) {
					path = fs::read_symlink(path);
					path = fs::system_complete(path);
					symlink_targets.insert(path);
				} else if (fs::is_regular_file(path)) {
					path = fs::system_complete(path);
					uintmax_t filesize = fs::file_size(path);
					if (filesize >= min_filesize) {
						found_files[make_pair(compute_checksum(path), filesize)].insert(path);
					}			
				}
			} catch (const exception& e) {
				cout << e.what() << endl;
			}
		}
	}

	uintmax_t profit = 0;

	for (auto it = found_files.cbegin(); it != found_files.cend() ; ++it) {
		auto same_files = it->second;
		try {
			if (same_files.size() > 1) {
				symlink_same_files(same_files, symlink_targets, interactive);
				profit += it->first.second * (same_files.size() - 1);
			}
		} catch (const exception& e) {
			cout << e.what() << endl;
		}
	}

	cout << "You saved " << profit / (1024. * 1024) << " MB" << endl;

	return 0;
}
