#include "daemon.h"
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void process_packet(unsigned char* buffer, struct ip_stat* stat, int* n, char iface[20  ]);

int main()
{

  pid_t pid = 0, sid = 0;

    pid = fork();

    // Indication of fork() failure
    if (pid < 0)
    {
        printf("fork failed!\n");
        exit(EXIT_FAILURE);
    }

    // Parent process. Need to kill it.
    if (pid > 0)
    {
        printf("PID: %d\n", pid);
        exit(EXIT_FAILURE);
    }

    // Set new session
    sid = setsid();
    if(sid < 0)
        exit(EXIT_FAILURE);

    // Close stdin. stdout and stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int saddr_size, data_size, sock_raw;
    struct sockaddr saddr;
    unsigned char* buffer = (unsigned char*)malloc(65536);

    struct ip_stat* stat = malloc(65536);
    int stat_size = 0;
    char iface[20];
    char *path_stat = "stat", *path_iface = "iface";
    
    read_stat(path_stat, stat, &stat_size);
    read_iface(path_iface, iface);
    //Create a raw socket that shall sniff
    sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

    // Set specific interface
    setsockopt(sock_raw, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface));

    if (sock_raw < 0) {
        printf("Socket Error\n");
                exit(EXIT_FAILURE);
    }

    // Background working
    while (1) {
        saddr_size = sizeof saddr;

        //Receive a packet
        data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, &saddr_size);

        if (data_size < 0) {
            printf("Recvfrom error, failed to get packets\n");
            close(sock_raw);
                   exit(EXIT_FAILURE);

        }

        process_packet(buffer, stat, &stat_size, iface);

        write_stat(path_stat, stat, &stat_size);
    }

}
   void process_packet(unsigned char* buffer, struct ip_stat* stat, int* n, char iface[20])
    {
        struct iphdr* iph = (struct iphdr*)buffer;
        int i;

        for (i = 0; i < *n; i++)
            if (stat[i].ip_address == iph->saddr && !strcmp(stat[i].iface,iface)) {
                stat[i].counter++;
                return;
            }

        stat[i].ip_address = iph->saddr;
        stat[i].counter = 1;
        strcpy(stat[i].iface, iface);
        (*n)++;

        sort(stat, *n);
    }