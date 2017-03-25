/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_CORE_LOGGER_HPP
#define NFD_CORE_LOGGER_HPP

#include "common.hpp"
#include "ns3/log.h"

namespace nfd {

#define NFD_LOG_INIT(name) NS_LOG_COMPONENT_DEFINE("nfd." name);

#define NFD_LOG_INCLASS_DECLARE() \
static ns3::LogComponent g_log

#define NFD_LOG_INCLASS_DEFINE(cls, name) \
ns3::LogComponent cls::g_log = ns3::LogComponent ("nfd." name, __FILE__)

#define NFD_LOG_INCLASS_TEMPLATE_DEFINE(cls, name) \
template<class T>                                  \
ns3::LogComponent cls<T>::g_log = ns3::LogComponent ("nfd." name, __FILE__)

#define NFD_LOG_INCLASS_TEMPLATE_SPECIALIZATION_DEFINE(cls, specialization, name) \
template<>                                                                        \
ns3::LogComponent cls<specialization>::g_log = ns3::LogComponent ("nfd." name, __FILE__)

#define NFD_LOG_INCLASS_2TEMPLATE_SPECIALIZATION_DEFINE(cls, s1, s2, name) \
template<>                                                                 \
s3::LogComponent cls<s1, s2>::g_log = ns3::LogComponent ("nfd." name, __FILE__)

#define NFD_LOG_TRACE(expression) NS_LOG_LOGIC(expression)
#define NFD_LOG_DEBUG(expression) NS_LOG_DEBUG(expression)
#define NFD_LOG_INFO(expression) NS_LOG_INFO(expression)
#define NFD_LOG_ERROR(expression) NS_LOG_ERROR(expression)
#define NFD_LOG_WARN(expression) NS_LOG_WARN(expression)
#define NFD_LOG_FATAL(expression) NS_LOG_FATAL(expression)

} // namespace nfd

#endif // NFD_CORE_LOGGER_HPP
