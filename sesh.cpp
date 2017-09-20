#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <string>
#include <vector>
#include <iostream>

#include "histrie.h"
#include "cli.cpp"

namespace state {
	bool in_child = false;
}

struct command {
	std::vector<std::string> arguments;

	struct parser {
		command &out;

		std::string *current_arg;

		std::string &get_arg() {
			if (current_arg) { return *current_arg; }
			out.arguments.emplace_back();
			return *(current_arg = &out.arguments.back());
		}

		void clear_arg() { current_arg = nullptr; }


		parser(command &_out) : out(_out), current_arg(nullptr) {}

		bool parse(const char *s) {
			while (const char &c = *s++) {
				switch (c) {
				case ' ':
					clear_arg();
					break;
				default:
					get_arg() += c;
				}
			}
			return true;
		}
	};

	command(const char *s) {
		parser(*this).parse(s);
	}

	int execute() {
		if (!arguments.size()) {
			return -1;
		}
		if (!fork()) {
			auto args = new const char *[arguments.size()];
			for (size_t i = 0; i < arguments.size(); ++i) {
				args[i] = arguments[i].c_str();
			}
			args[arguments.size()] = nullptr;

			if (execvp(arguments[0].c_str(), const_cast<char **>(args)) == -1) {
				printf("%s\n", strerror(errno));
			}
			_exit(1);
		}
		int exit_status;

		state::in_child = true;
		wait(&exit_status);
		state::in_child = false;

		return exit_status;
	}
};

sigjmp_buf sigint_buf;

void handle_sigint(int) {
	if (state::in_child) return;
	siglongjmp(sigint_buf, 1);
}

int main() {

	signal(SIGINT, handle_sigint);

	cli cli;
	cli.prompt = ": ";

	std::string histfile = getenv("HOME");
	histfile += "/.sesh_history";
	cli.read_history(histfile.c_str());

	for (;;) {
		std::string input;

		if (sigsetjmp(sigint_buf, 1)) {
			putchar('\n');
			continue;
		}

		try {
			input = cli.readline();
		} catch (cli::EOFException) { break; }

		if (input[0] != '\0') {
			cli.history.insert(input.c_str());
			int status = command(input.c_str()).execute();
			if (status != 0) {
				printf("< %d\n", status);
			}
		}
	}

	putchar('\n');
	// write_history(histfile.c_str());
}
