import requests
import json

base_url = "http://0.0.0.0:9302"

def main():
    response = requests.get(base_url)

    text = json.dumps(response.text)
    
    print("Port status:")
    #get port_status of all switches
    #from json get the lines with "port_status" and create a list
    ports = [line.replace("\\", "") for line in text.split('\\n') if ("port_status" in line and "#" not in line)]
    #want to get dp_name and port_status -> 'port_status{dp_id="0x1",dp_name="s1",port="1"} 1.0'
    dp_name = ports[0].split('"')[3]
    print("Switch: {}".format(dp_name))
    for line in ports:
        if dp_name == line.split('"')[3]:
            port = line.split('"')[7]
            status = line.split(' ')[-1]
            print("\tPort: {}\t\t\t\tStatus: Up".format(port) if status == "1.0" else "\tPort: {}\tStatus: Down".format(port))
        else:
            dp_name = line.split('"')[3]
            port = line.split('"')[7]
            status = line.split(' ')[-1]
            print("Switch: {}\n\tPort: {}\t\t\t\tStatus: Up".format(dp_name, port) if status == "1.0" else "\tPort: {}\tStatus: Down".format(port))

    #get faucet_config_table_names of all switches
    #from json get the lines with "faucet_config_table_names" and create a list
    print("\nGet Flow tables:")
    tables = [line.replace("\\", "") for line in text.split('\\n') if ("faucet_config_table_names" in line and "#" not in line)]
    dp_name = tables[0].split('"')[3]
    #get all lines of first switch
    switch = [line.replace("\\", "") for line in tables if (dp_name in line)]
    for line in switch:
        #from line get table_name and next_tables
        next_tables=(line.split("next_tables=\""))[1].split("\"")[0]
        table_name=(line.split("table_name=\""))[1].split("\"")[0]
        #replace , with -> in next_tables
        next_tables = next_tables.replace(",", " -> ") 
        #print table_name and next_tables
        print("\t{} -> {}".format(table_name, next_tables))

    print("\nPackages sent, received and ignored by switches:")

    print("\nOF Packages received by switches:")
    #get of_flowmsgs_sent_total of all switches
    #from json get the lines with "of_flowmsgs_sent_total" and create a list
    sent = [line.replace("\\", "") for line in text.split('\\n') if ("of_flowmsgs_sent_total" in line and "#" not in line)]
    #want to get dp_name and packages sent -> 'of_flowmsgs_sent_total{dp_id="0x1",dp_name="s1"} 0.0'
    for line in sent:
        dp_name = line.split('"')[3]
        packages_sent = line.split(' ')[-1]
        print("Switch: {} sent {} packages".format(dp_name, packages_sent))

    print("\nPackages received by switches:")
    #get number of OF packet_ins received from DP
    #from json get the lines with "of_packet_ins_total" and create a list
    dropped = [line.replace("\\", "") for line in text.split('\\n') if ("of_packet_ins_total" in line and "#" not in line)]
    #want to get dp_name and packages received -> 'of_packet_ins_total{dp_id="0x1",dp_name="s1"} 0.0'
    for line in dropped:
        dp_name = line.split('"')[3]
        packages_received = line.split(' ')[-1]
        print("Switch: {} received {} packages".format(dp_name, packages_received))

    print("\nPackages ignored by switches:")
    #get number of OF packet_ins received but ignored from DP (due to rate limiting)
    #from json get the lines with "of_ignored_packet_ins_total" and create a list
    dropped = [line.replace("\\", "") for line in text.split('\\n') if ("of_ignored_packet_ins_total" in line and "#" not in line)]
    #want to get dp_name and packages ignored -> 'of_ignored_packet_ins_total{dp_id="0x1",dp_name="s1"} 0.0'
    for line in dropped:
        dp_name = line.split('"')[3]
        packages_ignored = line.split(' ')[-1]
        print("Switch: {} ignored {} packages".format(dp_name, packages_ignored))

    print("\nPackages unexpected by switches:")
    #number of OF packet_ins received that are unexpected from DP (e.g. for unknown VLAN)
    #from json get the lines with "of_unexpected_packet_ins_total" and create a list
    dropped = [line.replace("\\", "") for line in text.split('\\n') if ("of_unexpected_packet_ins_total" in line and "#" not in line)]
    for line in dropped:
        dp_name = line.split('"')[3]
        packages_ignored = line.split(' ')[-1]
        print("Switch: {} unexpected {} packages".format(dp_name, packages_ignored))

    
if __name__ == "__main__":
    main()
