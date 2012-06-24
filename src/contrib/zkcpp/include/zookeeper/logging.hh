#ifndef LOGGING_H_
#define LOGGING_H_

#include <boost/format.hpp>
#include <log4cxx/logger.h>

#define LOG_DEBUG(message) LOG4CXX_DEBUG(logger, message)
#define LOG_INFO(message) LOG4CXX_DEBUG(logger, message)
#define LOG_WARN(message) LOG4CXX_DEBUG(logger, message)
#define LOG_ERROR(message) LOG4CXX_DEBUG(logger, message)
#define ENABLE_LOGGING \
  static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(__FILE__));

#endif  // LOGGING_H_