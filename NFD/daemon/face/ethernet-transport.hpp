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

#ifndef NFD_DAEMON_FACE_ETHERNET_TRANSPORT_HPP
#define NFD_DAEMON_FACE_ETHERNET_TRANSPORT_HPP

#include "core/common.hpp"
#include "transport.hpp"
#include "core/network-interface.hpp"

#ifndef HAVE_LIBPCAP
#error "Cannot include this file when libpcap is not available"
#endif

// forward declarations
struct pcap;
typedef pcap pcap_t;
struct pcap_pkthdr;

namespace nfd {
namespace face {

/**
 * \brief A multicast Transport that uses raw Ethernet II frames
 */
class EthernetTransport final : public Transport
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  /**
   * @brief Creates an Ethernet-based transport for multicast communication
   */
  EthernetTransport(const NetworkInterfaceInfo& interface,
                    const ethernet::Address& mcastAddress);

protected:
  virtual void
  beforeChangePersistency(ndn::nfd::FacePersistency newPersistency) final;

  virtual void
  doClose() final;

private:
  virtual void
  doSend(Transport::Packet&& packet) final;

  /**
   * @brief Allocates and initializes a libpcap context for live capture
   */
  void
  pcapInit();

  /**
   * @brief Installs a BPF filter on the receiving socket
   *
   * @param filterString string containing the source BPF program
   */
  void
  setPacketFilter(const char* filterString);

  /**
   * @brief Enables receiving frames addressed to our MAC multicast group
   *
   * @return true if successful, false otherwise
   */
  bool
  joinMulticastGroup();

  /**
   * @brief Sends the specified TLV block on the network wrapped in an Ethernet frame
   */
  void
  sendPacket(const ndn::Block& block);

  /**
   * @brief Receive callback
   */
  void
  handleRead(const boost::system::error_code& error, size_t nBytesRead);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /**
   * @brief Processes an incoming frame as captured by libpcap
   *
   * @param header pointer to capture metadata
   * @param packet pointer to the received frame, including the link-layer header
   */
  void
  processIncomingPacket(const pcap_pkthdr* header, const uint8_t* packet);

private:
  /**
   * @brief Handles errors encountered by Boost.Asio on the receive path
   */
  void
  processErrorCode(const boost::system::error_code& error);

  /**
   * @brief Returns the MTU of the underlying network interface
   */
  size_t
  getInterfaceMtu();

private:
  unique_ptr<pcap_t, void(*)(pcap_t*)> m_pcap;
  boost::asio::posix::stream_descriptor m_socket;

  ethernet::Address m_srcAddress;
  ethernet::Address m_destAddress;
  std::string m_interfaceName;
#if defined(__linux__)
  int m_interfaceIndex;
#endif

#ifdef _DEBUG
  /// number of packets dropped by the kernel, as reported by libpcap
  unsigned int m_nDropped;
#endif
};

} // namespace face
} // namespace nfd

#endif // NFD_DAEMON_FACE_ETHERNET_TRANSPORT_HPP
