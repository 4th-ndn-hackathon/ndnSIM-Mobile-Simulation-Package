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

#include "util/in-memory-storage-lru.hpp"
#include "security/key-chain.hpp"

#include "boost-test.hpp"
#include "../make-interest-data.hpp"

namespace ndn {
namespace util {
namespace tests {

using namespace ndn::tests;

BOOST_AUTO_TEST_SUITE(Util)
BOOST_AUTO_TEST_SUITE(TestInMemoryStorage)
BOOST_AUTO_TEST_SUITE(Lru)

BOOST_AUTO_TEST_CASE(UsedTimeQueue)
{
  InMemoryStorageLru ims;

  Name name1("/insert/1");
  shared_ptr<Data> data1 = makeData(name1);
  ims.insert(*data1);

  Name name2("/insert/2");
  shared_ptr<Data> data2 = makeData(name2);
  ims.insert(*data2);

  Name name3("/insert/3");
  shared_ptr<Data> data3 = makeData(name3);
  ims.insert(*data3);

  shared_ptr<Interest> interest1 = makeInterest(name1);
  shared_ptr<Interest> interest2 = makeInterest(name2);
  shared_ptr<Interest> interest3 = makeInterest(name3);

  ims.find(*interest1);
  ims.find(*interest2);
  ims.find(*interest3);
  ims.find(*interest1);
  ims.find(*interest3);
  ims.find(*interest3);

  ims.evictItem();
  BOOST_CHECK_EQUAL(ims.size(), 2);

  shared_ptr<const Data> found2 = ims.find(*interest2);
  BOOST_CHECK(found2 == nullptr);

  shared_ptr<const Data> found1 = ims.find(*interest1);
  BOOST_CHECK_EQUAL(found1->getName(), name1);
  shared_ptr<const Data> found3 = ims.find(*interest3);
  BOOST_CHECK_EQUAL(found3->getName(), name3);
}

BOOST_AUTO_TEST_CASE(UsedTimeQueue2)
{
  InMemoryStorageLru ims;

  Name name1("/insert/1");
  shared_ptr<Data> data = makeData(name1);
  ims.insert(*data);

  Name name2("/insert/2");
  shared_ptr<Data> data2 = makeData(name2);
  ims.insert(*data2);

  Name name3("/insert/3");
  shared_ptr<Data> data3 = makeData(name3);
  ims.insert(*data3);

  shared_ptr<Interest> interest1 = makeInterest(name1);
  shared_ptr<Interest> interest2 = makeInterest(name2);
  shared_ptr<Interest> interest3 = makeInterest(name3);

  ims.find(*interest1);
  ims.find(*interest2);
  ims.find(*interest3);
  ims.find(*interest1);
  ims.find(*interest3);
  ims.find(*interest3);

  ims.evictItem();
  BOOST_CHECK_EQUAL(ims.size(), 2);

  shared_ptr<const Data> found2 = ims.find(*interest2);
  BOOST_CHECK(found2 == nullptr);

  shared_ptr<const Data> found1 = ims.find(*interest1);
  BOOST_CHECK_EQUAL(found1->getName(), name1);
  shared_ptr<const Data> found3 = ims.find(*interest3);
  BOOST_CHECK_EQUAL(found3->getName(), name3);

  Name name4("/insert/4");
  shared_ptr<Data> data4 = makeData(name4);
  ims.insert(*data4);

  shared_ptr<Interest> interest4 = makeInterest(name4);
  ims.find(*interest4);
  ims.find(*interest1);
  ims.find(*interest3);

  ims.evictItem();
  BOOST_CHECK_EQUAL(ims.size(), 2);

  shared_ptr<const Data> found4 = ims.find(*interest4);
  BOOST_CHECK(found4 == nullptr);

  found1 = ims.find(*interest1);
  BOOST_CHECK_EQUAL(found1->getName(), name1);
  found3 = ims.find(*interest3);
  BOOST_CHECK_EQUAL(found3->getName(), name3);
}

BOOST_AUTO_TEST_SUITE_END() // Lru
BOOST_AUTO_TEST_SUITE_END() // TestInMemoryStorage
BOOST_AUTO_TEST_SUITE_END() // Util

} // namespace tests
} // namespace util
} // namespace ndn
