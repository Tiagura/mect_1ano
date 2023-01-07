from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import RemoteController
from mininet.cli import CLI
from mininet.log import setLogLevel
from mininet.node import OVSSwitch
from mininet.link import TCLink

#Topology is as follows:
#           h11   h12          h41
#            |      |          /
#            \      /         /
#               s1  --------s4---h44
#            /      \         \
#            |      |          \
#           s2-----s3          h45
#          / |      | \
#         /  |      |  \
#       h21  h22   h31  h32

#vlan100: h11, h21, h31, h41 10.0.1.0/24
#vlan200: h12, h22, h32, 10.0.2.0/24
#vlan400: h44 10.0.4.0/24
#vlan500: h45 10.0.5.0/24

class MyTopo( Topo ):
    def __init__( self ):
        Topo.__init__( self )
        #Add connectes to s1
        h11 = self.addHost('h11')#, ip='10.0.1.11/24')
        h12 = self.addHost('h12')#, ip='10.0.2.12/24')
        s1 = self.addSwitch('s1')
        self.addLink(s1, h11, port1=1, port2=1)
        self.addLink(s1, h12, port1=2, port2=1)

        #Add connectes to s2
        h21 = self.addHost('h21')#, ip='10.0.1.21/24')
        h22 = self.addHost('h22')#, ip='10.0.2.22/24')
        s2 = self.addSwitch('s2')
        self.addLink(s2, h21, port1=1, port2=1)
        self.addLink(s2, h22, port1=2, port2=1)

        #Add connectes to s3
        h31 = self.addHost('h31')#, ip='10.0.1.31/24')
        h32 = self.addHost('h32')#, ip='10.0.2.32/24')
        s3 = self.addSwitch('s3')
        self.addLink(s3, h31, port1=1, port2=1)
        self.addLink(s3, h32, port1=2, port2=1)

        #Add connectes to s4
        h41 = self.addHost('h41')#, ip='10.0.1.41/24')
        h44 = self.addHost('h44', ip='10.0.4.44/24')
        h45 = self.addHost('h45', ip='10.0.5.45/24')
        s4 = self.addSwitch('s4')
        self.addLink(s4, h41, port1=1, port2=1)
        self.addLink(s4, h44, port1=2, port2=1)
        self.addLink(s4, h45, port1=3, port2=1)

        #Add links between switches
        self.addLink(s1, s2, port1=3, port2=3)
        self.addLink(s1, s3, port1=4, port2=3)
        self.addLink(s2, s3, port1=4, port2=4)
        self.addLink(s1, s4, port1=5, port2=4)
        
        #Create DNS server
        dns = self.addHost('dns')
        
        #Link dns to s1
        self.addLink(s1, dns, port1=6, port2=1)

def main():
    net = Mininet(topo=MyTopo(), switch=OVSSwitch, cleanup=True, controller=RemoteController('c0', ip='0.0.0.0', port=6653, protocols="OpenFlow13"), autoSetMacs=True)
    
    net.start()
    CLI(net)
    net.stop()


if __name__ == "__main__":
    setLogLevel('info')
    main()


