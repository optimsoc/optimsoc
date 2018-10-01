check_PROGRAMS = \
	check_log \
	check_util \
	check_packet \
	check_hostmod \
	check_hostctrl \
	check_gateway \
	check_cl_mam \
	check_cl_scm \
	check_cl_stm \
	check_cl_ctm \
	check_cl_cdm \
	check_cl_dem_uart \
	check_memaccess \
	check_systracelogger \
	check_coretracelogger \
	check_terminal

check_hostmod_SOURCES = \
	check_hostmod.c \
	mock_host_controller.c

check_gateway_SOURCES = \
	check_gateway.c \
	mock_host_controller.c

check_cl_mam_SOURCES = \
	check_cl_mam.c \
	mock_hostmod.c

check_cl_scm_SOURCES = \
	check_cl_scm.c \
	mock_hostmod.c

check_cl_stm_SOURCES = \
	check_cl_stm.c \
	mock_hostmod.c

check_cl_ctm_SOURCES = \
	check_cl_ctm.c \
	mock_hostmod.c

check_cl_cdm_SOURCES = \
	check_cl_cdm.c \
	mock_hostmod.c

check_cl_dem_uart_SOURCES = \
	check_cl_dem_uart.c \
	mock_hostmod.c

check_memaccess_SOURCES = \
	check_memaccess.c \
	mock_host_controller.c

check_systracelogger_SOURCES = \
	check_systracelogger.c \
	mock_host_controller.c

check_coretracelogger_SOURCES = \
	check_coretracelogger.c \
	mock_host_controller.c
	
check_terminal_SOURCES = \
	check_terminal.c \
	mock_host_controller.c

TESTS = $(check_PROGRAMS)

AM_CFLAGS = \
	@CHECK_CFLAGS@ \
	-I$(top_srcdir)/src/libosd/include \
	-include $(top_builddir)/config.h

LDADD = \
	@CHECK_LIBS@ \
	$(top_builddir)/src/libosd/libosd.la

# Include make targets to generate code coverage reports
CODE_COVERAGE_IGNORE_PATTERN = "/usr/*"
@CODE_COVERAGE_RULES@
CODE_COVERAGE_BRANCH_COVERAGE=1

# Include make targets to generate check targets with valgrind (check-valgrind)
@VALGRIND_CHECK_RULES@
VALGRIND_memcheck_FLAGS = --leak-check=full
#VALGRIND_SUPPRESSIONS_FILES = my-project.supp
#EXTRA_DIST = my-project.supp

# Include make target to convert check-style XML reports to JUnit-style reports
JUNIT_XML_REPORTS = $(check_PROGRAMS:=.junit.xml)
check-junit-xml: $(JUNIT_XML_REPORTS)
%.junit.xml : %.xml
	xsltproc $(top_srcdir)/tests/tools/check2junit.xsl $^ > $@

# Include make target to convert lcov coverage data to cobertura XML
coverage-cobertura-xml: coverage-cobertura.xml

coverage-cobertura.xml: $(CODE_COVERAGE_OUTPUT_FILE)
	python $(top_srcdir)/tests/tools/lcov_cobertura.py \
		$^ --base-dir $(top_srcdir) --output $@
