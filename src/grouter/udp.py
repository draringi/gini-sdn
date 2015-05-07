# module_name, packaversionCmdge_name, ClassName, method_name,
#ExceptionName, function_name, GLOBAL_VAR_NAME, 
#instance_var_name, function_parameter_name, local_var_name.

import _GINIC
import inspect
import array
import struct
import string
from socket import htons, ntohs
import os
# class PCBCore:
#     print("PCBCore is initilizing...")
#     NAME = 'PCB CORE'
#     MAX_PCB_NUMBER = 20
#     MAX_PORT_NUMBER = 65536
#     MAX_BUFFER_SIZE = 1000
#     socket_count = 0
#     PCB_dict = {} #{port: (pkt1, pkt2, ...)}
#     #lock = thread.allocate_lock()
#     #print(lock)
#     print("PCBCore is initilaized")
#     @staticmethod
#     def pcb_bind_socket(port):
#         if PCBCore.socket_count == PCBCore.MAX_PCB_NUMBER:
#             print("Fail! Cannot have more sockets!")
#             return
#         if PCBCore.socket_count ==  PCBCore.MAX_PORT_NUMBER:
#             print("Fail! Port number invalid")
#             return
#         if PCBCore.PCB_dict.has_key(port):
#             print("Fail! Port already binded!")
#             return
#         PCBCore.PCB_dict.update({port : []})
#         print("Socket binded!")

#     @staticmethod
#     def pcb_close_socket(port):
#         #PCBCore.lock.acquire()
#         if PCBCore.PCB_dict.has_key(port):
#             ##PCBCore.lock.acquire()
#             del PCBCore.PCB_dict[port]
#             PCBCore.socket_count = PCBCore.socket_count - 1
#             #PCBCore.lock.release()
#         else:
#             #PCBCore.lock.release()
#             print("Socket doesn't exist!")

#     @staticmethod
#     def recv_packet(pkt):
#         print("[recv_packet] 1")
#         port = pkt.dport
#         print("[recv_packet] testing PCB_dict")
#         #PCBCore.lock.acquire()
#         if PCBCore.PCB_dict.has_key(port):
#             print("[recv_packet] 2")
#             new_value = PCBCore.PCB_dict[port].append(pkt)
#             PCBCore.PCB_dict.update({port: new_value})
#         print("[recv_packet] Done")
#         #PCBCore.lock.release()

#     @staticmethod
#     def pcb_get_packet(port):
#         print("[pcb_get_packet] 1")
#         #PCBCore.lock.acquire()
#         print("[pcb_get_packet] locked")
#         if len(PCBCore.PCB_dict[port]) > 0:
#             print("Found pkt")
#             pkt = PCBCore.PCB_dict[port].pop(0)
#             print("[pcb_get_packet] Done")
#             #PCBCore.lock.release()
#             return pkt
#         else:
#             print ""
#             #PCBCore.lock.release()
#     @staticmethod
#     def get_name():
#         print("[get_name]: %s") % PCBCore.NAME

# PCB
# class PCB:

#     def __init__(self, gpacket):
#         self.sport = gpacket.sport
#         self.dport = gpacket.dport
#         self.data = gpacket.data
#
MAX_PCB_NUMBER = 5
MAX_BUFFER_SIZE = 5


class UDPPcbEntry:
    def __init__(self,
                 port=-1,
                 buff=[]):
        self.port = port
        self.buff = buff


class UDPPcb:
    def __init__(self,
                 size=0,
                 entry=[]):
        self.size = size
        self.entry = [UDPPcbEntry() for i in range(5)]

    def pcb_bind(self, port):
        for check in range(MAX_PCB_NUMBER):
            if self.entry[check].port == -1:
                self.entry[check].port == port
                return True
        gprint("bind failed! Not enough space")

    def pcb_check(self, port):
        for check in range(MAX_PCB_NUMBER):
            if self.entry[check].port == port:
                gprint("check seccessfuly! port :", port)
                return check
        return -1

    def pcb_unbind(self, port):
        for check in range(MAX_PCB_NUMBER):
            if self.entry[check].port == port:
                self.entry[check].port == -1
                return True
        gprint("unbind failed! port invalid!")


