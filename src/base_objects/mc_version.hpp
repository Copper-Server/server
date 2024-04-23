#pragma once
#include <stdint.h>

namespace crafted_craft {
    struct McVersion {
		enum class Edition:uint16_t {
			custom_0,
			custom_1,
			custom_2,
			custom_3,
			custom_4,
			custom_5,
			custom_6
		};
		operator uint16_t () const { // used for comparing (==,<=, e.t.c....)
			union ya {
				uint16_t result;
				McVersion res;
				ya(){}
			} proxy;
			proxy.res.Major= Major;
			proxy.res.Minor= Minor;
			proxy.res.Path = Path;
			return proxy.result;
		}
		McVersion(uint8_t major = 0, uint8_t minor = 0, uint8_t path = 0,Edition variant= Edition::custom_0) {
			Major = major;
			Minor = minor;
			Path = path;
			Variant = variant;
		}
		uint16_t Major : 2;
		uint16_t Minor : 7;
		uint16_t Path : 4;
		Edition Variant : 3;
	};
}