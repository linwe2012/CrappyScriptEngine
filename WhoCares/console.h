#ifndef _OUTPUT_H_
#define _OUTPUT_H_
#include <stdio.h>
#include <stdarg.h>
#include <string>

namespace Console {
#define COLOR_LIST(V) \
	V(Cyan,  0x3, 36)          \
	V(Blue,  0x1, 34)          \
	V(Green, 0x2, 32)          \
	V(Red,   0x4, 31)          \
	V(Magent,0x5, 35)          \
	V(Yellow,0x6, 33)          \
	V(White, 0x7, 37)          \
	V(Black, 0x0, 37)          \
	
	enum Color
	{
		// any color <= 0 is considered default
#define ENUM_COLOR(c, win, nix) k##c,
		COLOR_LIST(ENUM_COLOR)
#undef ENUM_COLOR
		kColorGard,
		kDark  = ~(1 << 10),
		// kBlack = kGrey | kDark,
		kDefaultColor = kWhite | kDark,
	};
	int SetUpConsole();
	int PrintF(const char *fmt, ...);
	int PrintError(const char *fmt, ...);
	void ClearScreen();
	void SetTop(std::string top);
	void SetColor(int color, int *_where = nullptr);
	namespace _internal {
		struct ConsoleColorType
		{
			int color;
			ConsoleColorType(int c) :color(c) {}
			ConsoleColorType& dark() { color &= kDark; return *this; }
			ConsoleColorType& light() { color |= ~kDark; return *this; }
		};
		class OStreamProxy {
		public:
			OStreamProxy(std::ostream& os) :os_(os) {}
			template<typename T>
			friend OStreamProxy& operator<<(OStreamProxy& os, T rhs);
			friend OStreamProxy& operator<<(OStreamProxy& os, ConsoleColorType& rhs);
			// operator std::ostream& () { return os_; }
			OStreamProxy& operator<<(std::ostream& (*_Pfn)(std::ostream&)) { _Pfn(os_); return *this; }
		private:
			void getConsoleInfo();
			int console_height_;
			int console_witdth_;
			std::ostream& os_;
			int current_color_ = Console::kDefaultColor;
			int tab_;
		};
		void SetConsoleColorRefresh(std::ostream& os, int color);
		void SetConsoleColorRefreshEnd(std::ostream& os, int color);
		template <typename T>
		OStreamProxy& operator<<(OStreamProxy& os, T rhs) {
			SetConsoleColorRefresh(os.os_, os.current_color_);
			os.os_ << rhs;
			SetConsoleColorRefreshEnd(os.os_, os.current_color_);
			return os;
		}
	}
	extern _internal::OStreamProxy out;
#define DECL_COLOR(c, win, nix) \
	extern _internal::ConsoleColorType c;
	COLOR_LIST(DECL_COLOR)
#undef DECL_COLOR
	
}
#endif // _OUTPUT_H_
