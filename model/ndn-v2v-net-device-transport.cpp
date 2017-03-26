#include "ndn-v2v-net-device-transport.hpp"

#include "../helper/ndn-stack-helper.hpp"
#include "ndn-block-header.hpp"
#include "../utils/ndn-ns3-packet-tag.hpp"
#include "ns3/mobility-model.h"


#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/lp/packet.hpp>
#include <vector>
#include "ns3/random-variable-stream.h"


NS_LOG_COMPONENT_DEFINE("ndn.V2VNetDeviceTransport");

namespace ns3 {
namespace ndn {

    V2VNetDeviceTransport::V2VNetDeviceTransport(Ptr<Node> node,
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

      NS_LOG_FUNCTION(this << "Creating an ndnSIM V2V transport instance for netDevice with URI"
                      << this->getLocalUri());

      NS_ASSERT_MSG(m_netDevice != 0, "NetDeviceFace needs to be assigned a valid NetDevice");


      m_node->RegisterProtocolHandler(MakeCallback(&V2VNetDeviceTransport::receiveFromNetDevice, this),
                                      L3Protocol::ETHERNET_FRAME_TYPE, m_netDevice,
                                      true /*promiscuous mode*/);

      m_maxRetxCounter = 3;
    }

    V2VNetDeviceTransport::~V2VNetDeviceTransport()
    {
      NS_LOG_FUNCTION_NOARGS();
    }

    void
    V2VNetDeviceTransport::beforeChangePersistency(::ndn::nfd::FacePersistency newPersistency)
    {
      NS_LOG_FUNCTION(this << "Changing persistency for netDevice with URI"
                      << this->getLocalUri() << "currently does nothing");
      // do nothing for now
    }

    void
    V2VNetDeviceTransport::doClose()
    {
      NS_LOG_FUNCTION(this << "Closing transport for netDevice with URI"
                      << this->getLocalUri());

      // set the state of the transport to "CLOSED"
      this->setState(nfd::face::TransportState::CLOSED);
    }

    void
    V2VNetDeviceTransport::doSend(Packet&& packet)
    {
      NS_LOG_FUNCTION(this << "Sending packet from netDevice with URI"
                      << this->getLocalUri());

      //adding v2v header

      bool isLocal = false;

      Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel> ();
      if (mobility == 0) {
        NS_FATAL_ERROR("Mobility model has to be installed on the node");
        return;
      }
      Vector3D currentPosition = mobility->GetPosition();
      //todo add tag (we may need to remove previous tag if present)
      ::ndn::lp::Packet lpPacket = ::ndn::lp::Packet(packet.packet);

      //::ndn::lp::GeoTag geoCordTag = lpPacket.get<::ndn::lp::GeoTagField>();

      if (!lpPacket.has<::ndn::lp::GeoTagField>()) {
        m_geoTag.setPosX(currentPosition.x);
        m_geoTag.setPosY(currentPosition.y);
        lpPacket.set<::ndn::lp::GeoTagField>(m_geoTag);
        isLocal = true;
      }

      // convert NFD packet to NS3 packet
      BlockHeader header(packet);

      Ptr<ns3::Packet> ns3Packet = Create<ns3::Packet>();
      ns3Packet->AddHeader(header);


      //computing waiting time
      Time waitingTime;
      /*lp::GeoTag previousHopTag;
      if(ns3Packet->PeekPacketTag(previousHopTag)){
        //we are forwarding a packet
        waitingTime = computeWaitingTime(previousHopTag, ns3Packet, false);
      } else {
        //pkt has been generated locally. We will just wait a small random time
        waitingTime = computeWaitingTime(previousHopTag, ns3Packet, true);
      }*/

      //todo: we need to check the hop count (should we do this in the forwarding strategy?)

      // send the NS3 packet (we need to add the packet to the pending pkts queue)

      //add pkt to queue
      Name name = Name(packet.packet.get(::ndn::tlv::Name));

      // let's lookup the queue


      std::tuple<lp::Packet, Name, uint64_t, int> t(lpPacket, name,
        (Simulator::Now() + computeWaitingTime(lpPacket.get<::ndn::lp::GeoTagField>(), ns3Packet, isLocal)).GetMilliSeconds(), 1);



      if(!m_scheduledSend.IsRunning()){
        m_scheduledSend =  Simulator::Schedule(waitingTime, &V2VNetDeviceTransport::SendFromQueue, this);
      }
    }

    // callback
    void
    V2VNetDeviceTransport::receiveFromNetDevice(Ptr<NetDevice> device,
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


      //TODO if needed, extracts previous hop coordinates and create a new tag; check if the packet acks (with push progress) any of the pending packets (interest/interest - content/content - interest/content). We need prefix matching here. (remove pending packet if needed)
      //pass the pkt to the upper layer

      auto nfdPacket = Packet(std::move(header.getBlock()));

      this->receive(std::move(nfdPacket));
    }

    Ptr<NetDevice>
    V2VNetDeviceTransport::GetNetDevice() const
    {
      return m_netDevice;
    }



    /*void
    sendPacketWithDelay(Ptr<ns3::Packet> packet, Time delay)
    {

      m_netDevice->Send(ns3Packet, m_netDevice->GetBroadcast(),
                        L3Protocol::ETHERNET_FRAME_TYPE);

      Simulator::Schedule(delay, &V2VNetDeviceTransport::SendFromQueue, this);
    }
    */

    void
    V2VNetDeviceTransport::SendFromQueue()
    {
      //todo
      //look at the queue and send pkt that have an expired transmission time
      //increment retx counter and remove pkt with retx counter > m_maxRetxCounter;

      //schedule next SendFromQueue
    }


    Time
    V2VNetDeviceTransport::computeWaitingTime(lp::GeoTag previousHop, Ptr<ns3::Packet> Packet, bool isLocal)
    {
            double maxRandomTimer = 0.002; // HARD CORDED FOR NOW
            UniformRandomVariable randomVariable;
            Time randomTimer = Seconds(randomVariable.GetValue(0, maxRandomTimer));

            if(isLocal) {
                    return randomTimer;
            }

            // compute waiting time according to its own position and last hop's position

            // last hop position
            Vector3D lastHopPosition = Vector3D(previousHop.getPosX(), previousHop.getPosY(), 0);

            // own position
            Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel> ();
            if (mobility == 0) {
                    NS_FATAL_ERROR("Mobility model has to be installed on the node");
                    return randomTimer;
            }
            Vector3D currentPosition = mobility->GetPosition();

            // calculate distance
            double distToLastHop = CalculateDistance(lastHopPosition, currentPosition);
            if (distToLastHop < 0) {
                    NS_FATAL_ERROR("Mobility model is not valid");
                    return randomTimer;
            }

            Time waitingTimer;
            double minDelay = 0.005, maxDist = 150; // HARD CODED FOR NOW according to the "Rapid ... "" paper

            if (distToLastHop < maxDist) {
                    waitingTimer = Seconds(((maxDist - distToLastHop) / maxDist) * minDelay);
            }
            else {
                    NS_LOG_INFO("Tranmission distance is longer than max distance");
                    return randomTimer;
            }

            return waitingTimer;
    }


} // namespace ndn
} // namespace ns3
