#include <termios.h>
#include <fstream>

struct cli {

    struct EOFException {};

    struct line {
        const static std::unordered_map<char, void (cli::line::*)()> handlers;
        const static std::unordered_map<
            char, void (cli::line::*)()
        > escape_handlers;

        cli &cli;
        std::string buf;
        bool have_autocomplete;
        int pos;

        line(struct cli &_cli) : cli(_cli), have_autocomplete(false), pos(0) {}

        void pbuf() {
            cli.esc("s");
            std::cout << buf.c_str() + pos;
            cli.esc("u");
        }

        bool autocomplete(const std::string &buf) {
            auto completions = cli.history.completions(buf.c_str(), 1);
            if (!completions.size()) { return false; }
            cli.esc("30;1m");
            cli.esc("s");
            std::cout << completions[0].c_str() + buf.size();
            cli.esc("0m");
            cli.esc("u");
            return true;
        }

        void handle_tab() {
        }

        void handle_delete() {
            if (!pos) { return; }
            cli.esc("D");
            cli.esc("K");
            if (pos == buf.size()) {
                buf.resize(buf.size() - 1);
            } else {
                buf.erase(pos - 1, 1);
            }
            pos--;
            if (buf.size()) { pbuf(); }
        }

        void handle_escape() {
            int c = getchar();
            if (c != '[') {
                std::cout.put('\x7');
                return;
            }
            c = getchar();

            auto it = escape_handlers.find(c);
            if (it == escape_handlers.end()) { return; }
            (this->*it->second)();
        }

        void handle_up() {
        }

        void handle_down() {
        }

        void handle_right() {
            if (pos == buf.size()) {
                if (have_autocomplete) {
                }
                return;
            }
            cli.esc("C");
            pos++;
        }

        void handle_left() {
            if (pos == 0) { return; }
            cli.esc("D");
            pos--;
        }

        bool handle_special_key(char c) {

            auto it = handlers.find(c);
            if (it == handlers.end()) { return false; }

            (this->*it->second)();
            return true;
        }

        line &read() {

            std::cout << cli.prompt;

            for (;;) {
                buf.reserve(buf.size() + 1);
                char c = getchar();
                if (have_autocomplete) {
                    cli.esc("K");
                    have_autocomplete = false;
                }
                if (handle_special_key(c)) { continue; }
                if (c == 4 || c == '\n') {
                    if (c == 4 && !buf.size()) { throw EOFException{}; }
                    std::cout << std::endl;
                    break;
                }
                std::cout.put(c);
                buf.insert(pos, 1, c);
                pos++;
                if (pos != buf.size() - 1) { pbuf(); }

                have_autocomplete = autocomplete(buf);
            }

            return *this;
        }
    };

    histrie history;
    std::string prompt;

    void on() {
        struct termios tios{};
        if (tcgetattr(0, &tios) < 0) { abort(); }
        tios.c_lflag &= ~ICANON;
        tios.c_lflag &= ~ECHO;
        tios.c_lflag &= ~ECHOCTL;
        if (tcsetattr(0, TCSANOW, &tios) < 0) { abort(); }
    }

    void esc(const char *code) {
        std::cout << "\033[" << code;
    }

    void read_history(const char *file) {
        std::ifstream in;
        std::string line;
        in.open(file);
        while (std::getline(in, line)) {
            if (line.size()) {
                history.insert(line.c_str());
            }
        }
    }

	std::string readline() {
        return line{*this}.read().buf;
	}

    cli() { on(); }

};

const std::unordered_map<
    char, void (cli::line::*)()
> cli::line::handlers{
    {'\x7f', &cli::line::handle_delete},
    {'\033', &cli::line::handle_escape},
    {'\t', &cli::line::handle_tab},
};

const std::unordered_map<
    char, void (cli::line::*)()
> cli::line::escape_handlers{
    {'A', &cli::line::handle_up},
    {'B', &cli::line::handle_down},
    {'C', &cli::line::handle_right},
    {'D', &cli::line::handle_left},
};

