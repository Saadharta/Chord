import socket
import sys
import time
import json
import random

TailleReseau = 20

def main(argv):
    global TailleReseau
    if (len(argv) == 3):
        IP_Port=argv[0]
        IP=IP_Port.split(':')[0]
        PORT=IP_Port.split(':')[1]
        node = argv[1]
        key= argv[2]
        print("Je cherche la clé :"+ str(key)+" en partant de "+str(IP)+":"+str(PORT))
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
        time.sleep(.05)
        print("j'envoi a "+str(IP)+":"+str(PORT))
        s.connect((IP,int(PORT)))
        messageAEnvoye = {"cmd": "get",
                            "args":{
                                    "host" : {
                                    "idNode" : int(node),
                                    "IP" : IP,
                                    "port" : int(PORT)
                                    },
                                    "key" : int(key),
                            }
                        }
        jsonRes = json.dumps(messageAEnvoye)
        jsonRes=bytes(jsonRes,'utf-8')
        print("J'envoie "+str(jsonRes))
        s.sendto(jsonRes,(IP, int(PORT)))
    else:
        print("USAGE get.py <IP:port> <noeud de départ> <clé>")


if __name__ == '__main__':
    main (sys.argv[1:])
