#include "checks.h"

namespace Debugger {
	void logError(ERROR_INTERFACE* what_error, UNUSED std::string initiater) {
	    IGNORE(initiater);
		delete what_error;
		//TODO
	}
}

