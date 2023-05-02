#!/usr/bin/env python3

from netmiko import ConnectHandler

fw1 = {
  "device_type": "vyos",
  "host": "10.0.50.1",
  "username": "vyos",
  "password": "vyos",
  "port": 22,
}

fw2 = {
    "device_type": "vyos",
    "host": "10.0.51.1",
    "username": "vyos",
    "password": "vyos",
    "port": 22,
}

net_connect_fw1 = ConnectHandler(**fw1)
net_connect_fw2 = ConnectHandler(**fw2)

# Identify the attackers' IP addresses
attackers_ips = ['200.2.2.200']

# Create a list of firewall blocking rules for the attackers' IP addresses
block_rules = []
i=1
for ip in attackers_ips:
    block_rule = f'set firewall name FROM-OUTSIDE-TO-INSIDE rule '+ str(i) +' source address '+str(ip)+'/32'
    block_rules.append(block_rule)
    block_rule = f'set firewall name FROM-OUTSIDE-TO-INSIDE rule '+ str(i) +' action drop'
    block_rules.append(block_rule)
    block_rule = f'set firewall name FROM-OUTSIDE-TO-DMZ rule '+ str(i) +' source address '+str(ip)+'/32'
    block_rules.append(block_rule)
    block_rule = f'set firewall name FROM-OUTSIDE-TO-DMZ rule '+ str(i) +' action drop'
    block_rules.append(block_rule)
    i=i+1
    if i%10==0: i=i+1

# Set configuration and create blocking rules
config_commands = [
                    *block_rules, # Unpack the list of blocking rules into individual arguments
                  ]


print("\n---FW1---\n")
output_fw1 = net_connect_fw1.send_config_set(config_commands, exit_config_mode=False)
print(output_fw1)
output_fw1 = net_connect_fw1.commit()
print(output_fw1)
output_fw1 = net_connect_fw1.send_command("show firewall")
print(output_fw1)

print("\n---FW2---\n")
output_fw2 = net_connect_fw2.send_config_set(config_commands, exit_config_mode=False)
print(output_fw2)
output_fw2 = net_connect_fw2.commit()
print(output_fw2)
output_fw2 = net_connect_fw2.send_command("show firewall")
print(output_fw2)



