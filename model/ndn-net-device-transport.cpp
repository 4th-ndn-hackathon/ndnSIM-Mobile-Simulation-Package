/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2016  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-net-device-transport.hpp"

#include "../helper/ndn-stack-helper.hpp"
#include "ndn-block-header.hpp"
#include "../utils/ndn-ns3-packet-tag.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include "ns3/wifi-net-device.h"
#include "ns3/sta-wifi-mac.h"

NS_LOG_COMPONENT_DEFINE("ndn.NetDeviceTransport");

namespace ns3 {
namespace ndn {

NetDeviceTransport::NetDeviceTransport(Ptr<Node> node,
                                       const Ptr<NetDevice>& netDevice,
                                       const std::string& localUri,
                                       const std::string& remoteUri,
                                       ::ndn::nfd::FaceScope scope,
                                       ::ndn::nfd::FacePersistency persistency,
                                       ::ndn::nfd::LinkType linkType)
  : m_netDevice(netDevice)
  , m_node(node)
{
  this->setLocalUri(FaceUri(localUri));
  this->setRemoteUri(FaceUri(remoteUri));
  this->setScope(scope);
  this->setPersistency(persistency);
  this->setLinkType(linkType);
  // this->setMtu(udp::computeMtu(m_socket.local_endpoint())); // not sure what should be here

  NS_LOG_FUNCTION(this << "Creating an ndnSIM transport instance for netDevice with URI"
                  << this->getLocalUri());

  NS_ASSERT_MSG(m_netDevice != 0, "NetDeviceFace needs to be assigned a valid NetDevice");

  m_node->RegisterProtocolHandler(MakeCallback(&NetDeviceTransport::receiveFromNetDevice, this),
                                  L3Protocol::ETHERNET_FRAME_TYPE, m_netDevice,
                                  true /*promiscuous mode*/);
}

NetDeviceTransport::~NetDeviceTransport()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
NetDeviceTransport::beforeChangePersistency(::ndn::nfd::FacePersistency newPersistency)
{
  NS_LOG_FUNCTION(this << "Changing persistency for netDevice with URI"
                  << this->getLocalUri() << "currently does nothing");
  // do nothing for now
}

void
NetDeviceTransport::doClose()
{
  NS_LOG_FUNCTION(this << "Closing transport for netDevice with URI"
                  << this->getLocalUri());

  // set the state of the transport to "CLOSED"
  this->setState(nfd::face::TransportState::CLOSED);
}

void
NetDeviceTransport::doSend(Packet&& packet)
{
  NS_LOG_FUNCTION(this << "Sending packet from netDevice with URI"
                  << this->getLocalUri());

  // convert NFD packet to NS3 packet
  BlockHeader header(packet);

  Ptr<ns3::Packet> ns3Packet = Create<ns3::Packet>();
  ns3Packet->AddHeader(header);

  // send the NS3 packet
  Address dest = m_netDevice->GetBroadcast();
  Ptr<ns3::WifiNetDevice> wifiDev = m_netDevice->GetObject<ns3::WifiNetDevice>();
  if (wifiDev != nullptr) {
    Ptr<ns3::StaWifiMac> staMac = wifiDev->GetMac()->GetObject<ns3::StaWifiMac>();
    if (staMac != nullptr) {
      dest = staMac->GetBssid();
    }
  }
  m_netDevice->Send(ns3Packet, dest,
                    L3Protocol::ETHERNET_FRAME_TYPE);
}

// callback
void
NetDeviceTransport::receiveFromNetDevice(Ptr<NetDevice> device,
                                      Ptr<const ns3::Packet> p,
                                      uint16_t protocol,
                                      const Address& from, const Address& to,
                                      NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION(device << p << protocol << from << to << packetType);

  // Convert NS3 packet to NFD packet
  Ptr<ns3::Packet> packet = p->Copy();

  BlockHeader header;
  packet->RemoveHeader(header);

  auto nfdPacket = Packet(std::move(header.getBlock()));

  this->receive(std::move(nfdPacket));
}

Ptr<NetDevice>
NetDeviceTransport::GetNetDevice() const
{
  return m_netDevice;
}

} // namespace ndn
} // namespace ns3
