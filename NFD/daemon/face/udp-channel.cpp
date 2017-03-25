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

#include "udp-channel.hpp"
#include "generic-link-service.hpp"
#include "unicast-udp-transport.hpp"
#include "core/global-io.hpp"

namespace nfd {

NFD_LOG_INIT("UdpChannel");

using namespace boost::asio;

UdpChannel::UdpChannel(const udp::Endpoint& localEndpoint,
                       const time::seconds& timeout)
  : m_localEndpoint(localEndpoint)
  , m_socket(getGlobalIoService())
  , m_idleFaceTimeout(timeout)
{
  setUri(FaceUri(m_localEndpoint));
}

size_t
UdpChannel::size() const
{
  return m_channelFaces.size();
}

void
UdpChannel::connect(const udp::Endpoint& remoteEndpoint,
                    ndn::nfd::FacePersistency persistency,
                    const FaceCreatedCallback& onFaceCreated,
                    const FaceCreationFailedCallback& onConnectFailed)
{
  shared_ptr<Face> face;
  try {
    face = createFace(remoteEndpoint, persistency).second;
  }
  catch (const boost::system::system_error& e) {
    NFD_LOG_WARN("[" << m_localEndpoint << "] Connect failed: " << e.what());
    if (onConnectFailed)
      onConnectFailed(504, std::string("Connect failed: ") + e.what());
    return;
  }

  // Need to invoke the callback regardless of whether or not we had already
  // created the face so that control responses and such can be sent
  onFaceCreated(face);
}

void
UdpChannel::listen(const FaceCreatedCallback& onFaceCreated,
                   const FaceCreationFailedCallback& onReceiveFailed)
{
  if (isListening()) {
    NFD_LOG_WARN("[" << m_localEndpoint << "] Already listening");
    return;
  }

  m_socket.open(m_localEndpoint.protocol());
  m_socket.set_option(ip::udp::socket::reuse_address(true));
  if (m_localEndpoint.address().is_v6())
    m_socket.set_option(ip::v6_only(true));

  m_socket.bind(m_localEndpoint);
  this->waitForNewPeer(onFaceCreated, onReceiveFailed);
}

void
UdpChannel::waitForNewPeer(const FaceCreatedCallback& onFaceCreated,
                           const FaceCreationFailedCallback& onReceiveFailed)
{
  m_socket.async_receive_from(boost::asio::buffer(m_inputBuffer, ndn::MAX_NDN_PACKET_SIZE),
                              m_remoteEndpoint,
                              bind(&UdpChannel::handleNewPeer, this,
                                   boost::asio::placeholders::error,
                                   boost::asio::placeholders::bytes_transferred,
                                   onFaceCreated, onReceiveFailed));
}

void
UdpChannel::handleNewPeer(const boost::system::error_code& error,
                          size_t nBytesReceived,
                          const FaceCreatedCallback& onFaceCreated,
                          const FaceCreationFailedCallback& onReceiveFailed)
{
  if (error) {
    if (error == boost::asio::error::operation_aborted) // when the socket is closed by someone
      return;

    NFD_LOG_DEBUG("[" << m_localEndpoint << "] Receive failed: " << error.message());
    if (onReceiveFailed)
      onReceiveFailed(500, "Receive failed: " + error.message());
    return;
  }

  NFD_LOG_DEBUG("[" << m_localEndpoint << "] New peer " << m_remoteEndpoint);

  bool isCreated = false;
  shared_ptr<Face> face;
  try {
    std::tie(isCreated, face) = createFace(m_remoteEndpoint, ndn::nfd::FACE_PERSISTENCY_ON_DEMAND);
  }
  catch (const boost::system::system_error& e) {
    NFD_LOG_WARN("[" << m_localEndpoint << "] Failed to create face for peer "
                 << m_remoteEndpoint << ": " << e.what());
    if (onReceiveFailed)
      onReceiveFailed(504, "Failed to create face for peer");
    return;
  }

  if (isCreated)
    onFaceCreated(face);

  // dispatch the datagram to the face for processing
  static_cast<face::UnicastUdpTransport*>(face->getTransport())->receiveDatagram(m_inputBuffer, nBytesReceived, error);

  this->waitForNewPeer(onFaceCreated, onReceiveFailed);
}

std::pair<bool, shared_ptr<Face>>
UdpChannel::createFace(const udp::Endpoint& remoteEndpoint, ndn::nfd::FacePersistency persistency)
{
  auto it = m_channelFaces.find(remoteEndpoint);
  if (it != m_channelFaces.end()) {
    // we already have a face for this endpoint, so reuse it
    auto face = it->second;

    // TODO #3232: Remove persistency transitions from faces/create
    // only on-demand -> persistent -> permanent transition is allowed
    bool isTransitionAllowed = persistency != face->getPersistency() &&
                               (face->getPersistency() == ndn::nfd::FACE_PERSISTENCY_ON_DEMAND ||
                                persistency == ndn::nfd::FACE_PERSISTENCY_PERMANENT);
    if (isTransitionAllowed) {
      face->setPersistency(persistency);
    }
    return {false, face};
  }

  // else, create a new face
  ip::udp::socket socket(getGlobalIoService(), m_localEndpoint.protocol());
  socket.set_option(ip::udp::socket::reuse_address(true));
  socket.bind(m_localEndpoint);
  socket.connect(remoteEndpoint);

  auto linkService = make_unique<face::GenericLinkService>();
  auto transport = make_unique<face::UnicastUdpTransport>(std::move(socket), persistency, m_idleFaceTimeout);
  auto face = make_shared<Face>(std::move(linkService), std::move(transport));

  face->setPersistency(persistency);

  m_channelFaces[remoteEndpoint] = face;
  connectFaceClosedSignal(*face,
    [this, remoteEndpoint] {
      NFD_LOG_TRACE("Erasing " << remoteEndpoint << " from channel face map");
      m_channelFaces.erase(remoteEndpoint);
    });

  return {true, face};
}

} // namespace nfd
