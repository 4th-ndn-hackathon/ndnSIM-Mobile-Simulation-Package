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

#ifndef NFD_DAEMON_MGMT_NFD_MANAGER_BASE_HPP
#define NFD_DAEMON_MGMT_NFD_MANAGER_BASE_HPP

#include "core/manager-base.hpp"
#include "command-authenticator.hpp"

namespace nfd {

using ndn::mgmt::Dispatcher;
using ndn::nfd::ControlParameters;

/**
 * @brief a collection of common functions shared by all NFD managers,
 *        such as communicating with the dispatcher and command validator.
 */
class NfdManagerBase : public ManagerBase
{
public:
  NfdManagerBase(Dispatcher& dispatcher,
                 CommandAuthenticator& authenticator,
                 const std::string& module);

PUBLIC_WITH_TESTS_ELSE_PRIVATE: // command validation
  virtual ndn::mgmt::Authorization
  makeAuthorization(const std::string& verb) override;

private:
  CommandAuthenticator& m_authenticator;
};

} // namespace nfd

#endif // NFD_DAEMON_MGMT_NFD_MANAGER_BASE_HPP
