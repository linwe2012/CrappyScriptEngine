#include "compile-debugger.h"
#include "macro-utils.h"
#include <fstream>
#include <chrono>
namespace CompileDebugger {
	std::string log_path = "./compile.log";
	std::fstream log_file;
	int error_cnt;
	int warning_cnt;
	bool canLog() {
		return log_file.good() && log_file.is_open();
	}
	const char *getFormattedTime() {
		static char buf[100] = { 0 };
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		// http://www.cplusplus.com/reference/ctime/strftime/
		std::strftime(buf, sizeof(buf), "%c %Z", std::localtime(&now));
		return buf;
	}

	void __Logger(ExprAST *error_point, std::string& initiator, ErrorType what_error, std::string extra_info);
	void logError(ExprAST *error_point, std::string initiator, ErrorType what_error, std::string extra_info) {
		++error_cnt;
		if (!canLog()) return;
		log_file << "[Error] ";
		__Logger(error_point, initiator, what_error, extra_info);
		log_file << std::endl;
	}
	void logWarning(ExprAST *error_point, std::string initiator, ErrorType what_error, std::string extra_info) {
		++warning_cnt;
		if (!canLog()) return;
		log_file << "[Warning] ";
		__Logger(error_point, initiator, what_error, extra_info);
		log_file << std::endl;
	}

	

	void initLogging() {
		if (!canLog()) {
			log_file.open(log_path, std::ios::app);
		}
		if (canLog()) {
			log_file << getFormattedTime() << ": Compile started" << std::endl;
		}
		error_cnt = warning_cnt = 0;
	}
	
	void endLogging() {
		if (!canLog()) return;
		log_file << getFormattedTime() << ": Compile finished." << error_cnt << "errors found and " << warning_cnt << "warnings found." << std::endl;
		log_file.close();
	}
	
	void __Logger(ExprAST *error_point, UNUSED std::string& initiator, ErrorType what_error, std::string extra_info) {

		auto dbg = error_point->dbg();
		log_file << "Line(" << dbg.line() << ")";
		// to lazy to write actual log, it's toooo time consuming
#define  LAZY_LOG(e)                       \
		if(what_error & E_##e)             \
		     log_file << " " << #e << "," << extra_info; 
#define LAZY_SOLUTION_LOG(s)               \
		if(what_error & S_##s)             \
			log_file << "Trying to solve by " << #s; 
		COMPILE_ERROR_LIST(LAZY_LOG, LAZY_SOLUTION_LOG)
#undef LAZY_LOG
	}
}

