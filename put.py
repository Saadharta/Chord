import socket
import sys
import time
import json
import random

TailleReseau = 16

def main(argv):
    global TailleReseau
    if (len(argv) == 2):
        IP_Port=argv[0]
        IP=IP_Port.split(':')[0]
        PORT=IP_Port.split(':')[1]
        node = argv[1]
        key= random.randint(0,TailleReseau-1)
        valeur= random.randint(0,656547825)
        idMSG= random.randint(0,55546)
        print("Je mets dans le noeud = " + str(key)+" la valeur " + str(valeur)+" en partant de "+str(IP)+":"+str(PORT))
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
        time.sleep(.05)
        print("j'envoi a "+str(IP)+":"+str(PORT))
        s.connect((IP,int(PORT)))
        messageAEnvoye = {"cmd": "put",
                            "args":{
                                    "host" : {
                                    "idNode" : node,
                                    "IP" : IP,
                                    "port" : PORT
                                    },
                                    "key" : key,
                                    "value": valeur,
                                    "id_request" : idMSG
                            }
                        }
        jsonRes = json.dumps(messageAEnvoye)
        jsonRes=bytes(jsonRes,'utf-8')
        print("J'envoie "+str(jsonRes))
        s.sendto(jsonRes,(IP, int(PORT)))
    else:
        print("USAGE put.py <IP:port> <noeud de départ>")


if __name__ == '__main__':
    main (sys.argv[1:])
