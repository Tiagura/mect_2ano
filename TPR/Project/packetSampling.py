import sys
import argparse
import pyshark
from netaddr import IPNetwork, IPAddress, IPSet

def packetHandler(tsamp, sourceIP, destinationIP, lengthIP, time_delta, sampDelta, outfile):
    global scnets
    global numberPackets
    global T0
    global last_ks
    global out # [request, upload bytes, response, download bytes, avg_response_delta, avg_request_delta, avg_delta, min_delta, max_delta]
    global avg_response_delta   # time between dns request and response
    global avg_request_delta    # time between dns requests
    global avg_delta            # time between all dns packets
    global last_request_ts

    if(numberPackets == 0):
        T0 = float(tsamp)
        last_ks = 0
        avg_request_delta = [0,0]
        avg_response_delta = [0,0]
        avg_delta = [0,0]
        last_request_ts = 0
    
    ks = int((float(tsamp)-T0)/sampDelta)

    if ks > last_ks:
        #print("Writting to file")
        # calculate avg delta
        if avg_response_delta[1] != 0:
            out[4] = avg_response_delta[0]#/avg_response_delta[1]
        if avg_request_delta[1] != 0:
            out[5] = avg_request_delta[0]#/avg_request_delta[1]
        if avg_delta[1] != 0:
            out[6] = avg_delta[0]#/avg_delta[1]

        outfile.write('{} {} {} {} {} {} {} {} {}\n'.format(*out))
        #print('{} {} {} {} {}'.format(*out))
        out=[0, 0, 0, 0, 0, 0, 0, 0, 0]

        #clear delta variables
        avg_response_delta = [0,0]
        avg_request_delta = [0,0]
        avg_delta = [0,0]

    if(IPAddress(sourceIP) in scnets):  # This is a dns request
        # add count of bytes
        out[0] = out[0] + 1
        out[1] = out[1] + int(lengthIP)
        # calculate difference between requests
        if last_request_ts != 0:
            delta = float(tsamp) - last_request_ts
            avg_request_delta[0] = avg_request_delta[0] + delta
            avg_request_delta[1] = avg_request_delta[1] + 1
        last_request_ts = float(tsamp)

    elif(IPAddress(destinationIP) in scnets):  # This is a dns response
        # add count of bytes
        out[2] = out[2] + 1
        out[3] = out[3] + int(lengthIP)
        # calculate difference between dns request and response
        delta = float(tsamp) - last_request_ts
        avg_response_delta[0] = avg_response_delta[0] + delta
        avg_response_delta[1] = avg_response_delta[1] + 1

    avg_delta[0] = avg_delta[0] + time_delta
    avg_delta[1] = avg_delta[1] + 1

    if time_delta < out[7] or out[7] == 0:
        out[7] = time_delta
    
    if time_delta > out[8] or out[8] == 0:
        out[8] = time_delta

    last_ks = ks
    numberPackets = numberPackets + 1


def main():
    parser=argparse.ArgumentParser()
    #add arg for client IP
    parser.add_argument('-c', '--cnet', nargs='+',required=True, help='client network(s)')
    #add arg for dns or secure dns protocol
    parser.add_argument('-p', '--protocol', nargs='?',required=True, help='protocol')
    parser.add_argument('-i', '--input', nargs='?',required=True, help='input file')
    parser.add_argument('-o', '--output', nargs='?',required=False, help='output file')
    parser.add_argument('-d', '--delta', nargs='?',required=False, help='samplig delta interval')
    args=parser.parse_args()

    # delta interval
    if args.delta is None:
        sampDelta=5
    else:
        sampDelta=float(args.delta)

    # File input and output
    fileInput=args.input
    
    if args.output is None:
        fileOutput=fileInput+"_d"+str(sampDelta)+".dat"
    else:
        fileOutput=args.output

    outfile = open(fileOutput,'w') 

    # Client network
    cnets=[]
    for n in args.cnet:
        print(n)
        try:
            nn=IPNetwork(n)
            cnets.append(nn)
        except:
            print('{} is not a network prefix'.format(n))
    #print(cnets)
    if len(cnets)==0:
        print("No valid client network prefixes.")
        sys.exit()
    global scnets
    scnets=IPSet(cnets)

    # Protocol
    if args.protocol is None or (args.protocol != "dns" and args.protocol != "secure_dns"):
        print("No valid protocol.")
        sys.exit()

    #Global variables
    global numberPackets
    global T0
    global last_ks
    global out #  [request, upload bytes, response, download bytes, avg_response_delta, avg_request_delta]
    global avg_response_delta
    global avg_request_delta
    global last_request_ts
    
    numberPackets = 0
    out=[0,0,0,0,0,0,0,0,0]

    capture = pyshark.FileCapture(fileInput, display_filter=args.protocol)
    for pkt in capture:
    #for i in range(0,10):
        #pkt = capture[i]
        try:
            timestamp = pkt.sniff_timestamp

            sourceIP = pkt.ip.src
            #sourcePort = pkt[pkt.transport_layer].srcport

            destinationIP = pkt.ip.dst
            #destinationPort = pkt[pkt.transport_layer].dstport

            lengthIP = pkt.ip.len

            if  args.protocol == "dns":
                time_delta = pkt.udp.time_delta     # time between displayed packets
            elif  args.protocol == "secure_dns":
                time_delta = pkt.tcp.time_delta

            packetHandler(timestamp, sourceIP, destinationIP, lengthIP, float(time_delta), sampDelta, outfile)

        except AttributeError as e:
            print("AttributeError: "+str(e))
            continue

        except Exception as e:
            print("Exception: "+str(e))
            continue

    print(scnets)

    outfile.close()

if __name__ == "__main__":
    main()

