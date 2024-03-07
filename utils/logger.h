#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <stdio.h>

#define va_buffer(ap,parmN,buffer) { va_list ap; va_start(ap, parmN); vsnprintf(buffer, sizeof(buffer), parmN, ap); va_end(ap); }

#define PRINT_DEBUG_LOGS 1

namespace tslog
{
	enum class level { VERBOSE, LOG, INFO, WARNINGS, ERRORS, CRITICAL, DEBUG, OFF };

	static FILE* file = NULL;
	static level logLevel;
	static bool bShowConsole;

	bool init(level loglevel, bool showConsole);
	bool shutdown();
	void debug(const char* text, ...);
	void critical(const char* text, ...);
	void info(const char* text, ...);
	void log(const char* text, ...);
	void verbose(const char* text, ...);
	bool setLevel(int level);
}