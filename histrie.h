#pragma once

#include <map>
#include <unordered_map>
#include "stackstring.h"

class histrie {
	typedef unsigned int weight_t;

	struct weight_key {
		weight_t weight;
		char ch;

	};

	friend bool operator<(const weight_key &a, const weight_key &b) {
		return a.weight == b.weight
			? a.ch < b.ch
			: a.weight < b.weight
		;
	}

	std::unordered_map<char, histrie> map;
	// Making this a std::map<weight_key, histrie &> gives me a segfault on the
	// fifth insertion. I haven't tracked down the reason yet but using a
	// pointer sorts it.
	std::map<weight_key, histrie *> weight_map;
	weight_t weight;
	weight_t rweight;

	void completions(
		const char *s, stackstring *ss_in,
		std::multimap<weight_t, std::string> &out, size_t lim = 0
	) const {
		if (lim && out.size() == lim) { return; }
		if (!s) {
			if (weight) { out.emplace(weight, ss_in->output()); }
			for (auto it = weight_map.rbegin(); it != weight_map.rend(); ++it) {
				stackstring ss{it->first.ch, ss_in};
				it->second->completions(s, &ss, out, lim);
				if (lim && out.size() == lim) { return; }
			}
		} else {
			auto it = *s ? map.find(*s) : map.begin();
			if (it != map.end()) {
				stackstring ss{it->first, ss_in};
				it->second.completions(*s ? s + 1 : nullptr, &ss, out, lim);
			}
		}
	}

	public:
	histrie() : weight(0), rweight(0) {};

	void insert(const char *s) {

		rweight++;

		if (*s == '\0') {
			weight++;
			return;
		}

		auto insert_res = map.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(*s),
			std::forward_as_tuple()
		);
		auto &new_entry = insert_res.first->second;

		if (!insert_res.second) {
			weight_map.erase(weight_key{new_entry.rweight, *s});
		}

		new_entry.insert(s + 1);

		weight_map.emplace(weight_key{new_entry.rweight, *s}, &new_entry);
	}

	bool contains(const char *s) {
		if (*s == '\0') {
			return weight != 0;
		}
		auto it = map.find(s[0]);
		if (it == map.end()) { return false; }
		return it->second.contains(s + 1);
	}

	std::vector<std::string> completions(const char *s, size_t lim = 0) {
		std::multimap<weight_t, std::string> cands;

		completions(s, nullptr, cands, lim);

		std::vector<std::string> out;
		out.reserve(cands.size());
		for (const auto cand : cands) { out.push_back(cand.second); }
		return out;
	}
};
