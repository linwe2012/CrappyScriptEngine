#include "console.h"
#include "macro-utils.h"
#include <iostream>

namespace Console {
	//TODO: Make these memeber ofOStreamProxy
	static std::string top_;
	static int line_cnt_;
	static FILE *error_output_ = nullptr;
	static int global_current_color_ = Console::kDefaultColor;
	_internal::OStreamProxy out(std::cout);
	void TODO_FUNCTION(){
		TODO_IGNORE(line_cnt_);
	}
#define DEF_COLOR(c, win, nix) _internal::ConsoleColorType c(k##c);
		COLOR_LIST(DEF_COLOR)
#undef DEF_COLOR
		
	void SetTop(std::string top) { top_ = top; }
	int PrintF(const char *fmt, ...) {
		int result;
		va_list args;
		va_start(args, fmt);
		result = vprintf(fmt, args);
		va_end(args);
		return result;
	}


	int PrintError(const char *fmt, ...) {
		int result;
		va_list args;
		va_start(args, fmt);
		if (error_output_ == nullptr)
			result = vfprintf(stderr, fmt, args);
		else
			result = vfprintf(error_output_, fmt, args);
		va_end(args);
		return result;
	}
	void SetColor(int color, int *_where) {
		if (_where == nullptr)
			_where = &global_current_color_;
		if (color < 0) {
			*_where = kDefaultColor;
		}
		else if ((color & kDark) >= kColorGard)
			*_where = kDefaultColor;
		else
			*_where = color;
	}

#if defined(WIN32) + defined(_WIN32) > 0
#define WC_WIN32
	// windows has too many typedef & macros 
	// that may cause naming collisions
	namespace win {
#include <windows.h>
	}
#include <conio.h>
	int Console::SetUpConsole() {
		win::SetConsoleOutputCP(CP_UTF8);
		return 0;
	}

	void Console::ClearScreen() {
		using namespace win;
		COORD topLeft = { 0, 0 };
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;
		GetConsoleScreenBufferInfo(console, &screen);
		FillConsoleOutputCharacterA(
			console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
		);
		FillConsoleOutputAttribute(
			console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			screen.dwSize.X * screen.dwSize.Y, topLeft, &written
		);
		SetConsoleCursorPosition(console, topLeft);
	}
	namespace _internal {
		void SetConsoleColorRefresh(std::ostream& os, int color) {
			os << std::flush;
			using namespace win;
			static int LastColor = kDefaultColor;
			// if (global_current_color_ != LastColor) {
				HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);             
				if (hStdout == INVALID_HANDLE_VALUE)                          
				{ /*TODO: log failure*/                                       
					return;                                                   
				}                                                             
				switch (color & kDark)
				{
#define WIN_COLOR(c, win, nix)                                                             \
				case k##c:                                                                 \
					if(color & ~kDark)                                                     \
					/*sets the color to intense red on blue background*/                   \
						SetConsoleTextAttribute(hStdout, win | FOREGROUND_INTENSITY );     \
					else                                                                   \
						SetConsoleTextAttribute(hStdout, win);                             \
					break;
					COLOR_LIST(WIN_COLOR);
#undef WIN_COLOR
				default:
					break;
				}
			// }
		}
		void SetConsoleColorRefreshEnd(std::ostream& os, int color) {
			os << std::flush;
		}
	}
	
#else

	// #include <curses.h>
int SetUpConsole() {
	//TODO
	return 0;
}

void ClearScreen() {
	printf("\\033[2J");
}
namespace _internal {
	void SetConsoleColorRefresh(std::ostream &os, int color) {
		std::ios::sync_with_stdio(false);
		switch (color) {
#define NIX_COLOR(c, win, escape) \
                case k##c:    \
                    os << "\033[0;40;" #escape "m";                        \
                    break;
			COLOR_LIST(NIX_COLOR);
#undef NIX_COLOR
			default:
				break;
		}

	}

	void SetConsoleColorRefreshEnd(std::ostream &os, UNUSED int color) {
		IGNORE(color);
		os << "\033[0m" << std::flush;
		std::ios::sync_with_stdio(true);
	}
}
#endif
	namespace _internal {
		OStreamProxy& operator<<(OStreamProxy& os, ConsoleColorType& rhs) {
			SetColor(rhs.color, &os.current_color_);
			return os;
		}
		
	}
}
