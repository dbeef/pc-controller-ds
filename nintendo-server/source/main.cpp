#include <nds.h>
#include <stdio.h>
#include <dswifi9.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>

#define SSID "feebdx"
#define SSID_LEN 6
#define WEP_KEY "42086"
#define PORT 8080

char ack{};
int stat;

void receive_ack(int &stat, int &socket) {
    do {
        stat = recv(socket, &ack, sizeof(ack), 0);
        if (stat == 1) {
            break;
        }
    } while (stat < 0);

};

void receive_transmission() {

    printf("[*] Setting up server:\n");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1; //True, dla re-używania portu i adresu
    int addrlen = sizeof(address);

    printf("[*] Setting socket file descriptor\n");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[-] Setting socket file descriptor failed\n");
        exit(EXIT_FAILURE);
    }

    printf("[*] Setting socket options\n");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("[-] Setting socket options\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] Done setting socket\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    printf("[*] Binding socket to port %i\n", PORT);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("[-] Binding socket failed");
        exit(EXIT_FAILURE);
    }

    printf("[+] Done binding socket to port %i\n", PORT);

    printf("[*] Listening for connections\n");

    if (listen(server_fd, 3) < 0) {
        perror("[*] Error on listening for connection\n");
        exit(EXIT_FAILURE);
    }

    printf("[*] Accepting connection\n");

    if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
        perror("[-] Failed accepting connection\n");
        exit(EXIT_FAILURE);
    }

    printf("Entering loop: \n");

    for (;;) {
        receive_ack(stat, new_socket);
        scanKeys();
        u16 keys_held = keysHeld();

        do {
            stat = send(new_socket, &keys_held, sizeof(u16), 0);
        } while (stat < 0);

    }
    //koniec funkcji receive_transmission
}

int main() {

    Wifi_InitDefault(false); //Inicjalizacja biblioteki
    Wifi_SetIP(0, 0, 0, 0, 0); //Biblioteka od wifi będzie używać DHCP
    consoleDemoInit(); //Init konsoli, teraz można logować (ale tylko na dolnym ekranie)
    Wifi_AccessPoint *our_ap = new Wifi_AccessPoint; //Struktura zawierająca ustawienia naszej sieci
    strncpy(our_ap->ssid, SSID, SSID_LEN); //Ustawiamy nazwę sieci
    our_ap->ssid_len = SSID_LEN; //Ustawiamy długość nazwy sieci
    our_ap->channel = 1; //Ustawiamy kanał - należy pamiętać, żeby zgadzał się z tym od sieci
    memset(our_ap->bssid, 0, 6); //Ustawiamy BSSID na zera, aby biblioteka go zignorowała

    printf("Connecting to %s\n", our_ap->ssid);

    unsigned char wep_key[] = WEP_KEY;
    Wifi_ConnectAP(our_ap, WEPMODE_40BIT, 0, wep_key);
    int status = ASSOCSTATUS_DISCONNECTED;

    while (status != ASSOCSTATUS_ASSOCIATED && status != ASSOCSTATUS_CANNOTCONNECT) {
        status = Wifi_AssocStatus();
        int len = strlen(ASSOCSTATUS_STRINGS[status]);
        printf("\x1b[0;0H\x1b[K");
        printf("\x1b[0;%dH%s", (32 - len) / 2, ASSOCSTATUS_STRINGS[status]);
        swiWaitForVBlank(); //Czekanie do narysowania następnej klatki
    }

    if (status == ASSOCSTATUS_ASSOCIATED) {
        u32 ip = Wifi_GetIP();
        printf("\nip: [%li.%li.%li.%li]\n", (ip) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
        receive_transmission(); //Nasza funkcja otwierająca socket
    } else {
        printf("\nConnection failed!\n");
    }
    return 0;
}

