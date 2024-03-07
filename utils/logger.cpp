#include "logger.h"

bool tslog::init(level _logLevel, bool showConsole)
{
	logLevel = _logLevel;
	bShowConsole = showConsole;
	if (logLevel < level::OFF)
	{
		if (bShowConsole)
			if (AllocConsole())
				freopen_s(&file, "CONOUT$", "w", stdout);
			else
				bShowConsole = false;
	}
	return true;
}

bool tslog::shutdown()
{
	log("Closing logger...");
	if (logLevel >= level::OFF)
		return true;
	Sleep(5000);
	if (bShowConsole && file != NULL)
	{
		fclose(file);
		FreeConsole();
	}
	return true;
}

void tslog::debug(const char* text, ...)
{
#if not PRINT_DEBUG_LOGS
	return;
#endif
	if (logLevel < level::OFF)
		if (bShowConsole)
		{
			char buffer[256];
			va_buffer(args, text, buffer);
			std::cout << "[DEBUG]: " << buffer << std::endl;
		}
}

void tslog::critical(const char* text, ...)
{
	if (logLevel <= level::CRITICAL)
		if (bShowConsole)
		{
			char buffer[256];
			va_buffer(args, text, buffer);
			std::cout << "[CRITICAL]: " << buffer << std::endl;
		}
}

void tslog::info(const char* text, ...)
{
	if (logLevel <= level::INFO)
		if (bShowConsole)
		{
			char buffer[256];
			va_buffer(args, text, buffer);
			std::cout << "[INFO]: " << buffer << std::endl;
		}
}

void tslog::log(const char* text, ...)
{
	if (logLevel <= level::LOG)
		if (bShowConsole)
		{
			char buffer[256];
			va_buffer(args, text, buffer);
			std::cout << "[LOG]: " << buffer << std::endl;
		}
}

void tslog::verbose(const char* text, ...)
{
	if (logLevel <= level::VERBOSE)
		if (bShowConsole)
		{
			char buffer[256];
			va_buffer(args, text, buffer);
			std::cout << "[VERBOSE]: " << buffer << std::endl;
		}
}

bool tslog::setLevel(int level)
{
	return true;
}