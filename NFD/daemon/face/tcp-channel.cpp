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

#include "tcp-channel.hpp"
#include "generic-link-service.hpp"
#include "tcp-transport.hpp"
#include "core/global-io.hpp"

namespace nfd {

NFD_LOG_INIT("TcpChannel");

namespace ip = boost::asio::ip;

TcpChannel::TcpChannel(const tcp::Endpoint& localEndpoint)
  : m_localEndpoint(localEndpoint)
  , m_acceptor(getGlobalIoService())
  , m_acceptSocket(getGlobalIoService())
{
  setUri(FaceUri(m_localEndpoint));
}

void
TcpChannel::listen(const FaceCreatedCallback& onFaceCreated,
                   const FaceCreationFailedCallback& onAcceptFailed,
                   int backlog/* = tcp::acceptor::max_connections*/)
{
  if (isListening()) {
    NFD_LOG_WARN("[" << m_localEndpoint << "] Already listening");
    return;
  }

  m_acceptor.open(m_localEndpoint.protocol());
  m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
  if (m_localEndpoint.address().is_v6())
    m_acceptor.set_option(ip::v6_only(true));

  m_acceptor.bind(m_localEndpoint);
  m_acceptor.listen(backlog);

  // start accepting connections
  accept(onFaceCreated, onAcceptFailed);
}

void
TcpChannel::connect(const tcp::Endpoint& remoteEndpoint,
                    bool wantLocalFieldsEnabled,
                    const FaceCreatedCallback& onFaceCreated,
                    const FaceCreationFailedCallback& onConnectFailed,
                    const time::seconds& timeout/* = time::seconds(4)*/)
{
  auto it = m_channelFaces.find(remoteEndpoint);
  if (it != m_channelFaces.end()) {
    onFaceCreated(it->second);
    return;
  }

  auto clientSocket = make_shared<ip::tcp::socket>(ref(getGlobalIoService()));

  scheduler::EventId connectTimeoutEvent = scheduler::schedule(timeout,
    bind(&TcpChannel::handleConnectTimeout, this, clientSocket, onConnectFailed));

  clientSocket->async_connect(remoteEndpoint,
                              bind(&TcpChannel::handleConnect, this,
                                   boost::asio::placeholders::error, clientSocket,
                                   wantLocalFieldsEnabled, connectTimeoutEvent,
                                   onFaceCreated, onConnectFailed));
}

size_t
TcpChannel::size() const
{
  return m_channelFaces.size();
}

void
TcpChannel::createFace(ip::tcp::socket&& socket,
                       bool isOnDemand,
                       bool wantLocalFieldsEnabled,
                       const FaceCreatedCallback& onFaceCreated)
{
  shared_ptr<Face> face;
  tcp::Endpoint remoteEndpoint = socket.remote_endpoint();

  auto it = m_channelFaces.find(remoteEndpoint);
  if (it == m_channelFaces.end()) {
    auto persistency = isOnDemand ? ndn::nfd::FACE_PERSISTENCY_ON_DEMAND
                                  : ndn::nfd::FACE_PERSISTENCY_PERSISTENT;
    auto linkService = make_unique<face::GenericLinkService>();
    auto options = linkService->getOptions();
    options.allowLocalFields = wantLocalFieldsEnabled;
    linkService->setOptions(options);

    auto transport = make_unique<face::TcpTransport>(std::move(socket), persistency);

    face = make_shared<Face>(std::move(linkService), std::move(transport));

    m_channelFaces[remoteEndpoint] = face;
    connectFaceClosedSignal(*face,
      [this, remoteEndpoint] {
        NFD_LOG_TRACE("Erasing " << remoteEndpoint << " from channel face map");
        m_channelFaces.erase(remoteEndpoint);
      });
  }
  else {
    // we already have a face for this endpoint, just reuse it
    face = it->second;

    boost::system::error_code error;
    socket.shutdown(ip::tcp::socket::shutdown_both, error);
    socket.close(error);
  }

  // Need to invoke the callback regardless of whether or not we have already created
  // the face so that control responses and such can be sent.
  onFaceCreated(face);
}

void
TcpChannel::accept(const FaceCreatedCallback& onFaceCreated,
                   const FaceCreationFailedCallback& onAcceptFailed)
{
  m_acceptor.async_accept(m_acceptSocket, bind(&TcpChannel::handleAccept, this,
                                               boost::asio::placeholders::error,
                                               onFaceCreated, onAcceptFailed));
}

void
TcpChannel::handleAccept(const boost::system::error_code& error,
                         const FaceCreatedCallback& onFaceCreated,
                         const FaceCreationFailedCallback& onAcceptFailed)
{
  if (error) {
    if (error == boost::asio::error::operation_aborted) // when the socket is closed by someone
      return;

    NFD_LOG_DEBUG("[" << m_localEndpoint << "] Accept failed: " << error.message());
    if (onAcceptFailed)
      onAcceptFailed(500, "Accept failed: " + error.message());
    return;
  }

  NFD_LOG_DEBUG("[" << m_localEndpoint << "] Connection from " << m_acceptSocket.remote_endpoint());

  createFace(std::move(m_acceptSocket), true, false, onFaceCreated);

  // prepare accepting the next connection
  accept(onFaceCreated, onAcceptFailed);
}

void
TcpChannel::handleConnect(const boost::system::error_code& error,
                          const shared_ptr<ip::tcp::socket>& socket,
                          bool wantLocalFieldsEnabled,
                          const scheduler::EventId& connectTimeoutEvent,
                          const FaceCreatedCallback& onFaceCreated,
                          const FaceCreationFailedCallback& onConnectFailed)
{
  scheduler::cancel(connectTimeoutEvent);

#if (BOOST_VERSION == 105400)
  // To handle regression in Boost 1.54
  // https://svn.boost.org/trac/boost/ticket/8795
  boost::system::error_code anotherErrorCode;
  socket->remote_endpoint(anotherErrorCode);
  if (error || anotherErrorCode) {
#else
  if (error) {
#endif
    if (error == boost::asio::error::operation_aborted) // when the socket is closed by someone
      return;

    NFD_LOG_WARN("[" << m_localEndpoint << "] Connect failed: " << error.message());

    socket->close();

    if (onConnectFailed)
      onConnectFailed(504, "Connect failed: " + error.message());
    return;
  }

  NFD_LOG_DEBUG("[" << m_localEndpoint << "] Connected to " << socket->remote_endpoint());

  createFace(std::move(*socket), false, wantLocalFieldsEnabled, onFaceCreated);
}

void
TcpChannel::handleConnectTimeout(const shared_ptr<ip::tcp::socket>& socket,
                                 const FaceCreationFailedCallback& onConnectFailed)
{
  NFD_LOG_DEBUG("Connect to remote endpoint timed out");

  // abort the connection attempt
  boost::system::error_code error;
  socket->close(error);

  if (onConnectFailed)
    onConnectFailed(504, "Connect to remote endpoint timed out");
}

} // namespace nfd
