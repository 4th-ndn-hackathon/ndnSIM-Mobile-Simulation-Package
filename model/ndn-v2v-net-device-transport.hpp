#ifndef NDN_V2V_NET_DEVICE_TRANSPORT_HPP
#define NDN_V2V_NET_DEVICE_TRANSPORT_HPP

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/NFD/daemon/face/transport.hpp"
#include "ns3/ndnSIM/ndn-cxx/lp/geo-tag.hpp"

#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/pointer.h"

#include "ns3/point-to-point-net-device.h"
#include "ns3/channel.h"
namespace ns3 {
  namespace ndn {
    
    /**
     * \ingroup ndn-face
     * \brief ndnSIM-specific V2V transport
     */
    class V2VNetDeviceTransport : public nfd::face::Transport
    {
    public:
      V2VNetDeviceTransport(Ptr<Node> node, const Ptr<NetDevice>& netDevice,
                         const std::string& localUri,
                         const std::string& remoteUri,
                         ::ndn::nfd::FaceScope scope = ::ndn::nfd::FACE_SCOPE_NON_LOCAL,
                         ::ndn::nfd::FacePersistency persistency = ::ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
                         ::ndn::nfd::LinkType linkType = ::ndn::nfd::LINK_TYPE_POINT_TO_POINT);
      
      ~V2VNetDeviceTransport();
      
      Ptr<NetDevice>
      GetNetDevice() const;
      
    private:
      virtual void
      beforeChangePersistency(::ndn::nfd::FacePersistency newPersistency) override;
      
      virtual void
      doClose() override;
      
      virtual void
      doSend(Packet&& packet) override;
      
      void
      receiveFromNetDevice(Ptr<NetDevice> device,
                           Ptr<const ns3::Packet> p,
                           uint16_t protocol,
                           const Address& from, const Address& to,
                           NetDevice::PacketType packetType);
      
      virtual Time
      computeWaitingTime(lp::GeoTag previousHop, Ptr<ns3::Packet> Packet, bool isLocal);
      
      
      //virtual void
      //retransmitPacket(Ptr<ns3::Packet> packet);
      
      virtual void
      SendFromQueue();
      
      Ptr<NetDevice> m_netDevice; ///< \brief Smart pointer to NetDevice
      Ptr<Node> m_node;
      
      EventId m_scheduledSend;
      
      int m_maxRetxCounter;
    };
    
    
    
    class Item
    {
      public:
        Ptr<ns3::Packet> packet;
      Time nextTranmissionTime;
      uint32_t m_retxCount;

      
    };
  } // namespace ndn
} // namespace ns3

#endif // NDN_V2V_NET_DEVICE_TRANSPORT_HPP
