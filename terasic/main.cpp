#include <stdlib.h>
#include <stdio.h>
//#include <sys/types.h> //
//#include <sys/select.h> //
//#include <sys/time.h> //
#include <unistd.h>
//#include <sys/socket.h> //
//#include <netinet/in.h> //
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define debug

#define SIZE 2000000
#define CMDSIZE 10
#define ERROR_TIME 1
#define ERROR_CONNECTION 2
#define ERROR_READ 3
#define NO_ERROR 4
#define checkError(err)\
        if (err == ERROR_CONNECTION) break;\
        if (err == ERROR_TIME) { tv.tv_sec = 5; continue; }\
        if (err == ERROR_READ) continue;\

char message[SIZE];
char buf[SIZE];
char answer[CMDSIZE] = "answer";
//const char* preSend = "send";
char Ok[CMDSIZE] = "Ok";
char preSend[CMDSIZE];

int tcp_socket;
fd_set rfds;
timeval tv;
int size = 0;

void info() {
    tcp_info info;
    socklen_t l3 = (socklen_t)sizeof(tcp_info);
    int rcb = getsockopt(tcp_socket, SOL_TCP, TCP_INFO, (void*)&info, &l3);
    if (rcb == 0) {
        printf("tcpi_state: %d\n", info.tcpi_state);
        printf("info.tcpi_ca_state: %d\n", info.tcpi_ca_state);
        printf("info.tcpi_retransmits: %d\n", info.tcpi_retransmits);
        printf("info.tcpi_probes: %d\n", info.tcpi_probes);
        printf("info.tcpi_backoff: %d\n", info.tcpi_backoff);
        printf("info.tcpi_options: %d\n", info.tcpi_options);
        printf("info.tcpi_snd_wscale: %d\n", info.tcpi_snd_wscale);
        printf("tcpi_rcv_wscale: %d\n", info.tcpi_rcv_wscale);
        printf("info.tcpi_rto: %d\n", info.tcpi_rto);
        printf("info.tcpi_ato: %d\n", info.tcpi_ato);
        printf("info.tcpi_snd_mss: %d\n", info.tcpi_snd_mss);
        printf("info.tcpi_rcv_mss: %d\n", info.tcpi_rcv_mss);
        printf("info.tcpi_unacked: %d\n", info.tcpi_unacked);
        printf("info.tcpi_sacked: %d\n", info.tcpi_sacked);
        printf("info.tcpi_lost: %d\n", info.tcpi_lost);
        printf("info.tcpi_retrans: %d\n", info.tcpi_retrans);
        printf("info.tcpi_fackets: %d\n", info.tcpi_fackets);
        printf("info.tcpi_last_data_sent: %d\n", info.tcpi_last_data_sent);
        printf("info.tcpi_last_ack_sent: %d\n", info.tcpi_last_ack_sent);
        printf("info.tcpi_last_data_recv: %d\n", info.tcpi_last_data_recv);
        printf("info.tcpi_last_ack_recv: %d\n", info.tcpi_last_ack_recv);
        printf("info.tcpi_pmtu: %d\n", info.tcpi_pmtu);
        printf("info.tcpi_rcv_ssthresh: %d\n", info.tcpi_rcv_ssthresh);
        printf("info.tcpi_rtt: %d\n", info.tcpi_rtt);
        printf("info.tcpi_rttvar: %d\n", info.tcpi_rttvar);
        printf("info.tcpi_snd_ssthresh: %d\n", info.tcpi_snd_ssthresh);
        printf("info.tcpi_snd_cwnd: %d\n", info.tcpi_snd_cwnd);
        printf("info.tcpi_advmss: %d\n", info.tcpi_advmss);
        printf("info.tcpi_reordering: %d\n", info.tcpi_reordering);
        printf("info.tcpi_rcv_rtt: %d\n", info.tcpi_rcv_rtt);
        printf("info.tcpi_rcv_space: %d\n", info.tcpi_rcv_space);
        printf("info.tcpi_total_retrans: %d\n", info.tcpi_total_retrans);
    } else if (rcb == -1)
        printf("error info, errno: %d\n", errno);

}

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void checkSend(ssize_t n) {
    if (n == -1)
        perror("write");
    #ifdef debug
    printf("sent: %ld bytes\n", n);
    #endif
}

int readData(int &n, ssize_t &r, int& size, char (&refArray)[SIZE], timeval* tv) {
    FD_ZERO(&rfds);
    FD_SET(tcp_socket, &rfds);
    int retval = select(tcp_socket+1, &rfds, NULL, NULL, tv);
    if (retval) {
        if (FD_ISSET(tcp_socket,&rfds)) {
            r = read(tcp_socket, &refArray[n], size - n);
            if (r == -1) {
                perror("read");
                return ERROR_READ;
            }
            if (r == 0) {
                perror("connection");
                return ERROR_CONNECTION;
            }
            #ifdef debug
            printf("recv: %ld bytes\n", r);
            #endif
            return NO_ERROR;
        }
    } else if(retval == -1) {
        handle_error("select");
    } else {
        FD_ZERO(&rfds);
        printf("No data within five seconds.\n");
        return ERROR_TIME;
    }
}

