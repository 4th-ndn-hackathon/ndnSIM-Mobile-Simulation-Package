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

#ifndef NDN_UTIL_SCHEDULER_HPP
#define NDN_UTIL_SCHEDULER_HPP

#include "../common.hpp"
#include "monotonic_deadline_timer.hpp"

#include "ns3/simulator.h"

#include <set>

namespace ndn {
namespace util {
namespace scheduler {

/** \class EventId
 *  \brief Opaque type (shared_ptr) representing ID of a scheduled event
 */
typedef std::shared_ptr<ns3::EventId> EventId;

/**
 * \brief Generic scheduler
 */
class Scheduler : noncopyable
{
public:
  /**
   * \deprecated use EventCallback
   */
  typedef function<void()> Event;

  explicit
  Scheduler(boost::asio::io_service& ioService);

  ~Scheduler();

  /**
   * \brief Schedule a one-time event after the specified delay
   * \return EventId that can be used to cancel the scheduled event
   */
  EventId
  scheduleEvent(const time::nanoseconds& after, const Event& event);

  /**
   * \brief Cancel a scheduled event
   */
  void
  cancelEvent(const EventId& eventId);

  /**
   * \brief Cancel all scheduled events
   */
  void
  cancelAllEvents();

private:
  struct EventInfo
  {
    EventInfo(const time::nanoseconds& after, const Event& event);

    EventInfo(const time::steady_clock::TimePoint& when, const EventInfo& previousEvent);

    bool
    operator <=(const EventInfo& other) const
    {
      return this->m_scheduledTime <= other.m_scheduledTime;
    }

    bool
    operator <(const EventInfo& other) const
    {
      return this->m_scheduledTime < other.m_scheduledTime;
    }

    time::nanoseconds
    expiresFromNow() const;

    time::steady_clock::TimePoint m_scheduledTime;
    Event m_event;
    mutable EventId m_eventId;
  };

  typedef std::multiset<EventId> EventQueue;

  EventQueue m_events;
  EventQueue::iterator m_scheduledEvent;
};

} // namespace scheduler

using util::scheduler::Scheduler;

} // namespace util

// for backwards compatibility
using util::scheduler::Scheduler;
using util::scheduler::EventId;

} // namespace ndn

#endif // NDN_UTIL_SCHEDULER_HPP
