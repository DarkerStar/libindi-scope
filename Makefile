##############################################################################
#
# This file is part of libindi-scope.
#
# libindi-scope is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libindi-scope is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libindi-scope.  If not, see <https://www.gnu.org/licenses/>.
#
##############################################################################

##############################################################################
#
# This makefile supports the following commands/pseudo-targets:
#
#   *   all
#
#       Default target. Does nothing.
#
#   *   clean
#
#       Deletes all generated files. Returns project directory to pristine
#       state.
#
#   *   test
#
#       Builds the test executables for all modules. If that succeeds, then
#       runs all test executables.
#
#   *   run-tests
#
#       Runs all test executables WITHOUT rebuilding. (So this may fail if
#       all test executables don't already exist.)
#
#   *   build-tests
#
#       Builds the test executables for all modules, but does not run them.
#
#   *   <module>
#
#       Builds the test executable for the module, and if that succeeds,
#       runs the test executable.
#
##############################################################################

# List of test modules
modules ::= scope_exit \
            scope_success \
            scope_fail

# General configuration ######################################################

# Generally a good idea to specify the shell
SHELL ::= /bin/sh

# Restrict the suffixes to the ones used
.SUFFIXES :
.SUFFIXES : .cpp .hpp .o .d

# Dependencies directory
depsdir ::= .deps

# Default (`all`) target #####################################################

.PHONY : all

# Does nothing.
all :

# Install target #############################################################

.PHONY : install

# Print an error message, then trigger a make failure.
install :
	@printf '%s\n' 'This project is not meant to be installed.' >&2
	@false

# Test targets ###############################################################

.PHONY : test run-tests build-tests

# Canned recipe to run all tests, and then trigger a make error if any failed.
define do-run-tests
all_passed=true ; \
for m in ${modules} ; \
	do printf '%s\n' "Running test $$m..." ; \
	./$${m}.test || all_passed=false ; \
done ; \
$${all_passed}
endef

# Build all tests, then run them all.
test : build-tests
	@$(do-run-tests)

# Build all tests, but do not run them.
build-tests : ${modules:=.test}

# Run all tests, but do not build them.
run-tests :
	@$(do-run-tests)

# Test executables ###########################################################

# Canned recipe for modules that defines the module target, and module test
# executable target.
define module-testing

.PHONY : ${1}

${1} : ${1}.test
	@printf '%s\n' 'Running test ${1}...'
	@./${1}.test

${1}.test : indi/${1}.test.o
	$${CXX} $${CXXFLAGS} $${CPPFLAGS} $${LDFLAGS} -o $${@} $${^} $${LDLIBS}

endef

# For each module, generate the module target and module test executable
# target.
$(eval $(foreach module,${modules},$(call module-testing,${module})))

# Compile command:
#
#   1.  First make sure the necessary dependency directory exists.
#   2.  Print the (theoretical) compile command line.
#   3.  Do the actual compile. Extra compile arguments are used to generate
#       dependency information on the fly.
#   4.  Make the dependency file itself also dependent.
#   5.  Remove temporary files.
$(addprefix indi/,${modules:=.test.o}) : %.o : %.cpp
	@mkdir -p -- "${@D}" "${depsdir}/${*D}"
	@printf '%s\n' "${CXX} ${CXXFLAGS} ${CPPFLAGS} -I. -c -o ${@} ${<}"
	@${CXX} ${CXXFLAGS} ${CPPFLAGS} -I. -MMD -MP -MF "${depsdir}/${*}.d.tmp" -c -o ${@} ${<}
	@{ printf '%s ' "${depsdir}/${*}.d" && cat "${depsdir}/${*}.d.tmp" ; } >"${depsdir}/${*}.d"
	-@rm -f -- "${depsdir}/${*}.d.tmp"

# Include any existing dependency files.
#
# If any are missing, no worries. They will be regenerated as needed.
-include $(addprefix ${depsdir}/indi/,${modules:=.test.d})

# Clean ######################################################################

.PHONY : clean

# Remove the test executables, object files, and dependencies directory
clean :
	-@rm -f -- ${modules:=.test}
	-@rm -f -- $(addprefix indi/,${modules:=.test.o})
	-@rm -rf -- ${depsdir}
