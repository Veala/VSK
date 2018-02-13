#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
//#include <rpc/auth_des.h>
#include <errno.h>

#define debug
#define debug_time

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

using namespace std;

char message[SIZE];
char buf[SIZE];
char answer[CMDSIZE] = "answer";
char Ok[CMDSIZE] = "Ok";
char preSend[CMDSIZE];

int tcp_socket, rw_socket;
fd_set rfds;
timeval tv, startTime, endTime;
streampos fsize;
int intfsize;

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

int readData(int &n, ssize_t &r, int& size) {
    FD_ZERO(&rfds);
    FD_SET(rw_socket, &rfds);
    int retval = select(rw_socket+1, &rfds, NULL, NULL, &tv);
    if (retval) {
        if (FD_ISSET(rw_socket,&rfds)) {
            r = read(rw_socket, &buf[n], size - n);
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

int readAllData(int &n, int& size) {
    ssize_t r=0;
    while (1) {
        int err = readData(n, r, size);
        if (err != NO_ERROR) return err;
        n+=r;
        if (n>=size) return NO_ERROR;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "%s <dotted-address> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ifstream file(argv[2], ios_base::binary | ios_base::ate);
    if (file.is_open()) {
        fsize = file.tellg();
        intfsize = fsize;
        if (fsize > SIZE) {
            cout << "File size more than 500K" << endl;
            return 1;
        }
        file.seekg (0, ios_base::beg);
        file.read(message, fsize);
        file.close();
    } else {
        cout << "Unable to open file" << endl;
        return 1;
    }

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_socket == -1)
        handle_error("tcp_socket error");

    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3307);

    if (inet_aton(argv[1], &my_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
        handle_error("bind");

    if (listen(tcp_socket, 100) == -1)
        handle_error("listen");

    printf("listening...\n");

    sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(sockaddr_in);

    FD_ZERO(&rfds);
    FD_SET(tcp_socket, &rfds);
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    int retval = select(tcp_socket+1, &rfds, NULL, NULL, &tv);
    if (retval) {
        if (FD_ISSET(tcp_socket,&rfds)) {
            printf("Connection is available now.\n");
            rw_socket = accept(tcp_socket, (sockaddr*) &peer_addr, &peer_addr_size);
            if (rw_socket == -1)
                handle_error("accept");
        } else {
            FD_ZERO(&rfds);
            printf("No connection.\n");
            return 1;
        }
    } else {
        FD_ZERO(&rfds);
        printf("No connection within five seconds.\n");
        return 1;
    }

    if (close(tcp_socket) == -1)
        handle_error("close tcp socket");

    //setsockopt getsockopt--------------------
    int rcb;
    int rcvbufsize_d = 163840;
    int rcvbufsize_d2 = 163840;
    int rcvbufsize_;
    socklen_t l1 = (socklen_t)sizeof(int);
    socklen_t l2 = (socklen_t)sizeof(int);

//    rcb = setsockopt(rw_socket, SOL_SOCKET, SO_SNDBUFFORCE, (void*)&rcvbufsize_d, l1);
//    if (rcb == -1)
//        printf("error sndbufsize write, errno: %d\n", errno);
    rcb = getsockopt(rw_socket, SOL_SOCKET, SO_SNDBUF, (void*)&rcvbufsize_, &l1);
    if (rcb == -1)
        printf("SO_SNDBUF rcvbufsize, errno: %d\n", errno);
    else if (rcb == 0)
        printf("SO_SNDBUF rcvbufsize: %d\n", rcvbufsize_);


//    rcb = setsockopt(rw_socket, SOL_SOCKET, SO_RCVBUFFORCE, (void*)&rcvbufsize_d2, l1);
//    if (rcb == -1)
//        printf("error rcvbufsize write, errno: %d\n", errno);
    rcb = getsockopt(rw_socket, SOL_SOCKET, SO_RCVBUF, (void*)&rcvbufsize_, &l1);
    if (rcb == -1)
        printf("SO_RCVBUF rcvbufsize, errno: %d\n", errno);
    else if (rcb == 0)
        printf("SO_RCVBUF rcvbufsize: %d\n", rcvbufsize_);
    //--------------------
    int one = -1;
    int two;
    int three;
    int four;

//    rcb = setsockopt(rw_socket, SOL_SOCKET, SO_DONTROUTE, (void*)&one, l2);
//    if (rcb == -1)
//        printf("error one, errno: %d\n", errno);
//    rcb = getsockopt(rw_socket, SOL_SOCKET, SO_DONTROUTE, (void*)&two, &l2);
//    if (rcb == 0)
//        printf("two: %d\n", two);
//    else if (rcb == -1)
//        printf("error two, errno: %d\n", errno);
    //--------------------
//    rcb = setsockopt(rw_socket, IPPROTO_TCP, TCP_NODELAY, (void*)&one, l2);
//    if (rcb == -1)
//        printf("error one, errno: %d\n", errno);
//    rcb = getsockopt(rw_socket, IPPROTO_TCP, TCP_NODELAY, (void*)&three, &l2);
//    if (rcb == 0)
//        printf("three: %d\n", three);
//    else if (rcb == -1)
//        printf("error three, errno: %d\n", errno);
//    //--------------------
//    rcb = setsockopt(rw_socket, SOL_SOCKET, SO_PRIORITY, (void*)&four, l2);
//    if (rcb == -1)
//        printf("error four, errno: %d\n", errno);
//    rcb = getsockopt(rw_socket, SOL_SOCKET, SO_RCVLOWAT, (void*)&four, &l2);
//    if (rcb == 0)
//        printf("four: %d\n", four);
//    else if (rcb == -1)
//        printf("error four, errno: %d\n", errno);

//    //--------------------
//    int claim = 100000;
//    rcb = setsockopt(rw_socket, SOL_TCP, TCP_WINDOW_CLAMP, (void*)&claim, l2);
//    if (rcb == -1)
//        printf("error three, errno: %d\n", errno);
//    rcb = getsockopt(rw_socket, SOL_TCP, TCP_WINDOW_CLAMP, (void*)&three, &l2);
//    if (rcb == 0)
//        printf("three: %d\n", three);
//    else if (rcb == -1)
//        printf("error three, errno: %d\n", errno);


    //--------------------
    tcp_info info;
    socklen_t l3 = (socklen_t)sizeof(tcp_info);
    rcb = getsockopt(rw_socket, SOL_TCP, TCP_INFO, (void*)&info, &l3);
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

    //irq
    //TCP_WINDOW_CLAMP
    //tcp_info
    //EISDIR
    //setsockopt getsockopt--------------------
    //SOL_TCP == IPPROTO_TCP

    string str;
    cout << "=>";
    while (getline(cin, str)) {
        if (str == "") {
			
        } else if (str == "exit") {
            if (close(rw_socket) == -1)
                handle_error("close rw socket");
            break;
        } else if (str == "send") {
            if (gettimeofday(&startTime, NULL) != 0)
                printf("Start time is fail\n");
#ifdef debug_time
            cout << startTime.tv_sec  << "s, " << startTime.tv_usec << "mks" << endl;
#endif
            sprintf(preSend, "%ld", (size_t)fsize);
            checkSend(write(rw_socket, &preSend, sizeof(preSend)));
            int recvCmdSize = sizeof(Ok), n=0;
            int err = readAllData(n, recvCmdSize);
            checkError(err)
            if (!((n == recvCmdSize) && (strncmp(buf, Ok, recvCmdSize) == 0))) {
                printf("\"Ok\" is not received\n");
                continue;
            }
            checkSend(write(rw_socket, &message, (size_t)fsize));
            n=0;
            err = readAllData(n, recvCmdSize);
            checkError(err)
            if (!((n == recvCmdSize) && (strncmp(buf, Ok, recvCmdSize) == 0))) {
                printf("\"Ok\" is not received\n");
                continue;
            }
            if (gettimeofday(&endTime, NULL) != 0)
                printf("End time is fail\n");
            long double time = (long double)(endTime.tv_sec - startTime.tv_sec) + (long double)(endTime.tv_usec - startTime.tv_usec)/1000000;
            long double speed = ((long double)intfsize)/((long double)(1024*1024)*time);
#ifdef debug_time
            cout << endTime.tv_sec  << "s, " << endTime.tv_usec << "mks" << endl;
            cout << time  << "s" << endl;
#endif
            printf("Speed in the send: %6.2Lf MByte/s\n", speed);
        } else if (str == "recv") {
            if (gettimeofday(&startTime, NULL) != 0)
                printf("Start time is fail\n");
#ifdef debug_time
            cout << startTime.tv_sec  << "s, " << startTime.tv_usec << "mks" << endl;
#endif

            checkSend(write(rw_socket, &answer, sizeof(answer)));
            int n=0;

            //--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            int recvCmdSize = sizeof(Ok); // or sizeof(answer) == CMDSIZE
            int err = readAllData(n, recvCmdSize);
            //--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            n=0;
            err = readAllData(n, intfsize);
            checkError(err)
            if (n == intfsize)
                printf("Data received\n");
            else
                printf("Too big data\n");

            if (gettimeofday(&endTime, NULL) != 0)
                printf("End time is fail\n");
            long double time = (long double)(endTime.tv_sec - startTime.tv_sec) + (long double)(endTime.tv_usec - startTime.tv_usec)/1000000;
            long double speed = ((long double)intfsize)/((long double)(1024*1024)*time);
#ifdef debug_time
            cout << endTime.tv_sec  << "s, " << endTime.tv_usec << "mks" << endl;
            cout << time  << "s" << endl;
#endif
            printf("Speed in the recv: %6.2Lf MByte/s\n", speed);

        } else if (str == "cmp") {
            if (memcmp(buf, message, intfsize) == 0)
                printf("Ok, buf == message\n");
            else
                printf("Failed, buf != message\n");
        } else if (str == "ch") {
            getline(cin, str);
            strcpy(message, str.c_str());
            message[str.size()]='\n';
            message[str.size()+1]='\0';
            fsize = str.size()+2;
            intfsize = fsize;
        } else if (str == "save") {
            ofstream file("file2.pdf", ios_base::binary);
            if (file.is_open()) {
                file.write(buf, fsize);
                file.close();
            } else {
                cout << "Unable to open file" << endl;
                return 1;
            }
        } else if (str == "info") {
            rcb = getsockopt(rw_socket, SOL_SOCKET, SO_SNDBUF, (void*)&rcvbufsize_, &l1);
            if (rcb == -1)
                printf("SO_SNDBUF rcvbufsize, errno: %d\n", errno);
            else if (rcb == 0)
                printf("SO_SNDBUF rcvbufsize: %d\n", rcvbufsize_);

            rcb = getsockopt(rw_socket, SOL_SOCKET, SO_RCVBUF, (void*)&rcvbufsize_, &l1);
            if (rcb == -1)
                printf("SO_RCVBUF rcvbufsize, errno: %d\n", errno);
            else if (rcb == 0)
                printf("SO_RCVBUF rcvbufsize: %d\n", rcvbufsize_);

            tcp_info info;
            socklen_t l3 = (socklen_t)sizeof(tcp_info);
            rcb = getsockopt(rw_socket, IPPROTO_TCP, TCP_INFO, (void*)&info, &l3);
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

        cout << "=>";
    }
	
    return 0;
}
