/*
 * logger.h
 *
 *  Created on: Oct 1, 2019
 *      Author: oleh
 */

#ifndef LIB_UTILS_LOGGER_H_
#define LIB_UTILS_LOGGER_H_

#include <string>
#include <iostream>

namespace jetbeep {
	enum logger_level {
		VERBOSE = 0,
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		SILENT
	};

	class logger {
	public:
		logger(const char* module_name);

		logger& v();
		logger& d();
		logger& i();
		logger& w();
		logger& e();

		static logger_level level;
		static logger& endl(logger& a);

		static bool cout_enabled;
		static bool cerr_enabled;

	    typedef logger& (*logger_manipulator)(logger&);
	    logger& operator<<(logger_manipulator manip)
	    {
	        // call the function, and return it's value
	        return manip(*this);
	    }

		template<class T> logger& operator << (const T &t) {
			if (logger::_thread_level >= logger::level) {
				coutValue(t);
				cerrValue(t);
			}

			return *this;
		}
	private:
		std::string _module_name;
		logger& output();

		static thread_local logger_level _thread_level;

		template<class T> void coutValue(T t) {
			if (!cout_enabled) {
				return;
			}

			std::cout << t;
		}

		template<class T> void cerrValue(T t) {
			if (!cerr_enabled) {
				return;
			}

			std::cerr << t;
		}
	};
}


#endif /* LIB_UTILS_LOGGER_H_ */
