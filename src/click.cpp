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

// Local includes
#include "click.hpp"
// External libs
#include <boost/program_options.hpp>

static const char ydotool_tool_name[] = "click";

const char * ydotool::Tools::Click::Name() {
	return ydotool_tool_name;
}

void * ydotool::Tools::Click::construct() {
	return (void *)(new Click());
}

static void ShowHelp(){
	std::cerr << "Usage: click [--delay <ms>] <button>\n"
		<< "  --help                Show this help.\n"
		<< "  --delay ms            Delay time before start clicking. Default 100ms.\n"
		<< "  button                1: left 2: right 3: middle" << std::endl;
}

int ydotool::Tools::Click::Exec(int argc, const char **argv) {
	int time_delay = 100;
	std::vector<std::string> extra_args;

	try {
        boost::program_options::options_description desc("");
		desc.add_options()
			("help", "Show this help")
			("delay", boost::program_options::value<int>())
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
			ShowHelp();
			return -1;
		}

		if (vm.count("delay")) {
			time_delay = vm["delay"].as<int>();
			std::cerr << "Delay was set to "
				  << time_delay << " milliseconds.\n";
		}

		if (extra_args.size() != 1) {
			std::cerr << "Which mouse button do you want to click?\n"
				     "Use `ydotool " << argv[0] << " --help' for help.\n";

			return 1;
		}
	} catch (std::exception &e) {
		std::cerr << "ydotool: click: error: " << e.what() << std::endl;
		return 2;
	}

	if (time_delay)
		usleep(time_delay * 1000);

	int keycode = BTN_LEFT;

	switch (strtol(extra_args[0].c_str(), NULL, 10)) {
		case 2:
			keycode = BTN_RIGHT;
			break;
		case 3:
			keycode = BTN_MIDDLE;
			break;
		default:
			break;
	}

    uInputContext->SendKey(keycode, 1);
    uInputContext->SendKey(keycode, 0);

	return argc;
}

