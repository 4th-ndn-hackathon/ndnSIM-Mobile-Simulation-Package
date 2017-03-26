/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Md Ashiqur Rahman: University of Arizona.
 *
 **/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-velocity-mobility-model.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


NS_LOG_COMPONENT_DEFINE ("simple-wifi-mobility");

using namespace std;

namespace ns3 {

/**
 * DESCRIPTION: 
 * This scenario provides a basic Wifi AP (as infrastucture) and two mobile STA nodes
 * which do active association to an AP node when within the AP's range. Association
 * is done by fetching the Bssid of current AP. After association, mobile STA will
 * send unicast unicast interests to AP which reaches the producer ans fetches data back.
 * Data is broadcast by AP, so other STA asking for same data will also receive. A global
 * Routing is used, as Default routing doesn't forward interest from AP to producer. This
 * is expected to be fixed later.
 *
 * The scenario simulates a tree topology (using topology reader module)
 *
 *                                    /--------\
 *                           +------->|  root  |<--------+
 *                           |        \--------/         |    10Mbps 100ms
 *                           |                           |
 *                           v                           v
 *                      /-------\                    /-------\
 *              +------>| rtr-4 |<-------+   +------>| rtr-5 |<--------+
 *              |       \-------/        |   |       \-------/         |
 *              |                        |   |                         |   10Mbps 50ms
 *              v                        v   v                         v
 *         /-------\                   /-------\                    /-------\
 *      +->| rtr-1 |<-+             +->| rtr-2 |<-+              +->| rtr-3 |<-+
 *      |  \-------/  |             |  \-------/  |              |  \-------/  |
 *      |             |             |             |              |             | 10Mbps 2ms
 *      v             v             v             v              v             v
 *   /------\      /------\      /------\      /------\      /------\      /------\
 *   |wifi-1|      |wifi-2|      |wifi-3|      |wifi-4|      |wifi-5|      |wifi-6|
 *   \------/      \------/      \------/      \------/      \------/      \------/
 *
 *
 *  |m2|-->     |m1|--->
 *
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     ./waf --run=simple-wifi-mobility
 * 
 * With LOGGING: e.g.
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=simple-wifi-mobility 2>&1 | tee log.txt
 */

