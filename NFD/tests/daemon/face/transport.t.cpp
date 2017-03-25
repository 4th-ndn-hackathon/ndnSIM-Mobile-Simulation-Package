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

#include "face/transport.hpp"
#include "face/face.hpp"
#include "dummy-transport.hpp"
#include "dummy-receive-link-service.hpp"
#include "transport-test-common.hpp"

#include <boost/mpl/empty_sequence.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/transform_view.hpp>

namespace nfd {
namespace face {
namespace tests {

using namespace nfd::tests;
namespace mpl = boost::mpl;

class DummyTransportFixture : public BaseFixture
{
protected:
  DummyTransportFixture()
    : transport(nullptr)
    , sentPackets(nullptr)
    , receivedPackets(nullptr)
  {
    // Constructor does not initialize the fixture,
    // so that test case may specify different parameters to DummyTransport constructor.
  }

  void
  initialize(unique_ptr<DummyTransport> transport = make_unique<DummyTransport>())
  {
    this->face = make_unique<Face>(make_unique<DummyReceiveLinkService>(), std::move(transport));
    this->transport = static_cast<DummyTransport*>(face->getTransport());
    this->sentPackets = &this->transport->sentPackets;
    this->receivedPackets = &static_cast<DummyReceiveLinkService*>(face->getLinkService())->receivedPackets;
  }

protected:
  unique_ptr<Face> face;
  DummyTransport* transport;
  std::vector<Transport::Packet>* sentPackets;
  std::vector<Transport::Packet>* receivedPackets;
};

BOOST_AUTO_TEST_SUITE(Face)
BOOST_FIXTURE_TEST_SUITE(TestTransport, DummyTransportFixture)

BOOST_AUTO_TEST_CASE(DummyTransportStaticProperties)
{
  this->initialize();
  checkStaticPropertiesInitialized(*transport);
}

/** \brief a macro to declare a TransportState as a integral constant
 */
#define TRANSPORT_STATE_C(X) mpl::int_<static_cast<int>(TransportState::X)>

/** \brief a map from every TransportState to a valid state transition sequence
 *         for entering this state from UP
 */
typedef mpl::map<
  mpl::pair<TRANSPORT_STATE_C(UP),
    mpl::vector<>>,
  mpl::pair<TRANSPORT_STATE_C(DOWN),
    mpl::vector<
      TRANSPORT_STATE_C(DOWN)
    >>,
  mpl::pair<TRANSPORT_STATE_C(CLOSING),
    mpl::vector<
      TRANSPORT_STATE_C(CLOSING)
    >>,
  mpl::pair<TRANSPORT_STATE_C(FAILED),
    mpl::vector<
      TRANSPORT_STATE_C(FAILED)
    >>,
  mpl::pair<TRANSPORT_STATE_C(CLOSED),
    mpl::vector<
      TRANSPORT_STATE_C(CLOSING),
      TRANSPORT_STATE_C(CLOSED)
    >>
> StateEntering;

/** \brief a sequence of all valid TransportStates
 */
typedef mpl::fold<StateEntering,
  mpl::vector<>,
  mpl::push_back<mpl::_1, mpl::first<mpl::_2>>
>::type States;

/** \brief a set of all valid state transitions
 */
typedef mpl::set<
  mpl::pair<TRANSPORT_STATE_C(UP), TRANSPORT_STATE_C(DOWN)>,
  mpl::pair<TRANSPORT_STATE_C(DOWN), TRANSPORT_STATE_C(UP)>,
  mpl::pair<TRANSPORT_STATE_C(UP), TRANSPORT_STATE_C(CLOSING)>,
  mpl::pair<TRANSPORT_STATE_C(UP), TRANSPORT_STATE_C(FAILED)>,
  mpl::pair<TRANSPORT_STATE_C(DOWN), TRANSPORT_STATE_C(CLOSING)>,
  mpl::pair<TRANSPORT_STATE_C(DOWN), TRANSPORT_STATE_C(FAILED)>,
  mpl::pair<TRANSPORT_STATE_C(CLOSING), TRANSPORT_STATE_C(CLOSED)>,
  mpl::pair<TRANSPORT_STATE_C(FAILED), TRANSPORT_STATE_C(CLOSED)>
> ValidStateTransitions;

/** \brief a metafunction class to generate a sequence of all state transitions
 *         from a specified state
 */
struct StateTransitionsFrom
{
  template<typename FROM>
  struct apply
  {
    typedef typename mpl::fold<
      States,
      mpl::vector<>,
      mpl::push_back<mpl::_1, mpl::pair<FROM, mpl::_2>>
    >::type type;
  };
};

/** \brief a sequence of all state transitions
 */
typedef mpl::fold<
  States,
  mpl::empty_sequence,
  mpl::joint_view<mpl::_1, mpl::apply<StateTransitionsFrom, mpl::protect<mpl::_1>>::type>
>::type AllStateTransitions;

#undef TRANSPORT_STATE_C

BOOST_AUTO_TEST_CASE_TEMPLATE(SetState, T, AllStateTransitions)
{
  this->initialize();

  TransportState from = static_cast<TransportState>(T::first::value);
  TransportState to = static_cast<TransportState>(T::second::value);
  BOOST_TEST_MESSAGE("SetState " << from << " -> " << to);

  // enter from state
  typedef typename mpl::at<StateEntering, mpl::int_<T::first::value>>::type Steps;
  mpl::for_each<Steps>(
    [this] (int state) { transport->setState(static_cast<TransportState>(state)); });
  BOOST_REQUIRE_EQUAL(transport->getState(), from);

  bool hasSignal = false;
  this->transport->afterStateChange.connect(
    [from, to, &hasSignal] (TransportState oldState, TransportState newState) {
      hasSignal = true;
      BOOST_CHECK_EQUAL(oldState, from);
      BOOST_CHECK_EQUAL(newState, to);
    });

  // do transition
  bool isValid = from == to ||
                 mpl::has_key<ValidStateTransitions,
                   mpl::pair<mpl::int_<T::first::value>, mpl::int_<T::second::value>>
                 >::value;
  if (isValid) {
    BOOST_REQUIRE_NO_THROW(transport->setState(to));
    BOOST_CHECK_EQUAL(hasSignal, from != to);
  }
  else {
    BOOST_CHECK_THROW(this->transport->setState(to), std::runtime_error);
  }
}

BOOST_AUTO_TEST_CASE(NoExpirationTime)
{
  initialize();

  BOOST_CHECK_EQUAL(transport->getExpirationTime(), time::steady_clock::TimePoint::max());
}

BOOST_AUTO_TEST_CASE(Send)
{
  this->initialize();

  Block pkt1 = ndn::encoding::makeStringBlock(300, "Lorem ipsum dolor sit amet,");
  transport->send(Transport::Packet(Block(pkt1)));

  Block pkt2 = ndn::encoding::makeStringBlock(301, "consectetur adipiscing elit,");
  transport->send(Transport::Packet(Block(pkt2)));

  transport->setState(TransportState::DOWN);
  Block pkt3 = ndn::encoding::makeStringBlock(302, "sed do eiusmod tempor incididunt ");
  transport->send(Transport::Packet(Block(pkt3)));

  transport->setState(TransportState::CLOSING);
  Block pkt4 = ndn::encoding::makeStringBlock(303, "ut labore et dolore magna aliqua.");
  transport->send(Transport::Packet(Block(pkt4)));

  BOOST_CHECK_EQUAL(transport->getCounters().nOutPackets, 2);
  BOOST_CHECK_EQUAL(transport->getCounters().nOutBytes, pkt1.size() + pkt2.size());
  BOOST_REQUIRE_EQUAL(sentPackets->size(), 3);
  BOOST_CHECK(sentPackets->at(0).packet == pkt1);
  BOOST_CHECK(sentPackets->at(1).packet == pkt2);
  BOOST_CHECK(sentPackets->at(2).packet == pkt3);
}

BOOST_AUTO_TEST_CASE(Receive)
{
  this->initialize();

  Block pkt1 = ndn::encoding::makeStringBlock(300, "Lorem ipsum dolor sit amet,");
  transport->receivePacket(pkt1);

  Block pkt2 = ndn::encoding::makeStringBlock(301, "consectetur adipiscing elit,");
  transport->receivePacket(pkt2);

  transport->setState(TransportState::DOWN);
  Block pkt3 = ndn::encoding::makeStringBlock(302, "sed do eiusmod tempor incididunt ");
  transport->receivePacket(pkt3);

  BOOST_CHECK_EQUAL(transport->getCounters().nInPackets, 3);
  BOOST_CHECK_EQUAL(transport->getCounters().nInBytes, pkt1.size() + pkt2.size() + pkt3.size());
  BOOST_REQUIRE_EQUAL(receivedPackets->size(), 3);
  BOOST_CHECK(receivedPackets->at(0).packet == pkt1);
  BOOST_CHECK(receivedPackets->at(1).packet == pkt2);
  BOOST_CHECK(receivedPackets->at(2).packet == pkt3);
}

BOOST_AUTO_TEST_SUITE_END() // TestTransport
BOOST_AUTO_TEST_SUITE_END() // Face

} // namespace tests
} // namespace face
} // namespace nfd
