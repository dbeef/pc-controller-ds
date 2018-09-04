#include <nds.h>
#include <stdio.h>
#include <dswifi9.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <sys/socket.h>

#define SSID "feebdx"
#define SSID_LEN 6
#define WEP_KEY "42086"
#define PORT 8080

char ack{};

int receive_ack(int socket) {
    return recv(socket, &ack, sizeof(ack), 0);
}

void receive_transmission() {

    printf("[*] Trwa konfiguracja serwera:\n");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1; //True, dla re-używania portu i adresu
    int addrlen = sizeof(address);

    printf("[*] Ustawiam deskryptor pliku\n");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[-] Deskryptor pliku nie zostal ustawiony\n");
        exit(EXIT_FAILURE);
    }

    printf("[*] Ustawiam konfiguracje socketu \n");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("[-] Nie udalo sie ustawic konfiguracji\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] Ustawiono opcje\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    printf("[*] Wiazanie socketu z portem %i\n", PORT);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("[-] Nie udalo sie zwiazac socketu");
        exit(EXIT_FAILURE);
    }

    printf("[*] Zwiazano socket z portem, nasluchiwane polaczen\n");

    if (listen(server_fd, 3) < 0) {
        perror("[*] Blad podczas nasluchiwania \n");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
        perror("[-] Blad podczas nawiazywania polaczenia\n");
        exit(EXIT_FAILURE);
    }

    printf("[*] Nawiazano polaczenie\n");

    for (;;) {
        receive_ack(new_socket);
        scanKeys();
        uint16 keys_held = keysHeld();
        send(new_socket, &keys_held, sizeof(uint16), 0);
    }

    shutdown(new_socket, SHUT_WR);
    closesocket(new_socket);
    //ponieważ closesocket zadziała asynchronicznie, to musimy poczekać, aby upewnić się, że pakiet
    //RST (zamknięcie połączenia) został wysłany i pythonowy skrypt dowie się, że połączenie zostało zerwane -
    //inaczej od razu przejdziemy do wywołania return 0;
    //częstotliwość odświeżania ekranu NDS to 60Hz, co da 60 * (1/60) = 1s delayu
    for (int frames = 0; frames < 24; frames++) { swiWaitForVBlank(); }
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
    memset(our_ap->bssid, 0, 6); //Zerujemy BSSID, aby biblioteka go zignorowała

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
        printf("\nNie udalo sie polaczyc!\n");
    }
    return 0;
}