  int main (int argc, char *argv[])
  {
    std::string phyMode ("DsssRate1Mbps");
    uint32_t wifiSta = 2;

    int bottomrow = 6;            // number of AP nodes
    int spacing = 200;            // between bottom-row nodes
    int range = 110;
    double endtime = 20.0;
    double speed = (double)(bottomrow*spacing)/endtime; //setting speed to span full sim time 

    string animFile = "ap-mobility-animation.xml";

    CommandLine cmd;
    cmd.AddValue ("animFile", "File Name for Animation Output", animFile);
    cmd.Parse (argc, argv);
    
    ////// Reading file for topology setup
    AnnotatedTopologyReader topologyReader("", 1);
    topologyReader.SetFileName("src/ndnSIM/examples/x-topo.txt");
    topologyReader.Read();

    ////// Getting containers for the producer/wifi-ap
    Ptr<Node> producer = Names::Find<Node>("root");
    Ptr<Node> wifiApNodes[6] = {Names::Find<Node>("ap1"), 
                                Names::Find<Node>("ap2"),
                                Names::Find<Node>("ap3"),
                                Names::Find<Node>("ap4"),
                                Names::Find<Node>("ap5"),
                                Names::Find<Node>("ap6")};
    
    Ptr<Node> routers[5] = {Names::Find<Node>("r1"), 
                                Names::Find<Node>("r2"),
                                Names::Find<Node>("r3"),
                                Names::Find<Node>("r4"),
                                Names::Find<Node>("r5"),};

    ////// disable fragmentation, RTS/CTS for frames below 2200 bytes and fix non-unicast data rate
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    ////// The below set of helpers will help us to put together the wifi NICs we want 
    WifiHelper wifi;

    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

    ////// This is one parameter that matters when using FixedRssLossModel
    ////// set it to zero; otherwise, gain will be added
    // wifiPhy.Set ("RxGain", DoubleValue (0) );

    ////// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    // wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

    ////// The below FixedRssLossModel will cause the rss to be fixed regardless
    ////// of the distance between the two stations, and the transmit power
    // wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue(rss));

    ////// the following has an absolute cutoff at distance > range (range == radius)
    wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", 
                                    "MaxRange", DoubleValue(range));
    wifiPhy.SetChannel (wifiChannel.Create ());
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue (phyMode),
                                  "ControlMode", StringValue (phyMode));

    ////// Setup the rest of the upper mac
    ////// Setting SSID, optional. Modified net-device to get Bssid, mandatory for AP unicast
    Ssid ssid = Ssid ("wifi-default");
    // wifi.SetRemoteStationManager ("ns3::ArfWifiManager");

    ////// Add a non-QoS upper mac of STAs, and disable rate control
    NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default ();
    ////// Active associsation of STA to AP via probing.
    wifiMacHelper.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),
                           "ActiveProbing", BooleanValue (true),
                           "ProbeRequestTimeout", TimeValue(Seconds(0.25)));

    ////// Creating 2 mobile nodes
    NodeContainer consumers;
    consumers.Create(wifiSta);

    NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMacHelper, consumers);
    NetDeviceContainer devices = staDevice;

    ////// Setup AP.
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
    wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid),
                     "BeaconGeneration", BooleanValue(false));
    for (int i = 0; i < bottomrow; i++)
    {
        NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNodes[i]);
        devices.Add (apDevice);
    }

    ////// Note that with FixedRssLossModel, the positions below are not
    ////// used for received signal strength.
    
    ////// set positions for APs 
    MobilityHelper sessile;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    int Xpos = 0;
    for (int i=0; i < bottomrow; i++) {
        positionAlloc->Add(Vector(100+Xpos, 0.0, 0.0));
        Xpos += spacing;
    }
    sessile.SetPositionAllocator (positionAlloc);
    for (int i = 0; i < bottomrow; i++) {
        sessile.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        sessile.Install (wifiApNodes[i]);
    }
    
    ////// Setting mobility model and movement parameters for mobile nodes
    ////// ConstantVelocityMobilityModel is a subclass of MobilityModel
    MobilityHelper mobile; 
    mobile.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobile.Install(consumers);
    ////// Setting each mobile consumer 100m apart from each other
    int nxt = 0;
    for (uint32_t i=0; i<wifiSta ; i++) {
        Ptr<ConstantVelocityMobilityModel> cvmm = consumers.Get(i)->GetObject<ConstantVelocityMobilityModel> ();
        Vector pos (0-nxt, 0, 0);
        Vector vel (speed, 0, 0);
        cvmm->SetPosition(pos);
        cvmm->SetVelocity(vel);
        nxt += 100;
    }
    
    // std::cout << "position: " << cvmm->GetPosition() << " velocity: " << cvmm->GetVelocity() << std::endl;
    // std::cout << "mover mobility model: " << mobile.GetMobilityModelType() << std::endl; // just for confirmation

    // 3. Install NDN stack on all nodes
    NS_LOG_INFO("Installing NDN stack");
    ndn::StackHelper ndnHelper;
    //ndnHelper.InstallAll();
    ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000");
    //ndnHelper.SetDefaultRoutes(true);
    //ndnHelper.SetOldContentStore("ns3::ndn::cs::Nocache");
    ndnHelper.InstallAll();

    // Choosing forwarding strategy
    ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
    //ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

    // Installing global routing interface on all nodes
    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    ndnGlobalRoutingHelper.InstallAll();

    // 4. Set up applications
    NS_LOG_INFO("Installing Applications");
    // Consumer Helpers
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    consumerHelper.SetPrefix("/root/prefix");
    //consumerHelper.SetPrefix("/test/prefix");
    consumerHelper.SetAttribute("Frequency", DoubleValue(10.0));
    consumerHelper.Install(consumers.Get(0)).Start(Seconds(0.0));
    consumerHelper.Install(consumers.Get(1)).Start(Seconds(0.0));
    // Producer Helpers
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
    // Register /root prefix with global routing controller and
    // install producer that will satisfy Interests in /root namespace
    ndnGlobalRoutingHelper.AddOrigins("/root", producer);    
    producerHelper.SetPrefix("/root");
    producerHelper.Install(producer);

    // Calculate and install FIBs
    ndn::GlobalRoutingHelper::CalculateRoutes();

    // Tracing
    wifiPhy.EnablePcap ("simple-wifi-mobility", devices);

    Simulator::Stop (Seconds (endtime));

    AnimationInterface anim (animFile);

    for (uint32_t i=0; i<wifiSta ; i++) {
      string str = "simple-wifi-trace" + std::to_string(i+1) + ".txt";
      ndn::L3RateTracer::Install(consumers.Get(i), str, Seconds(endtime-0.5));
    }
    
    //ndn::CsTracer::Install(routers[0],"simple-wifi-mobility-trace.txt", Seconds(1.0));
    
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
  }
}

int main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
