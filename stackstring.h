#pragma once

class stackstring {
	char c;
	stackstring *parent;

	std::string output(size_t size) {
		if (parent) { return parent->output(size + 1) += c; }
		std::string out;
		out.reserve(size);
		return out += c;
	}

	public:
	stackstring(char _c, stackstring *_parent = nullptr) :
		c(_c), parent(_parent)
	{}
	std::string output() {
		return output(1);
	}
};
