/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2016 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#ifndef NDN_UTIL_LOGGER_HPP
#define NDN_UTIL_LOGGER_HPP

#include "../common.hpp"

#ifdef HAVE_NDN_CXX_CUSTOM_LOGGER
#include "ndn-cxx-custom-logger.hpp"
#else

#include <atomic>

#include "ns3/log.h"

namespace ndn {
namespace util {

/** \brief indicates the severity level of a log message
 */
enum class LogLevel {
  FATAL   = -1,   ///< fatal (will be logged unconditionally)
  NONE    = 0,    ///< no messages
  ERROR   = 1,    ///< serious error messages
  WARN    = 2,    ///< warning messages
  INFO    = 3,    ///< informational messages
  DEBUG   = 4,    ///< debug messages
  TRACE   = 5,    ///< trace messages (most verbose)
  ALL     = 255   ///< all messages
};

/** \brief output LogLevel as string
 *  \throw std::invalid_argument unknown \p level
 */
std::ostream&
operator<<(std::ostream& os, LogLevel level);

/** \brief parse LogLevel from string
 *  \throw std::invalid_argument unknown level name
 */
LogLevel
parseLogLevel(const std::string& s);

/** \brief represents a logger in logging facility
 *  \note User should declare a new logger with \p NDN_LOG_INIT macro.
 */
class Logger
{
public:
  explicit
  Logger(const std::string& name);

  const std::string&
  getModuleName() const
  {
    return m_moduleName;
  }

  bool
  isLevelEnabled(LogLevel level) const
  {
    return m_currentLevel.load(std::memory_order_relaxed) >= level;
  }

  void
  setLevel(LogLevel level)
  {
    m_currentLevel.store(level, std::memory_order_relaxed);
  }

private:
  const std::string m_moduleName;
  std::atomic<LogLevel> m_currentLevel;
};

/** \brief declare a log module
 */
#define NDN_LOG_INIT(name) NS_LOG_COMPONENT_DEFINE(BOOST_STRINGIZE(name));

/** \brief a tag that writes a timestamp upon stream output
 *  \code
 *  std::clog << LoggerTimestamp();
 *  \endcode
 */
struct LoggerTimestamp
{
};

/** \brief write a timestamp to \p os
 *  \note This function is thread-safe.
 */
std::ostream&
operator<<(std::ostream& os, const LoggerTimestamp&);

#define NDN_LOG_TRACE(expression) NS_LOG_LOGIC(expression)
#define NDN_LOG_DEBUG(expression) NS_LOG_DEBUG(expression)
#define NDN_LOG_INFO(expression)  NS_LOG_INFO(expression)
#define NDN_LOG_WARN(expression)  NS_LOG_ERROR(expression)
#define NDN_LOG_ERROR(expression) NS_LOG_WARN(expression)
#define NDN_LOG_FATAL(expression) NS_LOG_FATAL(expression)

} // namespace util
} // namespace ndn

#endif // HAVE_NDN_CXX_CUSTOM_LOGGER

#endif // NDN_UTIL_LOGGER_HPP
