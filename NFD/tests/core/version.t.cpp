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

#include "core/version.hpp"

#include "tests/test-common.hpp"

#include <stdio.h>

namespace nfd {
namespace tests {

BOOST_AUTO_TEST_SUITE(TestVersion)

BOOST_AUTO_TEST_CASE(VersionNumber)
{
  BOOST_TEST_MESSAGE("NFD_VERSION = " + to_string(NFD_VERSION));

  BOOST_CHECK_EQUAL(NFD_VERSION, NFD_VERSION_MAJOR * 1000000 +
                                 NFD_VERSION_MINOR * 1000 +
                                 NFD_VERSION_PATCH);
}

BOOST_AUTO_TEST_CASE(VersionString)
{
  BOOST_TEST_MESSAGE("NFD_VERSION_STRING = " NFD_VERSION_STRING);

  BOOST_STATIC_ASSERT(NFD_VERSION_MAJOR < 1000);
  BOOST_STATIC_ASSERT(NFD_VERSION_MINOR < 1000);
  BOOST_STATIC_ASSERT(NFD_VERSION_PATCH < 1000);
  char buf[12];
  ::snprintf(buf, sizeof(buf), "%d.%d.%d", NFD_VERSION_MAJOR, NFD_VERSION_MINOR, NFD_VERSION_PATCH);

  BOOST_CHECK_EQUAL(NFD_VERSION_STRING, buf);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace nfd
