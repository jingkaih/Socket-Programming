#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include "vector"
#include "queue"
#include "map"
#include <limits>

using namespace std;

struct GraphInfo{
    char id;
    int src;
    int des;
    int fz;
    double sp;
    int st;
    double matrix[99][99];
};

struct Answer{
    double tp, tt, delay;
    double len;
    vector<int> path;
};


GraphInfo GraphProcessing(char *p)
{
    struct GraphInfo mygraph;
    mygraph.id = *p;
    p = p + 2;
    mygraph.src = atoi(p);
    do{
        p++;
    }while(*p != ' ');
    mygraph.des = atoi(p);
    do{
        p++;
    }while(*p != ' ');
    mygraph.fz = atoi(p);
    do{
        p++;
    }while(*p != '\n');
    mygraph.sp = atof(p);
    do{
        p++;
    }while(*p != '\n');
    mygraph.st = atoi(p);

    //get the edges info
    //initialize
    for(int i = 0; i < 99; i++){
        for(int j = 0; j < 99; j++){
            if(i == j)
            mygraph.matrix[i][j] = 0;
            else
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
    }
    return mygraph;
}

Answer ShortestPath(GraphInfo graph)
{
    struct Answer ans;
    int src = graph.src;
    int des = graph.des;
    int filesize = graph.fz;
    bool isconnect[100];//check connectivity of each node
    bool visited[100];
    double cur_dis[100];//store the distance from source
    int prev_vert[100];//store the previous node
    priority_queue< pair<double, int>, vector<pair<double, int> >, greater<pair< double, int > > > q;//pair<distance, vertex>


    for(int i = 0; i < 99; i++){
        if(graph.matrix[i][i] == 0)
        isconnect[i] = true;
        else
        isconnect[i] = false;
    }

    for(int i = 0; i < 99; i++)//all vertices are unvisited
    visited[i] = false;


    for(int i = 0; i < 99; i++)
    cur_dis[i] = numeric_limits<double>::max();//all vertices' distance to source is set to double_max

    cur_dis[src] = 0;//except the source itself
    q.push(make_pair(cur_dis[src], src));


    while(!q.empty())
    {
        pair<double, int> cur_vert = q.top();
        q.pop();
        if(cur_vert.first > cur_dis[cur_vert.second])
        continue;
    
        visited[cur_vert.second] = true;

        for(int i = 0; i < 99; i++){
            if(isconnect[i] && (!visited[i])){
                double s = graph.matrix[cur_vert.second][i];
                if(s < 10000 && s != 0){//i is index of vertices that connect to cur_vert and not yet been visited, s is the distance of cur_vert to i
                    if(cur_dis[cur_vert.second] + s < cur_dis[i]){//is renewal neccessary?
                        q.push(make_pair(cur_dis[cur_vert.second] + s, i));//push the pair to priority queue
                        cur_dis[i] = cur_dis[cur_vert.second] + s;//renew cur_dis
                        prev_vert[i] = cur_vert.second;//record the previous vertex
                    }
                    
                }
            }

        }
    }
    ans.len = cur_dis[des];
    vector<int> path_rev, path;
    int it = des;
    while(true)
    {
        path_rev.push_back(it);
        if(it != src)
        it = prev_vert[it];
        else
        break;
    }
    for(int i = path_rev.size() - 1; i >= 0; i--)
    path.push_back(path_rev.at(i));

    ans.path = path;
    ans.tp = cur_dis[des] / graph.sp;
    ans.tt = (double)filesize / (double)graph.st;
    ans.delay = cur_dis[des] / graph.sp + (double)filesize / (double)graph.st;


    return ans;

}

int main(){
    //create an UDP socket
//reuse from beej's guildline start in this line
    int serverC_sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in serverC_udp_addr;
    memset(&serverC_udp_addr, 0, sizeof(serverC_udp_addr));
    serverC_udp_addr.sin_family = AF_INET;
    serverC_udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverC_udp_addr.sin_port = htons(32156);
    bind(serverC_sock_udp, (struct sockaddr*)&serverC_udp_addr, sizeof(serverC_udp_addr));
//reuse from beej's guildline end in this line
    printf("The Server C is up and running using UDP on port <32156>\n\n");

    while(true){
        //recvfrom AWS
        char recv_buf[700];
        struct sockaddr_in src_addr;
        socklen_t src_addr_len = sizeof src_addr;
        recvfrom(serverC_sock_udp, recv_buf, sizeof(recv_buf)-1, 0,
                (struct sockaddr *)&src_addr, &src_addr_len);


        struct GraphInfo graph = GraphProcessing(recv_buf);
        printf("The Server C has received data for calculation: \n* Propagation speed: %f km/s; \n* Transmission speed %d KB/s; \n* map ID: %c; \n* Source ID: %d    Destination ID: %d; \n\n", graph.sp, graph.st, graph.id, graph.src, graph.des);
        struct Answer ans = ShortestPath(graph);
        printf("The Server C has finished the calculation:\nShortest path: ");
        for(int i = 0; i < ans.path.size(); i++){
            printf("%d",ans.path.at(i));
            if(i != ans.path.size() - 1)
            printf(" -- ");
            else
            printf("\n");
        }
        printf("Shortest distance: %f km\nTransmission delay: %f s\nPropagation delay: %f s\n\n", ans.len, ans.tt, ans.tp);
        
        //send back to aws
        char send_buf[700];
        memset(send_buf, '\0', sizeof(send_buf));
        char len[16];
        char tt[16];
        char tp[16];
        char delay[16];
        sprintf(len,"%f",ans.len);
        sprintf(tt,"%f",ans.tt);
        sprintf(tp,"%f",ans.tp);
        sprintf(delay,"%f",ans.delay);
        strcpy(send_buf, len);
        strcat(send_buf, "\n");
        strcat(send_buf, tt);
        strcat(send_buf, "\n");
        strcat(send_buf, tp);
        strcat(send_buf, "\n");
        strcat(send_buf, delay);
        strcat(send_buf, "\n");
        for(int i = 0; i < ans.path.size(); i++){
            char ve[2];
            sprintf(ve,"%d",ans.path.at(i));
            strcat(send_buf, ve);
            if(i < ans.path.size() - 1)
            strcat(send_buf, "\n");
            else
            strcat(send_buf, "\0");
        }

        sendto(serverC_sock_udp, send_buf, strlen(send_buf) + 1, 0, 
            (struct sockaddr*)&src_addr, sizeof(src_addr));

        printf("The Server C has finished sending the output to AWS.\n\n");

        
    }
    close(serverC_sock_udp);

    return 0;
}