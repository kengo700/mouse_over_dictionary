#pragma once

//#include <map>
#include <regex>

//using namespace std;

class Singularizer
{
public:
	std::string singularize(std::string plural) {
		std::string singular = plural;
		for (auto singular_rule : singular_rules) {
			if (std::regex_match(plural, std::regex(singular_rule[0]))) {
				singular = std::regex_replace(plural, std::regex(singular_rule[0]), singular_rule[1]);
				break;
			}
		}
		return singular;
	}

	bool singularize(std::string plural, std::string& singular) {
		for (auto singular_rule : singular_rules) {
			if (std::regex_match(plural, std::regex(singular_rule[0]))) {
				singular = std::regex_replace(plural, std::regex(singular_rule[0]), singular_rule[1]);
				return true;
			}
		}
		return false;
	}

private:
	std::vector<std::vector<std::string>> singular_rules =
	{
		// https://gist.github.com/mrenouf/805745
		{"(.*)people$", "$1person"},
		{"oxen$", "ox"},
		{"children$", "child"},
		{"feet$", "foot"},
		{"teeth$", "tooth"},
		{"geese$", "goose"},
		{"(.*)ives?$", "$1ife"},
		{"(.*)ves?$", "$1f"},
		{"(.*)men$", "$1man"},
		{"(.+[aeiou])ys$", "$1y"},
		{"(.+[^aeiou])ies$", "$1y"},
		{"(.+)zes$", "$1"},
		{"([m|l])ice$", "$1ouse"},
		{"matrices$", "matrix"},
		{"indices$", "index"},
		{"(.+[^aeiou])ices$", "$1ice"},
		{"(.*)ices$", "$1ex"},
		{"(octop|vir)i$", "$1us"},
		{"(.+(s|x|sh|ch))es$", "$1"},
		{"(.+)s$", "$1"},
	};
};

