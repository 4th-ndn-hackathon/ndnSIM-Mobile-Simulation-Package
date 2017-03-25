/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2015,  Regents of the University of California,
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

#ifndef NFD_CORE_SCHEDULER_HPP
#define NFD_CORE_SCHEDULER_HPP

#include "common.hpp"
#include <ndn-cxx/util/scheduler.hpp>

namespace nfd {
namespace scheduler {

using ndn::Scheduler;

/** \class EventId
 *  \brief Opaque type (shared_ptr) representing ID of a scheduled event
 */
using ndn::EventId;

/** \brief schedule an event
 */
EventId
schedule(const time::nanoseconds& after, const Scheduler::Event& event);

/** \brief cancel a scheduled event
 */
void
cancel(const EventId& eventId);

Scheduler&
getGlobalScheduler();

/** \brief cancels an event automatically upon destruction
 */
class ScopedEventId : noncopyable
{
public:
  ScopedEventId();

  /** \brief implicit constructor from EventId
   *  \param event the event to be cancelled upon destruction
   */
  ScopedEventId(const EventId& event);

  /** \brief move constructor
   */
  ScopedEventId(ScopedEventId&& other);

  /** \brief assigns an event
   *
   *  If a different event has been assigned to this instance previously,
   *  that event will be cancelled immediately.
   */
  ScopedEventId&
  operator=(const EventId& event);

  /** \brief cancels the event
   */
  ~ScopedEventId();

  /** \brief cancels the event manually
   */
  void
  cancel();

  /** \brief releases the event so that it won't be disconnected
   *         when this ScopedEventId is destructed
   */
  void
  release();

private:
  EventId m_event;
};

} // namespace scheduler

} // namespace nfd

#endif // NFD_CORE_SCHEDULER_HPP
