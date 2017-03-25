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

#include "unix-stream-factory.hpp"

#include <boost/filesystem.hpp> // for canonical()

namespace nfd {

shared_ptr<UnixStreamChannel>
UnixStreamFactory::createChannel(const std::string& unixSocketPath)
{
  boost::filesystem::path p(unixSocketPath);
  p = boost::filesystem::canonical(p.parent_path()) / p.filename();
  unix_stream::Endpoint endpoint(p.string());

  auto channel = findChannel(endpoint);
  if (channel)
    return channel;

  channel = make_shared<UnixStreamChannel>(endpoint);
  m_channels[endpoint] = channel;
  return channel;
}

void
UnixStreamFactory::createFace(const FaceUri& uri,
                              ndn::nfd::FacePersistency persistency,
                              bool wantLocalFieldsEnabled,
                              const FaceCreatedCallback& onCreated,
                              const FaceCreationFailedCallback& onFailure)
{
  onFailure(406, "Unsupported protocol");
}

std::vector<shared_ptr<const Channel>>
UnixStreamFactory::getChannels() const
{
  std::vector<shared_ptr<const Channel>> channels;
  channels.reserve(m_channels.size());

  for (const auto& i : m_channels)
    channels.push_back(i.second);

  return channels;
}

shared_ptr<UnixStreamChannel>
UnixStreamFactory::findChannel(const unix_stream::Endpoint& endpoint) const
{
  auto i = m_channels.find(endpoint);
  if (i != m_channels.end())
    return i->second;
  else
    return nullptr;
}

} // namespace nfd
