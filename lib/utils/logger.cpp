/*
 * logger.cpp
 *
 *  Created on: Oct 1, 2019
 *      Author: oleh
 */

#include <string>
#include <iostream>
#include "../boost_1_71_0/boost/thread.hpp"
#include <ctime>
#include <chrono>
#include <stdint.h>
#include <utils/logger.hpp>

using namespace JetBeep;
using namespace std;
using namespace std::chrono;
using namespace boost;

bool Logger::cerr_enabled = false;
bool Logger::cout_enabled = false;
LoggerLevel Logger::level = SILENT;
thread_local LoggerLevel Logger::m_thread_level = SILENT;

Logger& Logger::endl(Logger& a) {
	if (Logger::cout_enabled) {
		cout << std::endl;
		cout.flush();
	}

	if (Logger::cerr_enabled) {
		cerr << std::endl;
		cerr.flush();
	}

	return a;
}

Logger::Logger(const char *module_name):
m_module_name(module_name) {

}

Logger& Logger::output() {
	string level_str = "";
	char time_str[40];
	time_t now = time(nullptr);

	switch (Logger::m_thread_level) {
	case VERBOSE:
		level_str = "[VERBOSE] "; break;
	case DEBUG:
		level_str = "[DEBUG]   "; break;
	case INFO:
		level_str = "[INFO]    "; break;
	case WARNING:
		level_str = "[WARNING] "; break;
	case ERROR:
		level_str = "[ERROR]   "; break;
	case SILENT:
		level_str = "[SILENT]  "; break;
	default:
		level_str = "[UNKNOWN] "; break;
	}

	strftime(time_str, 40, "%H:%M:%S", localtime(&now));
	uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 1000;
	stringstream ms_ss;

	if (ms < 10) {
		ms_ss << "00" << ms;
	} else if (ms < 100) {
		ms_ss << "0" << ms;
	} else {
		ms_ss << ms;
	}

	return *this << time_str << "." << ms_ss.str() << " " << level_str << "(" << m_module_name << "): ";
}

Logger& Logger::v() {
	Logger::m_thread_level = VERBOSE;
	return output();
}

Logger& Logger::d() {
	Logger::m_thread_level = DEBUG;

	return output();
}

Logger& Logger::i() {
	Logger::m_thread_level = INFO;

	return output();
}

Logger& Logger::w() {
	Logger::m_thread_level = WARNING;

	return output();
}

Logger& Logger::e() {
	Logger::m_thread_level = ERROR;

	return output();
}


