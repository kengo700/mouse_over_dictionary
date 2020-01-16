#pragma once

#include <regex>
#include <vector>

class Inflector
{
	// まとめて原型の候補を生成
public:
	bool getInfinitives(std::string word, std::vector<std::string>& infinitives) {
		bool is_infinitive = false;
		is_infinitive = !processPlural(word, infinitives);
		is_infinitive = !processPastTense(word, infinitives);
		is_infinitive = !processParticiple(word, infinitives);
		is_infinitive = !processPronoun(word, infinitives);
		return is_infinitive;
	}

	// 複数形、三人称単数現在形
public:
	bool processPlural(int rule_set_num, std::string plural, std::string& singular) {
		singular = plural;
		bool is_plural = false;
		if (rule_set_num > singular_rules_set.size()) {
			rule_set_num = singular_rules_set.size();
		}
		for (auto singular_rule : singular_rules_set[rule_set_num]) {
			if (std::regex_match(plural, std::regex(singular_rule[0]))) {
				singular = std::regex_replace(plural, std::regex(singular_rule[0]), singular_rule[1]);
				is_plural = true;
				return is_plural;
			}
		}
		return is_plural;
	}

	bool processPlural(std::string plural, std::vector<std::string>& singular) {
		bool is_plural = false;
		for (auto singular_rules : singular_rules_set) {
			for (auto singular_rule : singular_rules) {
				if (std::regex_match(plural, std::regex(singular_rule[0]))) {
					singular.push_back(std::regex_replace(plural, std::regex(singular_rule[0]), singular_rule[1]));
					return is_plural;
				}
			}
		}
		return is_plural;
	}

private:
	// Reference: https://gist.github.com/mrenouf/805745
	// Author: mrenouf https://gist.github.com/mrenouf
	std::vector<std::vector<std::vector<std::string>>> singular_rules_set =
	{
		{
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
		},{
			{"(.+)s$", "$1"}, // pulses → pulseなど
		},{
			{u8"(.+)'s$", "$1"}, // 所有格、記号の誤認識は多いので、それぞれ処理する
			{u8"(.+)＇s$", "$1"},
			{u8"(.+)ʼs$", "$1"},
			{u8"(.+)’s$", "$1"},
			{u8"(.+)‘s$", "$1"},
		}
	};

	// 過去形（仮）
public:
	bool processPastTense(int rule_set_num, std::string pasttense, std::string& presenttense) {
		presenttense = pasttense;
		bool is_pasttense = false;
		if (rule_set_num > pasttense_rules_set.size()) {
			rule_set_num = pasttense_rules_set.size();
		}
		for (auto pasttense_rule : pasttense_rules_set[rule_set_num]) {
			if (std::regex_match(pasttense, std::regex(pasttense_rule[0]))) {
				presenttense = std::regex_replace(pasttense, std::regex(pasttense_rule[0]), pasttense_rule[1]);
				is_pasttense = true;
				return is_pasttense;
			}
		}
		return is_pasttense;
	}

