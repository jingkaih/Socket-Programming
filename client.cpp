#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "vector"


using namespace std;

struct QueryInfo{
    char map_id;
    int starting;
    int destination;
    int filesize;
};

struct Answer{
    double tp, tt, delay;
    double len;
    vector<int> path;
};


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


int main(int argc, char *argv[])
{

    //create socket
//reuse from beej's guildline start in this line
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("The client is up and running.\n\n");
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;//IPv4
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//lo
    serv_addr.sin_port = htons(34156);//port# my USCid
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//reuse from beej's guildline end in this line
    struct QueryInfo query;
    query.map_id = *argv[1];
    query.starting = atoi(argv[2]);
    query.destination = atoi(argv[3]);
    query.filesize = atoi(argv[4]);



    //send the query, in a pattern of "A#1#2#1024"
    int query_len = 0;
    for(int i = 1; i < argc; i++)
    query_len += strlen(argv[i]);
    query_len += 3;//3 delimiters '#'
    char msg[query_len];
    strcpy(msg, argv[1]);
    for(int i = 2; i < argc; i++)
    {
        strcat(msg, "#");
        strcat(msg, argv[i]);
    }
    
    send(sock, msg, query_len + 1, 0);
    
    printf("The client has sent query to AWS using TCP:\nStart vertex %d; Destination vertex %d; Map %c; File size %d.\n\n", query.starting, query.destination, query.map_id, query.filesize);
    //receive from AWS
    char recv_buf[700];
    struct Answer ans;
    read(sock, recv_buf, sizeof(recv_buf) - 1);
    if(recv_buf[0] == 'm')//no map found
    printf("No map id %c found.\n\n", query.map_id);
    else if(recv_buf[0] == 's')//source not found
    printf("No vertex id %d found.\n\n", query.starting);
    else if(recv_buf[0] == 'd')//destination not found
    printf("No vertex id %d found.\n\n", query.destination);
    else if(recv_buf[0] == 'b')//both are not found
    printf("No vertex id %d and %d found.\n\n", query.starting, query.destination);
    else{
        ans = DataProcessing(recv_buf);
        printf("The client has received results from AWS:\n");
        for(int i = 0; i < 80; i++)
        printf("-");
        printf("\nSource Destination    Min Length    Tt    Tp      Delay\n");
        for(int i = 0; i < 80; i++)
        printf("-");
        printf("\n%d     %d             %.2f      %.2f   %.2f   %.2f\n", query.starting, query.destination, ans.len, ans.tt, ans.tp, ans.delay);
        for(int i = 0; i < 80; i++)
        printf("-");
        printf("\nShortest path: ");
        for(int i = 0; i < ans.path.size(); i++){
            printf("%d",ans.path.at(i));
            if(i != ans.path.size() - 1)
            printf(" -- ");
            else
            printf("\n\n");
        }
    }

    close(sock);

    return 0;
}