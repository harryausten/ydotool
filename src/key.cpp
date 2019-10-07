/*
    This file is part of ydotool.
	Copyright (C) 2019 Harry Austen
    Copyright (C) 2018-2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

// Local Includes
#include "key.hpp"
// C++ system Includes
#include <sstream>
// External libs
#include <evdevPlus/evdevPlus.hpp>
#include <boost/program_options.hpp>

void ydotool::key_help() {
	std::cerr << "Usage: key [--delay <ms>] [--key-delay <ms>] [--repeat <times>] [--repeat-delay <ms>] <key sequence> ...\n"
		<< "  --help                Show this help.\n"
		<< "  --delay ms            Delay time before start pressing keys. Default 100ms.\n"
		<< "  --key-delay ms        Delay time between keystrokes. Default 12ms.\n"
		<< "  --repeat times        Times to repeat the key sequence.\n"
		<< "  --repeat-delay ms     Delay time between repetitions. Default 0ms.\n"
		<< "\n"
		<< "Each key sequence can be any number of modifiers and keys, separated by plus (+)\n"
		<< "For example: alt+r Alt+F4 CTRL+alt+f3 aLT+1+2+3 ctrl+Backspace \n"
		<< "\n"
		<< "Since we are emulating keyboard input, combination like Shift+# is invalid.\n"
		<< "Because typing a `#' involves pressing Shift and 3." << std::endl;
}

std::vector<std::string> ExplodeString(const std::string & str, char delim) {
	std::vector<std::string> result;
	std::istringstream iss(str);

	for (std::string token; std::getline(iss, token, delim); )
		result.push_back(std::move(token));

	return result;
}

static std::vector<std::string> SplitKeys(const std::string & keys) {
	if (!strchr(keys.c_str(), '+')) {
		return {keys};
	}

	return ExplodeString(keys, '+');
}

static std::vector<int> KeyStroke2Code(const std::string &ks) {
	auto list_keystrokes = SplitKeys(ks);

	std::vector<int> list_keycodes;

	for (auto & it : list_keystrokes) {
		for (auto & itc : it) {
			if (islower(itc))
				itc = toupper(itc);
		}

		auto t_kms = evdevPlus::Table_ModifierKeys.find(it);
		if (t_kms != evdevPlus::Table_ModifierKeys.end()) {
			list_keycodes.push_back(t_kms->second);
			continue;
		}

		auto t_ks = evdevPlus::Table_FunctionKeys.find(it);
		if (t_ks != evdevPlus::Table_FunctionKeys.end()) {
			list_keycodes.push_back(t_ks->second);
		} else {
			auto t_kts = evdevPlus::Table_LowerKeys.find(tolower(it[0]));

			if (t_kts != evdevPlus::Table_LowerKeys.end()) {
				list_keycodes.push_back(t_kts->second);
			} else {
				throw std::invalid_argument("no matching keycode");
			}
		}
	}

	return list_keycodes;
}

int ydotool::key_emit_codes(long key_delay, const std::vector<std::vector<int>> & list_keycodes, const uInputPlus::uInput * uInputContext) {
	auto sleep_time = (uint)(key_delay * 1000 / (list_keycodes.size() * 2));

	for (auto & it : list_keycodes) {
		for (auto & it_m : it) {
			uInputContext->SendKey(it_m, 1);
			usleep(sleep_time);
		}

		for (auto i = it.size(); i-- > 0;) {
			uInputContext->SendKey(it[i], 0);
			usleep(sleep_time);
		}
	}

	return 0;
}

int ydotool::key_run(int argc, const char ** argv, const uInputPlus::uInput * uInputContext) {
	int time_delay = 100;
	int time_keydelay = 12;
	int time_repdelay = 0;

	int repeats = 1;


	std::string file_path;
	std::vector<std::string> extra_args;

	try {

        boost::program_options::options_description desc("");
		desc.add_options()
			("help", "Show this help")
			("delay", boost::program_options::value<int>())
			("key-delay", boost::program_options::value<int>())
			("repeat", boost::program_options::value<int>())
			("repeat-delay", boost::program_options::value<int>())
			("extra-args", boost::program_options::value(&extra_args));


        boost::program_options::positional_options_description p;
		p.add("extra-args", -1);


        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
			options(desc).
			positional(p).
			run(), vm);
        boost::program_options::notify(vm);


		if (vm.count("help")) {
			key_help();
			return -1;
		}

		if (vm.count("delay")) {
			time_delay = vm["delay"].as<int>();
			std::cerr << "Delay was set to "
				  << time_delay << " milliseconds.\n";
		}

		if (vm.count("key-delay")) {
			time_keydelay = vm["key-delay"].as<int>();
			std::cerr << "Key delay was set to "
				  << time_keydelay << " milliseconds.\n";
		}

		if (vm.count("repeat")) {
			repeats = vm["repeat"].as<int>();
			std::cerr << "Repeat was set to "
				  << repeats << " times.\n";
		}

		if (vm.count("repeat-delay")) {
			time_repdelay = vm["repeat-delay"].as<int>();
			std::cerr << "Repeat delay was set to "
				  << time_repdelay << " milliseconds.\n";
		}


		if (extra_args.empty()) {
			std::cerr << "Which keys do you want to press?\n"
				     "Use `ydotool key --help' for help.\n";

			return 1;
		}

	} catch (std::exception & e) {
		std::cerr << "ydotool: key: error: " << e.what() << std::endl;;
		return 2;
	}

	if (time_delay)
		usleep(time_delay * 1000);

	std::vector<std::vector<int>> keycodes;

	for (auto &ks : extra_args) {
		auto thiskc = KeyStroke2Code(ks);

		keycodes.emplace_back(thiskc);
	}

	while (repeats--) {
		key_emit_codes(time_delay, keycodes, uInputContext);
	}
	
	return argc;
}