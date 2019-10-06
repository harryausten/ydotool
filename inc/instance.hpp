/*
    This file is part of ydotool.
    Copyright (C) 2018-2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef YDOTOOL_LIB_INSTANCE_HPP
#define YDOTOOL_LIB_INSTANCE_HPP

#include "CommonIncludes.hpp"

namespace ydotool {
	class Instance {
	public:
		std::unique_ptr<uInputPlus::uInput> uInputContext;

		Instance() = default;

		void Init();
		void Init(const std::string& device_name);
	};
}
#endif //YDOTOOL_LIB_INSTANCE_HPP
