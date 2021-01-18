#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>

using namespace std;

string ReadFileSearchMap(char map_id)
{
    ifstream mapfile;
    mapfile.open("map1.txt", ios::in); //stdin??
    char n;//all numbers and map_id are in char, represent by ascii
    string info;
    while(true)
    {
        n = mapfile.get();
        if(n == EOF)
        break;
        if(n == map_id)
        {
            n = mapfile.get();//jump over the '\n' that right after the map id
            bool f_ifis_nextgraph;
            bool f_ifis_ending;
            do{
                n = mapfile.get();
                info.push_back(n);
                f_ifis_nextgraph = !isalpha(n);
                f_ifis_ending = (n != EOF);
            }while(f_ifis_nextgraph && f_ifis_ending);
            if(!f_ifis_nextgraph)
            info.erase(info.end()-2, info.end());//pop the last two chars which is a '\n' and the id of next map
            else if(!f_ifis_ending)
            info.erase(info.end()-1, info.end());//pop the last char which is a '\n'
            break;
        }
    }

    mapfile.close();
    
    return info;//main() needs to check if info is empty
}




int main(){
    //create an UDP socket
//reuse from beej's guildline start in this line
    int serverA_sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in serverA_udp_addr;
    memset(&serverA_udp_addr, 0, sizeof(serverA_udp_addr));
    serverA_udp_addr.sin_family = AF_INET;
    serverA_udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverA_udp_addr.sin_port = htons(30156);
    bind(serverA_sock_udp, (struct sockaddr*)&serverA_udp_addr, sizeof(serverA_udp_addr));
//reuse from beej's guildline end in this line
    printf("The Server A is up and running using UDP on port <30156>\n\n");
    while(true){
        //recvfrom AWS
        char recv_buf[30];
        struct sockaddr_in src_addr;
        socklen_t src_addr_len = sizeof src_addr;
        recvfrom(serverA_sock_udp, recv_buf, sizeof(recv_buf)-1, 0,
                (struct sockaddr *)&src_addr, &src_addr_len);

        printf("The Server A has received input for finding graph of map <%c>.\n\n", recv_buf[0]);
        
        //search for the map
        char map_id = recv_buf[0];
        string map_detials = ReadFileSearchMap(map_id);

        //check if is in map1.txt
        bool yesIfind = !map_detials.empty();
        if(!yesIfind)
        printf("The Server A does not have the required graph id <%c>.\n\n",map_id);

        //send back the map data or "Graph not found"
        char send_buf[700];
        if(yesIfind){
            strncpy(send_buf, map_detials.c_str(), map_detials.length() + 1);
            sendto(serverA_sock_udp, send_buf, strlen(send_buf) + 1, 0, 
            (struct sockaddr*)&src_addr, sizeof(src_addr));
            printf("The server A has sent Grpah to AWS.\n\n");
        }
        else{
            send_buf[0] = 'n';//sending 'n' means graph not found
            sendto(serverA_sock_udp, send_buf, strlen(send_buf) + 1, 0, 
            (struct sockaddr*)&src_addr, sizeof(src_addr));
            printf("The Server A has sent \"Graph not Found\" to AWS.\n\n");
        }

    }

    close(serverA_sock_udp);

    return 0;
}