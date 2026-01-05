import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import os

def parse_flowmon(file_name):
    if not os.path.exists(file_name):
        print(f"Error: {file_name} not found.")
        return []

    tree = ET.parse(file_name)
    root = tree.getroot()
    stats = []

    # Use 'Flow' (capital F) to match NS-3 FlowMonitor XML tags
    for flow in root.findall('.//FlowStats/Flow'):
        flow_id = int(flow.get('flowId'))
        
        # Only select Flow 1 (Cubic) and Flow 2 (BBR) from S1 and S2 senders
        if flow_id not in [1, 2]:
            continue
            
        # Convert duration from nanoseconds to seconds
        # timeFirstRxPacket usually ends with 'ns', so we slice the last two characters
        time_start = float(flow.get('timeFirstRxPacket')[:-2]) * 1e-9
        time_end = float(flow.get('timeLastRxPacket')[:-2]) * 1e-9
        duration = time_end - time_start
        
        # Throughput formula (Mbps): (Bytes * 8 bits) / (duration * 10^6)
        rx_bytes = float(flow.get('rxBytes'))
        throughput = (rx_bytes * 8) / (duration * 1e6) if duration > 0 else 0
        stats.append(throughput)
    return stats

# Execute data extraction from results.xml
file_path = 'results.xml'
throughputs = parse_flowmon(file_path)

if len(throughputs) >= 2:
    labels = ['TCP Cubic', 'TCP BBR']
    plt.figure(figsize=(8, 6))
    colors = ['#3498db', '#e74c3c'] # Blue for Cubic, Red for BBR
    
    # Generate the bar chart
    bars = plt.bar(labels, throughputs, color=colors)
    plt.ylabel('Throughput (Mbps)')
    plt.title('TCP Performance Comparison: Cubic vs BBR (5% Loss)')
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Add data labels on top of each bar
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + 0.01, f'{yval:.3f} Mbps', 
                 ha='center', va='bottom', fontweight='bold')

    # Save the chart as an image for your report
    plt.savefig('simulation_chart.png')
    
    print("--- SIMULATION RESULTS ---")
    print(f"TCP Cubic Throughput: {throughputs[0]:.3f} Mbps")
    print(f"TCP BBR Throughput:   {throughputs[1]:.3f} Mbps")
    print("Chart saved as 'simulation_chart.png'")
    
    plt.show()
else:
    print("Error: Insufficient data found in results.xml.")
    print("Please ensure the NS-3 simulation ran correctly and produced FlowStats data.")