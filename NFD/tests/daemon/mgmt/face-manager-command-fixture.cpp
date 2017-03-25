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

#include "face-manager-command-fixture.hpp"

namespace nfd {
namespace tests {

FaceManagerCommandNode::FaceManagerCommandNode(ndn::KeyChain& keyChain, uint16_t port)
  : face(getGlobalIoService(), keyChain, {true, true})
  , dispatcher(face, keyChain, ndn::security::SigningInfo())
  , authenticator(CommandAuthenticator::create())
  , manager(faceTable, dispatcher, *authenticator)
{
  dispatcher.addTopPrefix("/localhost/nfd");

  std::string config =
    "face_system\n"
    "{\n"
    "  tcp\n"
    "  {\n"
    "    port " + to_string(port) + "\n"
    "  }\n"
    "  udp\n"
    "  {\n"
    "    port " + to_string(port) + "\n"
    "    mcast no\n"
    "  }\n"
    "  ether\n"
    "  {\n"
    "    mcast no\n"
    "  }\n"
    "}\n"
    "authorizations\n"
    "{\n"
    "  authorize\n"
    "  {\n"
    "    certfile any\n"
    "    privileges\n"
    "    {\n"
    "      faces\n"
    "    }\n"
    "  }\n"
    "}\n"
    "\n";

  ConfigFile cf;
  manager.setConfigFile(cf);
  authenticator->setConfigFile(cf);
  cf.parse(config, false, "dummy-config");
}

FaceManagerCommandNode::~FaceManagerCommandNode()
{
  // Explicitly closing faces is necessary. Otherwise, in a subsequent test case,
  // incoming packets may be delivered to an old socket from a previous test case.
  // This should be handled by the FaceTable destructor - see #2517
  std::vector<std::reference_wrapper<Face>> facesToClose;
  std::copy(faceTable.begin(), faceTable.end(), std::back_inserter(facesToClose));
  for (Face& face : facesToClose) {
    face.close();
  }
}

FaceManagerCommandFixture::FaceManagerCommandFixture()
  : node1(m_keyChain, 16363)
  , node2(m_keyChain, 26363)
{
  advanceClocks(time::milliseconds(1), 5);
}

FaceManagerCommandFixture::~FaceManagerCommandFixture()
{
  advanceClocks(time::milliseconds(1), 5);
}

} // namespace tests
} // namespace nfd
