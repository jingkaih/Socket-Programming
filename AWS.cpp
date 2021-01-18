#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "vector"

using namespace std;

struct QueryInfo{
    char map_id;
    int starting;
    int destination;
    int filesize;
};

struct GraphInfo{
    double sp;
    int st;
    double matrix[99][99];
};

struct Answer{
    double tp, tt, delay;
    double len;
    vector<int> path;
};


QueryInfo getinfo(char *p){
    struct QueryInfo myinfo;
    myinfo.map_id = *p;
    p = p + 2;//now pointing to the start vertex
    myinfo.starting = atoi(p);
    do{
        p++;
    }
    while((*p)!='#');
    p++;//p is not pointing to the destination vertex
    myinfo.destination = atoi(p);
    do{
        p++;
    }
    while((*p)!='#');
    p++;//p is not pointing to the file size
    myinfo.filesize = atoi(p);
    return myinfo;
}

GraphInfo GraphProcessing(char *p)
{
    struct GraphInfo mygraph;
    mygraph.sp = atof(p);

    //get the transmission speed
    do{
        p++;
    }while(*p != '\n');
    mygraph.st = atoi(p);

    //get the edges info
    //initialize
    for(int i = 0; i < 99; i++){
        for(int j = 0; j < 99; j++){
            mygraph.matrix[i][j] = 10000;
        }
    }
    
    do{
        p++;
    }while(*p != '\n');

    while(*p != '\0')
    {
        int start, end;
        double dis;
        start = atoi(p);
        do{
            p++;
        }while(*p != ' ');
        end = atoi(p);
        do{
            p++;
        }while(*p != ' ');
        dis = atof(p);
        do{
            p++;
        }while(*p != '\n' && *p != '\0');

        mygraph.matrix[start][end] = dis;
        mygraph.matrix[end][start] = dis;
        mygraph.matrix[start][start] = 0;
        mygraph.matrix[end][end] = 0;
    }
    return mygraph;
}

Answer DataProcessing(char *p){
    struct Answer ans;

    ans.len = atof(p);
    do{
        p++;
    }while(*p != '\n');

    ans.tt = atof(p);
    do{
        p++;
    }while(*p != '\n');

    ans.tp = atof(p);
    do{
        p++;
    }while(*p != '\n');

    ans.delay = atof(p);
    do{
        p++;
    }while(*p != '\n');

    do{
        ans.path.push_back(atoi(p));
        do{
            p++;
        }while(*p != '\n' && *p != '\0');
    }while(*p != '\0');

    return ans;
}