	bool processPastTense(std::string pasttense, std::vector<std::string>& presenttense) {
		bool is_pasttense = false;
		for (auto pasttense_rules : pasttense_rules_set) {
			for (auto pasttense_rule : pasttense_rules) {
				if (std::regex_match(pasttense, std::regex(pasttense_rule[0]))) {
					presenttense.push_back(std::regex_replace(pasttense, std::regex(pasttense_rule[0]), pasttense_rule[1]));
					is_pasttense = true;
				}
			}
		}
		return is_pasttense;
	}

private:
	std::vector<std::vector<std::vector<std::string>>> pasttense_rules_set =
	{
		{
			{"abode", "abide"},
			{"addrest", "address"},
			{"arose$", "arise"},
			{"arisen$", "arise"},
			{"awoke$", "awake"},
			{"awoken$", "awake"},
			{"was$", "be"},
			{"were$", "be"},
			{"been$", "be"},
			{"bore$", "bear"},
			{"borne$", "bear"},
			{"born$", "bear"},
			{"beaten$", "beat"},
			{"became$", "become"},
			{"become$", "become"},
			{"began$", "begin"},
			{"begun$", "begin"},
			{"bent$", "bend"},
			{"blew$", "blow"},
			{"blown$", "blow"},
			{"broke$", "break"},
			{"broken$", "break"},
			{"brought$", "bring"},
			{"built$", "build"},
			{"bought$", "buy"},
			{"caught$", "catch"},
			{"chose$", "choose"},
			{"chosen$", "choose"},
			{"came$", "come"},
			{"dealt$", "deal"},
			{"dug$", "dig"},
			{"did$", "do"},
			{"done$", "do"},
			{"drew$", "draw"},
			{"drawn$", "draw"},
			{"drank$", "drink"},
			{"drunk$", "drink"},
			{"drove$", "drive"},
			{"driven$", "drive"},
			{"ate$", "eat"},
			{"eaten$", "eat"},
			{"fell$", "fall"},
			{"fallen$", "fall"},
			{"fed$", "feed"},
			{"felt$", "feel"},
			{"fought$", "fight"},
			{"found$", "find"},
			{"flew$", "fly"},
			{"flown$", "fly"},
			{"forgot$", "forget"},
			{"forgotten$", "forget"},
			{"got$", "get"},
			{"gotten$", "get"},
			{"gave$", "give"},
			{"given$", "give"},
			{"went$", "go"},
			{"gone$", "go"},
			{"grew$", "grow"},
			{"grown$", "grow"},
			{"hung$", "hang"},
			{"had$", "have"},
			{"heard$", "hear"},
			{"hid$", "hide"},
			{"hidden$", "hide"},
			{"held$", "hold"},
			{"kept$", "keep"},
			{"knew$", "know"},
			{"known$", "know"},
			{"laid$", "lay"},
			{"led$", "lead"},
			{"left$", "leave"},
			{"lent$", "lend"},
			{"lay$", "lie"},
			{"lain$", "lie"},
			{"lost$", "lose"},
			{"made$", "make"},
			{"met$", "meet"},
			{"mistaken", "mistake"},
			{"mistook", "mistake"},
			{"paid$", "pay"},
			{"rode$", "ride"},
			{"ridden$", "ride"},
			{"rang$", "ring"},
			{"rung$", "ring"},
			{"rose$", "rise"},
			{"risen$", "rise"},
			{"ran$", "run"},
			{"sawn$", "saw"},
			{"said$", "say"},
			{"saw$", "see"},
			{"seen$", "see"},
			{"sought$", "seek"},
			{"sold$", "sell"},
			{"sent$", "send"},
			{"sewed$", "sew"},
			{"sewn$", "sew"},
			{"shook$", "shake"},
			{"shaken$", "shake"},
			{"shot$", "shoot"},
			{"shown$", "show"},
			{"shrank$", "shrink"},
			{"sang$", "sing"},
			{"sung$", "sing"},
			{"sank$", "sink"},
			{"sunk$", "sink"},
			{"slept$", "sleep"},
			{"spoke$", "speak"},
			{"spoken$", "speak"},
			{"spent$", "spend"},
			{"stood$", "stand"},
			{"stole$", "steal"},
			{"stolen$", "steal"},
			{"stuck$", "stick"},
			{"swore$", "swear"},
			{"sworn$", "swear"},
			{"swam$", "swim"},
			{"swum$", "swim"},
			{"took$", "take"},
			{"taken$", "take"},
			{"taught$", "teach"},
			{"tore$", "tear"},
			{"torn$", "tear"},
			{"told$", "tell"},
			{"thought$", "think"},
			{"threw$", "throw"},
			{"thrown$", "throw"},
			{"understood$", "understand"},
			{"undertook$", "undertake" },
			{"undertaken$", "undertake" },
			{"woke$", "wake"},
			{"woken$", "wake"},
			{"wore$", "wear"},
			{"worn$", "wear"},
			{"wove$", "weave"},
			{"woven$", "weave"},
			{"won$", "win"},
			{"withdrew$", "withdraw"},
			{"withdrawn$", "withdraw"},
			{"withheld$", "withhold"},
			{"withstood$", "withstand"},
			{"wrote$", "write"},
			{"written$", "write"},
			{"(.+)lit", "$1alight" },
			{"(.+)slid", "$1slide" },
			{"(.+)slidden", "$1slide" },
			{"(.+)bit$", "$1bite"},
			{"(.+)bitten$", "$1bite"},
			{"(.+)sat$", "$1sit"},
			{"(.+)cked$", "$1c"},
			{"(.+)nned$", "$1n"},
			{"(.+)tted$", "$1t"},
			{"(.+)dded$", "$1d"},
			{"(.+)gged$", "$1g"},
			{"(.+)pped$", "$1p"},
			{"(.+)mmed$", "$1m"},
			{"(.+)bbed$", "$1b"},
			{"(.+)rred$", "$1r"},
			{"(.+)zzed$", "$1z"},
			{"(.+)ied$", "$1y"},
			{"(.+)ed$", "$1"},
		},
		{
			{"(.+)ed$", "$1e"}, // agreed → agreeなど
		}
	};


	// 現在分詞（仮）
public:
	bool processParticiple(int rule_set_num, std::string participle, std::string& infinitive) {
		infinitive = participle;
		bool is_participle = false;
		if (rule_set_num > participle_rules_set.size()) {
			rule_set_num = participle_rules_set.size();
		}
		for (auto participle_rule : participle_rules_set[rule_set_num]) {
			if (std::regex_match(participle, std::regex(participle_rule[0]))) {
				infinitive = std::regex_replace(participle, std::regex(participle_rule[0]), participle_rule[1]);
				is_participle = true;
				return is_participle;
			}
		}
		return is_participle;
	}

