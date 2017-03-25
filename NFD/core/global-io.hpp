/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  Regents of the University of California,
 *                     Arizona Board of Regents,
 *                     Colorado State University,
 *                     University Pierre & Marie Curie, Sorbonne University,
 *                     Washington University in St. Louis,
 *                     Beijing Institute of Technology
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
 **/

#ifndef NFD_CORE_GLOBAL_IO_HPP
#define NFD_CORE_GLOBAL_IO_HPP

#include "common.hpp"

namespace nfd {

namespace detail {

/**
 * @brief Simulator-based IO that implements a few interfaces from boost::asio::io_service
 */
class SimulatorIo
{
public:
  void
  post(const std::function<void()>& callback);

  void
  dispatch(const std::function<void()>& callback);
};

} // namespace detail

/** \return Simulator-based IO object
 */
detail::SimulatorIo&
getGlobalIoService();

} // namespace nfd

#endif // NFD_CORE_GLOBAL_IO_HPP
