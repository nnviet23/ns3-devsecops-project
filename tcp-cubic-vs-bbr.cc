#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

// Define logging component for tracing
NS_LOG_COMPONENT_DEFINE ("TcpComparison");

int main (int argc, char *argv[])
{
    // 1. Simulation parameters
    double errorRate = 0.05;         // 5% packet loss rate
    std::string bandwidth = "10Mbps"; // Bottleneck bandwidth
    std::string delay = "50ms";      // Propagation delay
    double simulationTime = 30.0;    // Total simulation time in seconds

    // Allow user to change errorRate via command line (e.g., --errorRate=0.1)
    CommandLine cmd;
    cmd.AddValue ("errorRate", "Packet loss rate (0.0 to 1.0)", errorRate);
    cmd.Parse (argc, argv);

    // 2. Node creation (S1, S2 as senders, Router, and Receiver)
    NodeContainer nodes;
    nodes.Create (4);
    
    // Create node containers for individual links
    NodeContainer s1r = NodeContainer (nodes.Get (0), nodes.Get (2));    // S1 to Router
    NodeContainer s2r = NodeContainer (nodes.Get (1), nodes.Get (2));    // S2 to Router
    NodeContainer rDest = NodeContainer (nodes.Get (2), nodes.Get (3));  // Router to Receiver

    // 3. Configure Point-to-Point physical layer and channel
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
    p2p.SetChannelAttribute ("Delay", StringValue (delay));

    // Install network devices on nodes
    NetDeviceContainer dev1 = p2p.Install (s1r);
    NetDeviceContainer dev2 = p2p.Install (s2r);
    NetDeviceContainer devDest = p2p.Install (rDest);

    // 4. Introduce Packet Loss on the destination link (Router -> Receiver)
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (errorRate));
    em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
    // Apply error model to the receiving device at the destination node
    devDest.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

    // 5. Install Internet Stack
    InternetStackHelper stack;
    
    // Configure Sender S1: Use TCP Cubic
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpCubic"));
    stack.Install (nodes.Get (0)); 
    
    // Configure Sender S2: Use TCP BBR
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpBbr"));
    stack.Install (nodes.Get (1)); 
    
    // Install default TCP stack on Router and Receiver nodes
    stack.Install (nodes.Get (2)); 
    stack.Install (nodes.Get (3)); 

    // Assign IP addresses to interfaces
    Ipv4AddressHelper address;
    
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if1 = address.Assign (dev1);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer if2 = address.Assign (dev2);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ifDest = address.Assign (devDest);

    // Populate global routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // 6. Application Configuration (Data Traffic)
    uint16_t port = 8080; 
    
    // Receiver App: PacketSink installed on Receiver node
    PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApp = sink.Install (nodes.Get (3));
    sinkApp.Start (Seconds (0.0));
    sinkApp.Stop (Seconds (simulationTime));

    // Sender Flow S1 (TCP Cubic) to Receiver
    OnOffHelper client1 ("ns3::TcpSocketFactory", InetSocketAddress (ifDest.GetAddress (1), port));
    client1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    client1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    client1.SetAttribute ("DataRate", StringValue (bandwidth));
    ApplicationContainer app1 = client1.Install (nodes.Get (0));
    app1.Start (Seconds (1.0)); // Start traffic at 1s
    app1.Stop (Seconds (simulationTime));

    // Sender Flow S2 (TCP BBR) to Receiver
    OnOffHelper client2 ("ns3::TcpSocketFactory", InetSocketAddress (ifDest.GetAddress (1), port));
    client2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    client2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    client2.SetAttribute ("DataRate", StringValue (bandwidth));
    ApplicationContainer app2 = client2.Install (nodes.Get (1));
    app2.Start (Seconds (1.0)); // Start traffic at 1s
    app2.Stop (Seconds (simulationTime));

    // 7. FlowMonitor to collect statistics (Throughput, Delay, Loss)
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    // 8. Execution
    Simulator::Stop (Seconds (simulationTime));
    Simulator::Run ();

    // Serialize statistics to XML file for Python plotting
    monitor->SerializeToXmlFile ("results.xml", true, true);
    
    // Cleanup
    Simulator::Destroy ();
    return 0;
}