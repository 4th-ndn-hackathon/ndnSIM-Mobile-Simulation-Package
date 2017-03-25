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

#include "tcp-transport.hpp"

namespace nfd {
namespace face {

NFD_LOG_INCLASS_TEMPLATE_SPECIALIZATION_DEFINE(StreamTransport, TcpTransport::protocol, "TcpTransport");

time::milliseconds TcpTransport::s_initialReconnectWait = time::seconds(1);
time::milliseconds TcpTransport::s_maxReconnectWait = time::minutes(5);
float TcpTransport::s_reconnectWaitMultiplier = 2.0f;

TcpTransport::TcpTransport(protocol::socket&& socket, ndn::nfd::FacePersistency persistency)
  : StreamTransport(std::move(socket))
  , m_remoteEndpoint(m_socket.remote_endpoint())
  , m_nextReconnectWait(s_initialReconnectWait)
{
  this->setLocalUri(FaceUri(m_socket.local_endpoint()));
  this->setRemoteUri(FaceUri(m_socket.remote_endpoint()));

  if (m_socket.local_endpoint().address().is_loopback() &&
      m_socket.remote_endpoint().address().is_loopback())
    this->setScope(ndn::nfd::FACE_SCOPE_LOCAL);
  else
    this->setScope(ndn::nfd::FACE_SCOPE_NON_LOCAL);

  this->setPersistency(persistency);
  this->setLinkType(ndn::nfd::LINK_TYPE_POINT_TO_POINT);
  this->setMtu(MTU_UNLIMITED);

  NFD_LOG_FACE_INFO("Creating transport");
}

void
TcpTransport::beforeChangePersistency(ndn::nfd::FacePersistency newPersistency)
{
  // if persistency is changing from permanent to any other value
  if (this->getPersistency() == ndn::nfd::FACE_PERSISTENCY_PERMANENT) {
    if (this->getState() == TransportState::DOWN) {
      // non-permanent transport cannot be in DOWN state, so fail hard
      this->setState(TransportState::FAILED);
      doClose();
    }
  }
}

void
TcpTransport::handleError(const boost::system::error_code& error)
{
  if (this->getPersistency() == ndn::nfd::FACE_PERSISTENCY_PERMANENT) {
    NFD_LOG_FACE_TRACE("TCP socket error: " << error.message());
    this->setState(TransportState::DOWN);

    // cancel all outstanding operations
    boost::system::error_code error;
    m_socket.cancel(error);

    // do this asynchronously because there could be some callbacks still pending
    getGlobalIoService().post([this] { reconnect(); });
  }
  else {
    StreamTransport::handleError(error);
  }
}

void
TcpTransport::reconnect()
{
  NFD_LOG_FACE_TRACE(__func__);

  if (getState() == TransportState::CLOSING ||
      getState() == TransportState::FAILED ||
      getState() == TransportState::CLOSED) {
    // transport is shutting down, don't attempt to reconnect
    return;
  }

  BOOST_ASSERT(getPersistency() == ndn::nfd::FACE_PERSISTENCY_PERMANENT);
  BOOST_ASSERT(getState() == TransportState::DOWN);

  // recreate the socket
  m_socket = protocol::socket(m_socket.get_io_service());
  this->resetReceiveBuffer();
  this->resetSendQueue();

  m_reconnectEvent = scheduler::schedule(m_nextReconnectWait,
                                         [this] { handleReconnectTimeout(); });
  m_socket.async_connect(m_remoteEndpoint,
                         [this] (const boost::system::error_code& error) { handleReconnect(error); });
}

void
TcpTransport::handleReconnect(const boost::system::error_code& error)
{
  if (getState() == TransportState::CLOSING ||
      getState() == TransportState::FAILED ||
      getState() == TransportState::CLOSED ||
      error == boost::asio::error::operation_aborted) {
    // transport is shutting down, abort the reconnection attempt and ignore any errors
    return;
  }

  if (error) {
    NFD_LOG_FACE_TRACE("Reconnection attempt failed: " << error.message());
    return;
  }

  m_reconnectEvent.cancel();
  m_nextReconnectWait = s_initialReconnectWait;

  this->setLocalUri(FaceUri(m_socket.local_endpoint()));
  NFD_LOG_FACE_TRACE("TCP connection reestablished");
  this->setState(TransportState::UP);
  this->startReceive();
}

void
TcpTransport::handleReconnectTimeout()
{
  // abort the reconnection attempt
  boost::system::error_code error;
  m_socket.close(error);

  // exponentially back off the reconnection timer
  m_nextReconnectWait =
      std::min(time::duration_cast<time::milliseconds>(m_nextReconnectWait * s_reconnectWaitMultiplier),
               s_maxReconnectWait);

  // do this asynchronously because there could be some callbacks still pending
  getGlobalIoService().post([this] { reconnect(); });
}

void
TcpTransport::doClose()
{
  m_reconnectEvent.cancel();
  StreamTransport::doClose();
}

} // namespace face
} // namespace nfd
