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

#ifndef NDN_TRANSPORT_TCP_TRANSPORT_HPP
#define NDN_TRANSPORT_TCP_TRANSPORT_HPP

#include "transport.hpp"
#include "../util/config-file.hpp"

namespace boost {
namespace asio {
namespace ip {
class tcp;
} // namespace ip
} // namespace asio
} // namespace boost

namespace ndn {

template<typename BaseTransport, typename Protocol>
class StreamTransportImpl;

template<typename BaseTransport, typename Protocol>
class StreamTransportWithResolverImpl;

/** \brief a transport using TCP socket
 */
class TcpTransport : public Transport
{
public:
  explicit
  TcpTransport(const std::string& host, const std::string& port = "6363");

  ~TcpTransport();

  virtual void
  connect(boost::asio::io_service& ioService,
          const ReceiveCallback& receiveCallback) override;

  virtual void
  close() override;

  virtual void
  pause() override;

  virtual void
  resume() override;

  virtual void
  send(const Block& wire) override;

  virtual void
  send(const Block& header, const Block& payload) override;

  /** \brief Create transport with parameters defined in URI
   *  \throw Transport::Error incorrect URI or unsupported protocol is specified
   */
  static shared_ptr<TcpTransport>
  create(const std::string& uri);

NDN_CXX_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static std::pair<std::string, std::string>
  getSocketHostAndPortFromUri(const std::string& uri);

private:
  std::string m_host;
  std::string m_port;

  typedef StreamTransportWithResolverImpl<TcpTransport, boost::asio::ip::tcp> Impl;
  friend class StreamTransportImpl<TcpTransport, boost::asio::ip::tcp>;
  friend class StreamTransportWithResolverImpl<TcpTransport, boost::asio::ip::tcp>;
  shared_ptr<Impl> m_impl;
};

} // namespace ndn

#endif // NDN_TRANSPORT_TCP_TRANSPORT_HPP