int main(){
//reuse from beej's guildline start in this line
    //create TCP socket
    int aws_sock_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    

    struct sockaddr_in aws_tcp_addr;
    memset(&aws_tcp_addr, 0, sizeof(aws_tcp_addr));
    aws_tcp_addr.sin_family = AF_INET;
    aws_tcp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    aws_tcp_addr.sin_port = htons(34156);
    bind(aws_sock_tcp, (struct sockaddr*)&aws_tcp_addr, sizeof(aws_tcp_addr));

//reuse from beej's guildline end in this line

    printf("The AWS is up and running.\n\n");
    //listening to client, waiting for connection initializing
    while(true){
        listen(aws_sock_tcp, 20);

        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        int clnt_sock = accept(aws_sock_tcp, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        
        //receive the query from client
        char buf[30];
        recv(clnt_sock, buf, 30, 0);

        //process the query
        struct QueryInfo query = getinfo(buf);//call getinfo(), expecting a struct that stores the info
        printf("The AWS has received map ID: %c,\nstart vertex: %d, destination vertex: %d\nand file size: %d from the client using TCP over port #34156.\n\n", query.map_id,query.starting,query.destination,query.filesize);

        //send map id to serverA
//reuse from beej's guildline start in this line
        //create UDP socket
        int aws_sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        //AWS address info
        struct sockaddr_in aws_udp_addr;
        memset(&aws_udp_addr, 0, sizeof(aws_udp_addr));
        aws_udp_addr.sin_family = AF_INET;
        aws_udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        aws_udp_addr.sin_port = htons(33156);
        //bind
        bind(aws_sock_udp, (struct sockaddr*)&aws_udp_addr, sizeof(aws_udp_addr));
        
        //serverA address info
        struct sockaddr_in serverA;
        memset(&serverA, 0, sizeof(serverA));
        serverA.sin_family = AF_INET;
        serverA.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverA.sin_port = htons(30156);

        //serverB address info
        struct sockaddr_in serverB;
        memset(&serverB, 0, sizeof(serverB));
        serverB.sin_family = AF_INET;
        serverB.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverB.sin_port = htons(31156);

        //serverC address info
        struct sockaddr_in serverC;
        memset(&serverC, 0, sizeof(serverC));
        serverC.sin_family = AF_INET;
        serverC.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverC.sin_port = htons(32156);
//reuse from beej's guildline end in this line


        //sendto server A
        char send_buf_serverA[20];
        send_buf_serverA[0] = query.map_id;
        sendto(aws_sock_udp, send_buf_serverA, 2/*strlen(send_buf_serverA) + 1*/, 0, 
                (struct sockaddr*)&serverA, sizeof(serverA));
        printf("The AWS has sent map ID to server A using UDP over port #33156.\n\n");

        //recvfrom server A
        char recv_buf_serverA[700];
        char recv_buf_serverB[700];
        char recv_buf_serverC[700];
        struct GraphInfo mygraph;
        struct sockaddr_storage src_addr_serverA;
        socklen_t src_addr_serverA_len = sizeof src_addr_serverA;
        recvfrom(aws_sock_udp, recv_buf_serverA, sizeof(recv_buf_serverA)-1, 0,
                (struct sockaddr *)&src_addr_serverA, &src_addr_serverA_len);

        //check if map AWS required is in server A
        bool foundinA = (recv_buf_serverA[0] != 'n');
        bool foundinB = false;
        if(!foundinA)
        {
            //AWS is about to ask for server B
            char send_buf_serverB[20];
            send_buf_serverB[0] = query.map_id;
            sendto(aws_sock_udp, send_buf_serverB, 2/*strlen(send_buf_serverB) + 1*/, 0, 
                (struct sockaddr*)&serverB, sizeof(serverB));
            printf("The AWS has sent map ID to server B using UDP over port #33156.\n\n");

            //AWS is about to recvfrom server B
            struct sockaddr_storage src_addr_serverB;
            socklen_t src_addr_serverB_len = sizeof src_addr_serverB;
            recvfrom(aws_sock_udp, recv_buf_serverB, sizeof(recv_buf_serverB)-1, 0,
                    (struct sockaddr *)&src_addr_serverB, &src_addr_serverB_len);
            
            //check if required map is in server B
            foundinB = (recv_buf_serverB[0] != 'n');
        }

        //Did AWS receive the graph info?
        char send_buf_clnt[50];
        if((foundinA == false) && (foundinB == false)){
            //send message "I couldn't find required map" to client
            send_buf_clnt[0] = 'm';//"no corresponding map"
            send(clnt_sock, send_buf_clnt, strlen(send_buf_clnt) + 1, 0);
        }
        else{//I find graph in A but can I find vertices?
            if(foundinA){
                mygraph = GraphProcessing(recv_buf_serverA);
                printf("The AWS has received map information from server A.\n\n");
            }
            else if(foundinB){
                mygraph = GraphProcessing(recv_buf_serverB);
                printf("The AWS has received map information from server B.\n\n");
            }
            
            //can I find the vertices?
            bool starting_found = false;
            bool destination_found = false;
            for(int i = 0; i < 99; i++){
                if(i != query.starting){
                    if(mygraph.matrix[i][query.starting] < 10000){
                        starting_found = true;
                        break;
                    }
                }
            }
            for(int i = 0; i < 99; i++){
                if(i != query.destination){
                    if(mygraph.matrix[i][query.destination] < 10000){
                        destination_found = true;
                        break;
                    }
                }
            }


            if(!starting_found && destination_found){
                printf("Source vertex not found in the graph, sending error to client using TCP over port #34156.\n\n");
                send_buf_clnt[0] = 's';//no source
            }
            else if(starting_found && !destination_found){
                printf("Destination vertex not found in the graph, sending error to client using TCP over port #34156.\n\n");
                send_buf_clnt[0] = 'd';//no destination
            }
            else if(!starting_found && !destination_found){
                printf("Both source and destination vertex not found in the graph,\nsending error to client using TCP over port #34156.\n\n");
                send_buf_clnt[0] = 'b';//no any
            }
            else{//vertices exist

                printf("The source and destination vertex are in the graph.\n\n");

                //send the graph to server C
                char send_buf_serverC[700];
                memset(send_buf_serverC, '\0', sizeof(send_buf_serverC));
                char sr[2];
                char de[2];
                char fz[7];
                sprintf(sr,"%d",query.starting);
                sprintf(de,"%d",query.destination);
                sprintf(fz,"%d",query.filesize);
                send_buf_serverC[0] = query.map_id;
                strcat(send_buf_serverC, " ");
                strcat(send_buf_serverC, sr);
                strcat(send_buf_serverC, " ");
                strcat(send_buf_serverC, de);
                strcat(send_buf_serverC, " ");
                strcat(send_buf_serverC, fz);
                strcat(send_buf_serverC, "\n");

                if(foundinA)
                strcat(send_buf_serverC, recv_buf_serverA);
                else if(foundinB)
                strcat(send_buf_serverC, recv_buf_serverB);


                sendto(aws_sock_udp, send_buf_serverC, strlen(send_buf_serverC) + 1, 0, 
                    (struct sockaddr*)&serverC, sizeof(serverC));
                printf("The AWS has sent map, source ID, destination ID, propagation speed and transmission speed to server C using UDP over port #33156.\n\n");

                //AWS is about to recvfrom server C
                struct sockaddr_storage src_addr_serverC;
                socklen_t src_addr_serverC_len = sizeof src_addr_serverC;
                recvfrom(aws_sock_udp, recv_buf_serverC, sizeof(recv_buf_serverC)-1, 0,
                        (struct sockaddr *)&src_addr_serverC, &src_addr_serverC_len);
                
                //processing the data from server C
                struct Answer ans;
                ans = DataProcessing(recv_buf_serverC);
                printf("The AWS has received results from server C:\nShortest path:\n");
                for(int i = 0; i < ans.path.size(); i++){
                    printf("%d",ans.path.at(i));
                    if(i != ans.path.size() - 1)
                    printf(" -- ");
                    else
                    printf("\n");
                }
                printf("Shortest distance: %.2f km\nTransmission delay: %.2f s\nPropagation delay: %.2f s\n\n", ans.len, ans.tt, ans.tp);
                strcpy(send_buf_clnt, recv_buf_serverC);
                printf("The AWS has sent calculated results to client using TCP over port #34156.\n\n");
                
            }
            
            //send error or actual data to client

            send(clnt_sock, send_buf_clnt, strlen(send_buf_clnt) + 1, 0);
            
        }

        close(aws_sock_udp);
        close(clnt_sock);

    }

    close(aws_sock_tcp);//parent socket never closed

    return 0;
}