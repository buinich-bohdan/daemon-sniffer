#include <stdio.h>
#include <string.h>
#include <ifaddrs.h> //for interfaces
#include <stdlib.h> //exit
#include <stdbool.h>
#include <arpa/inet.h>
#include <zconf.h>

#include "daemon.h"
char iface[20];
char ifaces[20][20];
int countIfaces = 0;

void getInterfaces();

int main(int argc, char* argv[])
{
    char *path_stat = "stat", *path_iface = "iface";
    system("rm iface  2> /dev/null");
    getInterfaces();
    if (argc < 2) {
        printf("Need more arguments, use --help\n");
        exit(EXIT_FAILURE);
    }

    if (!strcmp(argv[1], "--help")) {
        printf("Welcome to Daemon-Snifer\n");
        printf("Available interfaces:\n");
        for (int i = 0; i < countIfaces; ++i)
            printf("%-8s \n", ifaces[i]);
        printf("Use this command for control program\n");
        printf("start               - packets are being sniffed from now on from default iface(eth0)\n");
        printf("stop                - stop sniffer\n");
        printf("showCnt   [ip]      - print number of packets from selected ip\n");
        printf("selIface  [iface]   - select interface for sniffing\n");
        printf("statIface [iface]   - show all collected statistics for particular interface, if iface omitted - for all interfaces \n");
        printf("--help              - show this menu\n");
        printf("GOODLUCK\n");
    }
    else if (!strcmp(argv[1], "start")) {
        printf("Starting daemon...\n");
        system("sudo ./daemon");
    }
    else if (!strcmp(argv[1], "stop")) {
        printf("Stoppping daemon...\n");
        system("kill `pidof ./daemon` 2> /dev/null");
    }
    else if (argc >= 3 && !strcmp(argv[1], "showCnt")) {
        struct ip_stat* stat = malloc(65536);
        int stat_size = 0;
        read_stat(path_stat, stat, &stat_size);

        // read passed argument of IP address
        unsigned ip = inet_addr(argv[2]);

        printf("Count: %d\n", search_ip(stat, stat_size, ip));
        free(stat);
    }
    else if (argc >= 3 && !strcmp(argv[1], "selIface")) {
        bool res = false;
        for (int i = 0; i < countIfaces; ++i) {
            if (!strcmp(argv[2], ifaces[i])) {
                strcpy(iface, argv[2]);
                res = true;
                write_iface(path_iface, iface);
                printf("Starting daemon...\n");
                system("sudo ./daemon");
                break;
            }
        }
        if (!res) {
            printf("Interface not found\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (!strcmp(argv[1], "statIface")) {
        if (argc != 3) {
            outputall("stat");
        }
        else {
            outputif("stat", argv[2]);
        }
    }
    else
        printf("Command not found\n");
    return 0;
}

void getInterfaces()
{
    struct ifaddrs *ifaddr, *ifa;
    int n;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr, n = 0; ifa != NULL && ifa->ifa_addr->sa_family == AF_PACKET; ifa = ifa->ifa_next, ++n) {
        if (ifa->ifa_addr == NULL)
            continue;
        strcpy(ifaces[n], ifa->ifa_name);
        ++countIfaces;
    }
    freeifaddrs(ifaddr);
}
