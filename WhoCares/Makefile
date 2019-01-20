# tested under gcc version 7.3.0 (Ubuntu 7.3.0-27ubuntu1~18.04) 
# in windows tested with Visual Studio 2017

# Progress bar defs
#--------------------------------
#  words = count the number of words
ifneq ($(words $(MAKECMDGOALS)),1) # if no argument was given to make...
.DEFAULT_GOAL = all # set the default goal to all
#  http://www.gnu.org/software/make/manual/make.html
#  $@ = target name
#  %: = last resort recipe
#  --no-print-directory = don't print enter/leave messages for each output grouping
#  MAKEFILE_LIST = has a list of all the parsed Makefiles that can be found *.mk, Makefile, etc
#  -n = dry run, just print the recipes
#  -r = no builtin rules, disables implicit rules
#  -R = no builtin variables, disables implicit variables
#  -f = specify the name of the Makefile

%:                  # define a last resort default rule
	@$(MAKE) $@ --no-print-directory -rRf $(firstword $(MAKEFILE_LIST))  
# recursive make call
else
ifndef ECHO
#  execute a dry run of make, defining echo beforehand, and count all the instances of "COUNTTHIS"
T := $(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory \
      -nrRf $(firstword $(MAKEFILE_LIST)) \
      ECHO="COUNTTHIS" | grep -c "COUNTTHIS")
#  eval = evaluate the text and read the results as makefile commands
N := x
#  Recursively expand C for each instance of ECHO to count more x's
C = $(words $N)$(eval N := x $N)
#  Multipy the count of x's by 100, and divide by the count of "COUNTTHIS"
#  Followed by a percent sign
#  And wrap it all in square brackets
ECHO = echo -ne "\r [`expr $C '*' 100 / $T`%]"
endif
#------------------
# end progress bar
#------------------

CXX = g++
CC = gcc
PROM = whocares

DEPS = attribute.h bootstrapper.h builtins.h checks.h console.h factory.h game.h listeners.h macro-utils.h parser-ast.h pretty_print.h templates.h templates_library.h token.h whocares.h world.h worldgen.h wrapper.h

SRC = bootstrapper.cc builtins.cc checks.cc compile-debugger.cc console.cc factory.cc parser-ast.cc whocares.cc world.cc worldgen.cc 

OBJ = $(SRC:%.c=%.o)
GCCVERSIONGTEQ71 := $(shell expr `gcc -dumpfullversion | cut -f1-2 -d.` \>= 7.1)

# https://stackoverflow.com/questions/33149878/experimentalfilesystem-linker-error
CXXFLAGS = -std=c++1z -Wall -Wextra -pedantic-errors -g -ggdb
CCFLAGS = -g -ggdb
CXXFLAGSEND = -lstdc++fs

.PHONY: all

all: CheckVersion $(PROM)
	@$(ECHO) All done

debug: CXXFLAGS += -ggdb
debug: CCFLAGS +=  -ggdb
debug: all

$(PROM): $(OBJ)
	@$(ECHO) Linking $@
	@$(CXX) $(CXXFLAGS) -o $(PROM) $(OBJ) $(CXXFLAGSEND)
	@touch $@

# http://www.cs.utexas.edu/~cannata/cs345/GNU%20Make%20Manual.pdf 4.11.1 Page.38(48)
%.o: %.c $(DEPS) Makefile
	@$(ECHO) Compiling $@
	$(CC) $(CCFLAGS) -c $^ -o $@
	@touch $@

CheckVersion:
ifeq "$(GCCVERSIONGTEQ71)" "0"
	@echo +-----------------------------------------------------------------------+
	@echo "|\033[31m Your GCC may may not support C++17. Please update to gcc7.1 or higher \033[0m|"
	@echo +-----------------------------------------------------------------------+
endif

endif