	bool processParticiple(std::string participle, std::vector<std::string>& infinitives) {
		bool is_participle = false;
		for (auto participle_rules : participle_rules_set) {
			for (auto participle_rule : participle_rules) {
				if (std::regex_match(participle, std::regex(participle_rule[0]))) {
					infinitives.push_back(std::regex_replace(participle, std::regex(participle_rule[0]), participle_rule[1]));
					is_participle = true;
				}
			}
		}
		return is_participle;
	}

private:
	std::vector<std::vector<std::vector<std::string>>> participle_rules_set =
	{
		{
			{"(.+)cking$", "$1c"}, // picnicking → picnicなど
			{"(.+)nning$", "$1n"},
			{"(.+)tting$", "$1t"},
			{"(.+)dding$", "$1d"},
			{"(.+)pping$", "$1p"},
			{"(.+)mming$", "$1m"},
			{"(.+)bbing$", "$1b"},
			{"(.+)rring$", "$1r"},
			{"(.+)lling$", "$1l"},
			{"(.+)zzing$", "$1z"},
			{"(.+)ying$", "$1ie"}, // dying → dieなど
			{"(.+)ing$", "$1"},
		},
		{
		   {"(.+)ing$", "$1e"}, // coming → comeなど
		},
		{
		   {"(.+)ing$", "$1"}, // replaying → replayなど
		}
	};

	// 代名詞
public:
	bool processPronoun(int rule_set_num, std::string pronoun, std::string& general) {
		general = pronoun;
		bool is_pronoun = false;
		if (rule_set_num > pronoun_rules_set.size()) {
			rule_set_num = pronoun_rules_set.size();
		}
		for (auto pronoun_rule : pronoun_rules_set[rule_set_num]) {
			if (std::regex_match(pronoun, std::regex(pronoun_rule[0]))) {
				general = std::regex_replace(pronoun, std::regex(pronoun_rule[0]), pronoun_rule[1]);
				is_pronoun = true;
				return is_pronoun;
			}
		}
		return is_pronoun;
	}

	bool processPronoun(std::string pronoun, std::vector<std::string>& general) {
		bool is_pronoun = false;
		for (auto pronoun_rules : pronoun_rules_set) {
			for (auto pronoun_rule : pronoun_rules) {
				if (std::regex_match(pronoun, std::regex(pronoun_rule[0]))) {
					general.push_back(std::regex_replace(pronoun, std::regex(pronoun_rule[0]), pronoun_rule[1]));
					is_pronoun = true;
				}
			}
		}
		return is_pronoun;
	}

private:
	// Reference: https://github.com/wtetsu/mouse-dictionary/tree/master/rule
	// Author: wtetsu https://github.com/wtetsu
	std::vector<std::vector<std::vector<std::string>>> pronoun_rules_set =
	{
	  {
		{"my", "one's"},
		{"your", "one's"},
		{"his", "one's"},
		{"her", "one's"},
		{"its", "one's"},
		{"our", "one's"},
		{"their", "one's"},
		{"'s", "one's"},
		{"one's", "one's"},
		{"someone's", "someone's"},
		{"myself", "oneself"},
		{"yourself", "oneself"},
		{"himself", "oneself"},
		{"herself", "oneself"},
		{"ourselves", "oneself"},
		{"themselves", "oneself"},
		{"him", "someone"},
		{"them", "someone"},
		{"us", "someone"}
	  },
	  {
		{"my", "someone's"},
		{"your", "someone's"},
		{"his", "someone's"},
		{"her", "someone's"},
		{"its", "someone's"},
		{"our", "someone's"},
		{"their", "someone's"},
		{"'s", "someone's"},
		{"one's", "one's"},
		{"someone's", "someone's"},
		{"myself", "oneself"},
		{"yourself", "oneself"},
		{"himself", "oneself"},
		{"herself", "oneself"},
		{"ourselves", "oneself"},
		{"themselves", "oneself"},
		{"him", "someone"},
		{"them", "someone"},
		{"us", "someone"}
	  },
	  {
		{"my", "someone's"},
		{"your", "someone's"},
		{"his", "someone's"},
		{"her", "someone"},
		{"its", "someone's"},
		{"our", "someone's"},
		{"their", "someone's"},
		{"'s", "someone's"},
		{"one's", "one's"},
		{"someone's", "someone's"},
		{"myself", "oneself"},
		{"yourself", "oneself"},
		{"himself", "oneself"},
		{"herself", "oneself"},
		{"ourselves", "oneself"},
		{"themselves", "oneself"},
		{"him", "someone"},
		{"them", "someone"},
		{"us", "someone"}
	  }
	};
};

