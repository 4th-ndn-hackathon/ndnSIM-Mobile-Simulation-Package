/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
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

#include "key-locator.hpp"
#include "encoding/block-helpers.hpp"

#include "boost-test.hpp"

namespace ndn {
namespace tests {

BOOST_AUTO_TEST_SUITE(TestKeyLocator)

BOOST_AUTO_TEST_CASE(TypeNone)
{
  KeyLocator a;
  BOOST_CHECK_EQUAL(a.getType(), KeyLocator::KeyLocator_None);
  BOOST_CHECK_THROW(a.getName(), KeyLocator::Error);
  BOOST_CHECK_THROW(a.getKeyDigest(), KeyLocator::Error);

  Block wire;
  BOOST_REQUIRE_NO_THROW(wire = a.wireEncode());

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  // for (Buffer::const_iterator it = wire.begin(); it != wire.end(); ++it) {
  //   printf("0x%02x, ", *it);
  // }
  static const uint8_t expected[] = {
    0x1c, 0x00
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(expected, expected + sizeof(expected),
                                wire.begin(), wire.end());

  BOOST_REQUIRE_NO_THROW(KeyLocator(wire));
  KeyLocator b(wire);
  BOOST_CHECK(a == b);
  BOOST_CHECK_EQUAL(b.getType(), KeyLocator::KeyLocator_None);
  BOOST_CHECK_THROW(b.getName(), KeyLocator::Error);
  BOOST_CHECK_THROW(b.getKeyDigest(), KeyLocator::Error);
}

BOOST_AUTO_TEST_CASE(TypeName)
{
  KeyLocator a;
  a.setName("/N");
  BOOST_CHECK_EQUAL(a.getType(), KeyLocator::KeyLocator_Name);
  BOOST_CHECK_EQUAL(a.getName(), Name("/N"));
  BOOST_CHECK_THROW(a.getKeyDigest(), KeyLocator::Error);

  Block wire;
  BOOST_REQUIRE_NO_THROW(wire = a.wireEncode());

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  // for (Buffer::const_iterator it = wire.begin(); it != wire.end(); ++it) {
  //   printf("0x%02x, ", *it);
  // }
  static const uint8_t expected[] = {
    0x1c, 0x05, 0x07, 0x03, 0x08, 0x01, 0x4e
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(expected, expected + sizeof(expected),
                                wire.begin(), wire.end());

  BOOST_REQUIRE_NO_THROW(KeyLocator(wire));
  KeyLocator b(wire);
  BOOST_CHECK(a == b);
  BOOST_CHECK_EQUAL(b.getType(), KeyLocator::KeyLocator_Name);
  BOOST_CHECK_EQUAL(b.getName(), Name("/N"));
  BOOST_CHECK_THROW(b.getKeyDigest(), KeyLocator::Error);
}

BOOST_AUTO_TEST_CASE(TypeKeyDigest)
{
  char digestOctets[] = "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD";
  ConstBufferPtr digestBuffer = make_shared<Buffer>(digestOctets, 8);
  Block expectedDigestBlock = makeBinaryBlock(tlv::KeyDigest, digestOctets, 8);

  KeyLocator a;
  a.setKeyDigest(digestBuffer);
  BOOST_CHECK_EQUAL(a.getType(), KeyLocator::KeyLocator_KeyDigest);
  BOOST_CHECK(a.getKeyDigest() == expectedDigestBlock);
  BOOST_CHECK_THROW(a.getName(), KeyLocator::Error);

  Block wire;
  BOOST_REQUIRE_NO_THROW(wire = a.wireEncode());

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  // for (Buffer::const_iterator it = wire.begin(); it != wire.end(); ++it) {
  //   printf("0x%02x, ", *it);
  // }
  static const uint8_t expected[] = {
    0x1c, 0x0a, 0x1d, 0x08, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(expected, expected + sizeof(expected),
                                wire.begin(), wire.end());

  BOOST_REQUIRE_NO_THROW(KeyLocator(wire));
  KeyLocator b(wire);
  BOOST_CHECK(a == b);
  BOOST_CHECK_EQUAL(b.getType(), KeyLocator::KeyLocator_KeyDigest);
  BOOST_CHECK(b.getKeyDigest() == expectedDigestBlock);
  BOOST_CHECK_THROW(b.getName(), KeyLocator::Error);
}

BOOST_AUTO_TEST_CASE(Equality)
{
  KeyLocator a;
  KeyLocator b;
  BOOST_CHECK_EQUAL(a == b, true);
  BOOST_CHECK_EQUAL(a != b, false);

  a.setName("ndn:/A");
  BOOST_CHECK_EQUAL(a == b, false);
  BOOST_CHECK_EQUAL(a != b, true);

  b.setName("ndn:/B");
  BOOST_CHECK_EQUAL(a == b, false);
  BOOST_CHECK_EQUAL(a != b, true);

  b.setName("ndn:/A");
  BOOST_CHECK_EQUAL(a == b, true);
  BOOST_CHECK_EQUAL(a != b, false);

  char digestOctets[] = "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD";
  ConstBufferPtr digestBuffer = make_shared<Buffer>(digestOctets, 8);

  a.setKeyDigest(digestBuffer);
  BOOST_CHECK_EQUAL(a == b, false);
  BOOST_CHECK_EQUAL(a != b, true);

  b.setKeyDigest(digestBuffer);
  BOOST_CHECK_EQUAL(a == b, true);
  BOOST_CHECK_EQUAL(a != b, false);
}

BOOST_AUTO_TEST_CASE(UnknownType)
{
  static const uint8_t wireOctets[] = {
    0x1c, 0x03, 0x7f, 0x01, 0xcc
  };
  Block wire(wireOctets, sizeof(wireOctets));
  BOOST_REQUIRE_NO_THROW(KeyLocator(wire));
  KeyLocator a(wire);
  BOOST_CHECK_EQUAL(a.getType(), KeyLocator::KeyLocator_Unknown);

  KeyLocator b(wire);
  BOOST_CHECK_EQUAL(a == b, true);
  BOOST_CHECK_EQUAL(a != b, false);

  b.setName("/N");
  BOOST_CHECK_EQUAL(a == b, false);
  BOOST_CHECK_EQUAL(a != b, true);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace ndn
