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

#include "nfdc/channel-module.hpp"

#include "module-fixture.hpp"

namespace nfd {
namespace tools {
namespace nfdc {
namespace tests {

BOOST_AUTO_TEST_SUITE(Nfdc)
BOOST_FIXTURE_TEST_SUITE(TestChannelModule, ModuleFixture<ChannelModule>)

const std::string STATUS_XML = stripXmlSpaces(R"XML(
  <channels>
    <channel>
      <localUri>tcp4://192.0.2.1:6363</localUri>
    </channel>
    <channel>
      <localUri>ws://[::]:9696/NFD</localUri>
    </channel>
  </channels>
)XML");

const std::string STATUS_TEXT = std::string(R"TEXT(
Channels:
  tcp4://192.0.2.1:6363
  ws://[::]:9696/NFD
)TEXT").substr(1);

BOOST_AUTO_TEST_CASE(Status)
{
  this->fetchStatus();
  ChannelStatus payload1;
  payload1.setLocalUri("tcp4://192.0.2.1:6363");
  ChannelStatus payload2;
  payload2.setLocalUri("ws://[::]:9696/NFD");
  this->sendDataset("/localhost/nfd/faces/channels", payload1, payload2);
  this->prepareStatusOutput();

  BOOST_CHECK(statusXml.is_equal(STATUS_XML));
  BOOST_CHECK(statusText.is_equal(STATUS_TEXT));
}

BOOST_AUTO_TEST_SUITE_END() // TestChannelModule
BOOST_AUTO_TEST_SUITE_END() // Nfdc

} // namespace tests
} // namespace nfdc
} // namespace tools
} // namespace nfd
