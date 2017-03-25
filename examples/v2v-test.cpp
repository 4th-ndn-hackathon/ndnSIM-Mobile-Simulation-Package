#include "ns3/core-module.h"

#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>

#include <algorithm>
#include <vector>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("V2VTest");


static const uint32_t numNodes = 6;
  
void printPosition(Ptr<const MobilityModel> mobility) //DEBUG purpose
{
  Simulator::Schedule(Seconds(1), &printPosition, mobility);
  NS_LOG_INFO("Car "<<  mobility->GetObject<Node>()->GetId() << " is at: " <<mobility->GetPosition());
}

  
void installMobility(NodeContainer &c, int simulationEnd)
{
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::WaypointMobilityModel");
  mobility.Install(c);

  bool test = true;
  if(test){
    Ptr<WaypointMobilityModel> wayMobility[numNodes];
    for (uint32_t i = 0; i < numNodes; i++) {
      wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
      Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
      Waypoint waypointEnd(Seconds(simulationEnd), Vector3D(i*10, 0, 0));
      
      wayMobility[i]->AddWaypoint(waypointStart);
      wayMobility[i]->AddWaypoint(waypointEnd);
      NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointEnd);
    }

    
    return;
  }
  Ptr<WaypointMobilityModel> wayMobility[numNodes];
  for (uint32_t i = 0; i < numNodes; i++) {
    wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
    Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
    Waypoint waypointMiddle(Seconds(simulationEnd/2), Vector3D(i*20+1000, 0, 0));
    Waypoint waypointEnd(Seconds(simulationEnd+1), Vector3D(i*20+1000, 0, 0));

    wayMobility[i]->AddWaypoint(waypointStart);
    wayMobility[i]->AddWaypoint(waypointMiddle);
    wayMobility[i]->AddWaypoint(waypointEnd);
    NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointMiddle << " " << waypointEnd);
  }
  

  
}


void installWifi(NodeContainer &c, NetDeviceContainer &devices)
{
  // Modulation and wifi channel bit rate
  std::string phyMode("OfdmRate24Mbps");
  
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
  
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                 "MaxRange", DoubleValue(19.0));
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                 "m0", DoubleValue(1.0),
                                 "m1", DoubleValue(1.0),
                                 "m2", DoubleValue(1.0));
  wifiPhy.SetChannel(wifiChannel.Create());
  
  // Add a non-QoS upper mac
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
  // Set it to adhoc mode
  wifiMac.SetType("ns3::AdhocWifiMac");
  
  // Disable rate control
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));
  
  devices = wifi.Install(wifiPhy, wifiMac, c);
}

void installNDN(NodeContainer &c)
{
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);

  ndnHelper.Install(c);
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/broadcast");

  ///todo add v2v face
  

}

void installConsumer(NodeContainer &c)
{
  ndn::AppHelper helper("ns3::ndn::ConsumerCbr");
  helper.SetAttribute("Frequency", DoubleValue (1.0));
  helper.SetAttribute("Randomize", StringValue("uniform"));
  helper.SetPrefix("/v2v/test");

  helper.Install(c);
}

void installProducer(NodeContainer &c)
{
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/v2v");

  producerHelper.Install(c.Get(0));
  NS_LOG_INFO("Producer installed on node " << c.Get(0)->GetId());

}

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("V2VTest Simulator");
  
  uint32_t numProducer = 1;
  int simulationEnd = 10;
  
  NodeContainer c;
  c.Create(numNodes);
  
  installMobility(c, simulationEnd);
  
  NetDeviceContainer netDevices;
  installWifi(c, netDevices);
  
  installNDN(c);

  //setting application
  Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  int producerId = randomNum->GetValue(0,numNodes-1);

  NodeContainer producer;
  producer.Add(c.Get(producerId));
  
  NodeContainer consumers;
  for(int i=0; i<numNodes; i++){
    if(i!=producerId){
      consumers.Add(c.Get(i));
      break;//tmp
    }
  }
  
  installConsumer(consumers);
  installProducer(producer);
  
  
  
  for(int i=0; i<c.GetN(); i++){
    Simulator::Schedule(Seconds(1), &printPosition, c.Get(i)->GetObject<WaypointMobilityModel>());
  }
  
  Simulator::Stop(Seconds(simulationEnd));

  
  Simulator::Run ();
  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}