#class udp:







# class Ncer:
#
#     def __init__(self):
#         print("Init: NC")
#         self.port = 777
#
#     def _recv_from(self):
#         print("Ncer:[_recv_from] 1")
#         pkt = PCBCore.pcb_get_packet(self.port)
#         print(">>>")
#         print(pkt)
#     def thread_recv_from(self):
#         #PCBCore.pcb_close_socket(self.port)
#         PCBCore.pcb_bind_socket(self.port)
#         print("creating thread")
#         while True:
#             Thread(target = self._recv_from).start()
#             time.sleep(3)
#         print("[thread_recv_from] closing socket")
#         PCBCore.pcb_close_socket(self.port)

def Config():
    print("Py::[Config]")
    return "nc"


def Command_Line(str):
    print("[Command_Line] start!")
    print(str)
    if pcb == None:
        pcb = UDPPcb()
    print("[Command_Line] end!")


def giniudp():
    print("[giniudp]:: UDP sever/client::")
    print("call with 'nc -u -l port/nc -u ip port'")
    #print("???")
    #nc = Ncer()
    #upkt = Packet(7000, 7, 10, 0, "bb")
    upkt = Packet(34591, 8889, 5, 0, "1111\n")
    src_ip = ip_ltostr([128, 1, 168, 192])
    dest_ip = ip_ltostr([2, 1, 168, 192])
    #upkt_a = assemble(upkt)
    upkt_a = assemble(upkt, 0)  # no checksum
    _GINIC.IPOutgoingPacket(upkt_a, dest_ip, len(upkt_a), 1, 17)

    #nc.thread_recv_from()


def Protocol_Processor(gpkt):
    print("=====Py#[Packet_Processor]::=====")
    print("[UDPPacketProcess]Process ID: %d") % os.getpid();
    print("ready")
    if (pcb == None):
        pcb = UDPPcb()

    #print(gpkt)
    #print("dir:")
    #print(dir(gpkt))
    udpPacketFromC = _GINIC.getUDPPacketString(gpkt)
    packet = disassemble(udpPacketFromC, 1)
    print(packet)
    if packet.dport == 7:
        print("recieved an UDP ECHO packet")
        _udp_echo_reply(packet)
    else:
        pass
    print("Done")


def _udp_echo_reply(packet):
    port_tmp = packet.sport
    packet.sport = packet.dport
    packet.dport = port_tmp
    dest_ip = __find_dest_ip(packet)
    print("[_udp_echo_reply]dest ip: %d", dest_ip)
    newflag = 1
    prot = 17
    pkt = assemble(packet)
    size = len(pkt)
    print("[_udp_echo_reply]sending to %s : %d", dest_ip, packet.dport)
    print("pkt size: %d") % (len(pkt))
    udp2gpkt = _GINIC.createGPacket(pkt)  #process udp2gpkt in typemap
    #print("udppkt size: %d") % (len(udp2gpkt))
    print("check Arg type:")
    print(udp2gpkt)
    print(size)
    print(newflag)
    print(prot)
    print("Done Checking==")
    print("Start to send back to C")
    #_GINIC.IPOutgoingPacket(udp2gpkt, dest_ip, size, newflag, prot)
    _GINIC.IPOutgoingPacket(pkt, dest_ip, size, newflag, prot)


# def _UDPPacketProcess(packet):
#     print("[_UDPPacketProcess] Thread")
#     PCBCore.recv_packet(packet)
def ip_ltostr(iplist):
    return struct.pack('BBBB', iplist[0], iplist[1], iplist[2], iplist[3])


def __find_dest_ip(pkt):
    ip = [2, 1, 168, 192]
    #return " ".join(str(x) for x in ip)
    ipstr = ip_ltostr(ip)
    print("[__find_dest_ip]len(ipstr) = %d") % len(ipstr)
    print("[__find_dest_ip]lip after ltostr:", ipstr)
    return ipstr


def __ntohs(s):
    print("in ntohs")
    return struct.pack('H', struct.unpack('!H', s)[0])


