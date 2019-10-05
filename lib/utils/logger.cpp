/*
 * logger.cpp
 *
 *  Created on: Oct 1, 2019
 *      Author: oleh
 */

#include "logger.h"
#include <string>
#include <iostream>
#include <boost/thread.hpp>
#include <ctime>
#include <chrono>
#include <stdint.h>

using namespace jetbeep;
using namespace std;
using namespace std::chrono;
using namespace boost;

bool logger::cerr_enabled = false;
bool logger::cout_enabled = false;
logger_level logger::level = SILENT;
thread_local logger_level logger::_thread_level = SILENT;

logger& logger::endl(logger& a) {
	if (logger::cout_enabled) {
		cout << std::endl;
		cout.flush();
	}

	if (logger::cerr_enabled) {
		cerr << std::endl;
		cerr.flush();
	}

	return a;
}

logger::logger(const char *module_name):
_module_name(module_name) {

}

logger& logger::output() {
	string level_str = "";
	char time_str[40];
	time_t now = time(nullptr);

	switch (logger::_thread_level) {
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

	return *this << time_str << "." << ms_ss.str() << " " << level_str << "(" << _module_name << "): ";
}

logger& logger::v() {
	logger::_thread_level = VERBOSE;
	return output();
}

logger& logger::d() {
	logger::_thread_level = DEBUG;

	return output();
}

logger& logger::i() {
	logger::_thread_level = INFO;

	return output();
}

logger& logger::w() {
	logger::_thread_level = WARNING;

	return output();
}

logger& logger::e() {
	logger::_thread_level = ERROR;

	return output();
}


