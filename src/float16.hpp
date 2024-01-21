#pragma once
#include <string>
class float16_t {
	short val:14;
	unsigned short dot_p:2;
public:
	float16_t(float floa = 0) {
		auto tmp = std::to_string(floa);
		auto dot_pos = tmp.find('.');		
		while (tmp[tmp.size() - 1] == '0' || tmp[tmp.size() - 1] == '.') {
			tmp.erase(tmp.end() - 1);
			if (tmp.size() == 0) {
				dot_p = val = 0;
				return;
			}
		}
		if (dot_pos == std::string::npos) {
			val = floa;
			val *= 10;
		}
		else {
			val = std::stoi(tmp.substr(0, dot_pos));
			dot_p = tmp.size() - 1 - dot_pos;
			for (short i = 0; i < dot_p; i++)
				val *= 10;
			val += std::stoi(tmp.substr(dot_pos+1, tmp.size()));
		}
	}
	operator float() const {
		float res = val;
		if(val)
			for (short i = 0; i < dot_p; i++)
				res /= 10;
		return res;
	}
};



