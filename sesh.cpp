#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

sigjmp_buf sigint_buf;

void handle_sigint(int) {
	siglongjmp(sigint_buf, 1);
}

void execute(const char *command) {
	if (!fork()) {
		execlp(command, command);
		_exit(1);
	}
	wait(nullptr);
}

int main() {

	signal(SIGINT, handle_sigint);

	for (;;) {
		const char *input;

		if (sigsetjmp(sigint_buf, 1)) {
			putchar('\n');
			continue;
		}

		if (!(input = readline("$ "))) break;

		if (*input != '\0') {
			execute(input);
		}
	}

	putchar('\n');
}