int readAllData(int &n, int& size, char (&refArray)[SIZE], timeval* tv) {
    ssize_t r=0;
    while (1) {
        int err = readData(n, r, size, refArray, tv);
        if (err != NO_ERROR) return err;
        n+=r;
        if (n>=size) return NO_ERROR;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "%s <dotted-address-host> <dotted-address-server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1)
        handle_error("tcp_socket error");

    sockaddr_in my_addr, peer_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3308);
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(3307);

    if (inet_aton(argv[1], &my_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (inet_aton(argv[2], &peer_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
        handle_error("bind");

    if (connect(tcp_socket, (sockaddr *) &peer_addr, sizeof(sockaddr_in)) == -1)
        handle_error("connect");

    printf("connected!\n");

    tv.tv_sec = 5;
    tv.tv_usec = 0;


    //setsockopt getsockopt--------------------
    int rcb;
    int rcvbufsize_d = 100000;
    int rcvbufsize_;
    socklen_t l1 = (socklen_t)sizeof(int);
    socklen_t l2 = (socklen_t)sizeof(int);

//    rcb = setsockopt(tcp_socket, SOL_SOCKET, SO_SNDBUF, (void*)&rcvbufsize_d, l1);
//    if (rcb == -1)
//        printf("error rcvbufsize write, errno: %d\n", errno);
//    rcb = getsockopt(tcp_socket, SOL_SOCKET, SO_SNDBUF, (void*)&rcvbufsize_, &l1);
//    if (rcb == -1)
//        printf("error rcvbufsize, errno: %d\n", errno);
//    else if (rcb == 0)
//        printf("rcvbufsize: %d\n", rcvbufsize_);


//    rcb = setsockopt(tcp_socket, SOL_SOCKET, SO_RCVBUF, (void*)&rcvbufsize_d, l1);
//    if (rcb == -1)
//        printf("error rcvbufsize write, errno: %d\n", errno);
//    rcb = getsockopt(tcp_socket, SOL_SOCKET, SO_RCVBUF, (void*)&rcvbufsize_, &l1);
//    if (rcb == -1)
//        printf("error rcvbufsize, errno: %d\n", errno);
//    else if (rcb == 0)
//        printf("rcvbufsize: %d\n", rcvbufsize_);

    //--------------------
    int one = 0;
    int two;
    int three;
    int four;
//    rcb = setsockopt(tcp_socket, SOL_SOCKET, SO_DONTROUTE, (void*)&one, l2);
//    if (rcb == -1)
//        printf("error one, errno: %d\n", errno);
    rcb = getsockopt(tcp_socket, SOL_SOCKET, SO_DONTROUTE, (void*)&two, &l2);
    if (rcb == 0)
        printf("two: %d\n", two);
    else if (rcb == -1)
        printf("error two, errno: %d\n", errno);
    //--------------------
    rcb = setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, (void*)&one, l2);
    if (rcb == -1)
        printf("error one, errno: %d\n", errno);
    rcb = getsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, (void*)&three, &l2);
    if (rcb == 0)
        printf("three: %d\n", three);
    else if (rcb == -1)
        printf("error three, errno: %d\n", errno);
    //--------------------
//    rcb = setsockopt(tcp_socket, SOL_SOCKET, SO_PRIORITY, (void*)&four, l2);
//    if (rcb == -1)
//        printf("error four, errno: %d\n", errno);
    rcb = getsockopt(tcp_socket, SOL_SOCKET, SO_RCVLOWAT, (void*)&four, &l2);
    if (rcb == 0)
        printf("four: %d\n", four);
    else if (rcb == -1)
        printf("error four, errno: %d\n", errno);

    //--------------------
    info();
    //setsockopt getsockopt--------------------




    while (1) {
        int n=0;
        int recvCmdSize = sizeof(Ok); // or sizeof(answer) == CMDSIZE
        int err = readAllData(n, recvCmdSize, buf, NULL);
        checkError(err)
        if ((n == recvCmdSize) && (strcmp(buf, answer) == 0)) {
            //--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            sprintf(preSend, "%ld", (size_t)size);
            checkSend(write(tcp_socket, &preSend, sizeof(preSend)));
            //--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            checkSend(write(tcp_socket, &message, size));

            info();

        } else if (n == recvCmdSize) {
            size=atoi(buf);
            #ifdef debug
            printf("size: %d\n", size);
            #endif
            checkSend(write(tcp_socket, Ok, sizeof(Ok)));
            n=0;
            err = readAllData(n, size, message, &tv);
            checkError(err)
            checkSend(write(tcp_socket, Ok, sizeof(Ok)));

            info();
        }
    }

    if (close(tcp_socket) == -1)
        handle_error("close tcp socket");

    return 0;
}
