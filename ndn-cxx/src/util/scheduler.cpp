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

#include "scheduler.hpp"
#include <boost/scope_exit.hpp>

namespace ns3 {

/// @cond include_hidden

template<>
struct EventMemberImplObjTraits<std::function<void()>> {
  typedef std::function<void()> T;
  static T&
  GetReference(T& p)
  {
    return p;
  }
};

/// @endcond

} // namespace ns3

namespace ndn {
namespace util {
namespace scheduler {

Scheduler::Scheduler(boost::asio::io_service& ioService)
  : m_scheduledEvent(m_events.end())
{
}

Scheduler::~Scheduler()
{
  cancelAllEvents();
}

EventId
Scheduler::scheduleEvent(const time::nanoseconds& after, const Event& event)
{
  EventId eventId = std::make_shared<ns3::EventId>();
  weak_ptr<ns3::EventId> eventWeak = eventId;
  std::function<void()> eventWithCleanup = [this, event, eventWeak] () {
    event();
    shared_ptr<ns3::EventId> eventId = eventWeak.lock();
    if (eventId != nullptr) {
      this->m_events.erase(eventId); // remove the event from the set after it is executed
    }
  };

  ns3::EventId id = ns3::Simulator::Schedule(ns3::NanoSeconds(after.count()),
                                             &std::function<void()>::operator(), eventWithCleanup);
  *eventId = std::move(id);
  m_events.insert(eventId);

  return eventId;
}

void
Scheduler::cancelEvent(const EventId& eventId)
{
  if (eventId != nullptr) {
    ns3::Simulator::Remove(*eventId);
    const_cast<EventId&>(eventId).reset();
    m_events.erase(eventId);
  }
}

void
Scheduler::cancelAllEvents()
{
  for (auto i = m_events.begin(); i != m_events.end(); ) {
    auto next = i;
    ++next; // ns3::Simulator::Remove can call cancelEvent
    if ((*i) != nullptr) {
      ns3::Simulator::Remove((**i));
      const_cast<EventId&>(*i).reset();
    }
    i = next;
  }
  m_events.clear();
}

} // namespace scheduler
} // namespace util
} // namespace ndn