def __htons(s):
    return struct.pack('!H', struct.unpack('H', s)[0])


# def iph2net(s):
#     return s[:2] + __htons(s[2:4]) + __htons(s[4:6]) + __htons(s[6:8]) + s[8:]

# def net2iph(s):
#     return s[:2] + __ntohs(s[2:4]) + __ntohs(s[4:6]) + __ntohs(s[6:8]) + s[8:]

def udph2net(s):
    print("[udp2net]")
    return __htons(s[0:2]) + __htons(s[2:4]) + __htons(s[4:6]) + s[6:]


def net2updh(s):
    print("[net2updh]")
    return __ntohs(s[0:2]) + __ntohs(s[2:4]) + __ntohs(s[4:6]) + s[6:]


def udpcksum(s):
    print("[udpcksum]")
    if len(s) & 1:
        s = s + '\0'
    words = array.array('h', s)
    sum = 0
    for word in words:
        sum = sum + (word & 0xffff)
    hi = sum >> 16
    lo = sum & 0xffff
    sum = hi + lo
    sum = sum + (sum >> 16)
    print("checksum is : %s") % ((~sum) & 0xffff)
    #print("chsum>>end")
    return (~sum) & 0xffff


HDR_SIZE_IN_BYTES = 8


class Packet:
    def __init__(self,
                 sport=0,
                 dport=0,
                 ulen=8,
                 sum=0,
                 data=''):
        self.sport = sport
        self.dport = dport
        self.ulen = ulen
        self.sum = sum
        self.data = data

    def __repr__(self):
        begin = "<UDP %d->%d len=%d " % (self.sport, self.dport, self.ulen)
        if self.ulen == 8:
            rep = begin + "\'\'>"
        elif self.ulen < 18:
            rep = begin + "%s>" % repr(self.data)
        else:
            rep = begin + "%s>" % repr(self.data[:10] + '...')
        return rep

    def __eq__(self, other):
        if not isinstance(other, Packet):
            return 0

        return self.sport == other.sport and \
               self.dport == other.dport and \
               self.ulen == other.ulen and \
               self.sum == other.sum and \
               self.data == other.data


    def _assemble(self, cksum=1):
        print("[_assemble]")
        self.ulen = 8 + len(self.data)
        src_ip = ip_ltostr([128, 1, 168, 192])
        dest_ip = ip_ltostr([2, 1, 168, 192])
        print("1")
        begin = struct.pack('HHH', self.sport, self.dport, self.ulen)
        print("2")
        pseudo_header = src_ip + dest_ip + '\000\000' + struct.pack('H', self.ulen)
        print("3")
        pseudo_packet = pseudo_header + begin + '\000\000' + self.data
        print("4")
        if cksum:
            self.sum = udpcksum(pseudo_packet)
            #self.sum = udpchecksum(packet)
            packet = begin + struct.pack('H', self.sum) + self.data
        else:
            self.sum = 0
            packet = begin + struct.pack('H', self.sum) + self.data
        self.__packet = udph2net(packet)
        return self.__packet

    def _disassemble(self, raw_packet, cksum=1):
        print("[_disassemble]")
        packet = net2updh(raw_packet)
        if cksum and packet[6:8] != '\000\000':
            our_cksum = udpcksum(packet)
            # no check sum
            # if our_cksum != 0:
            #      print("[_disassemble]Check sum invalid!!")
            #      raise ValueError, packet
        elts = map(lambda x: x & 0xffff, struct.unpack('HHHH', packet[:8]))
        [self.sport, self.dport, self.ulen, self.sum] = elts
        #tail = self.ulen# Haowei
        self.data = packet[8:self.ulen]


def assemble(packet, cksum=1):
    return packet._assemble(cksum)


def disassemble(buffer, cksum=1):
    print("[disassemble]")
    packet = Packet()
    packet._disassemble(buffer, cksum)
    return packet


def gprint(str):
    print(inspect.stack()[0][3] + "::" + str)


print("****************************   UDP   ****************************")
print("*               this is the beginning of the module             *")

pcb = UDPPcb()

print("pcb created!")