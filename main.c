#include "main.h"

/*
    1. прямое создание форка при подключении и переключение клиента на него
    2. создание пула заранее и сообщение эндпоинта клиенту
    3. сервер создает очередь заявок (связанный список или ipc). при коннекте клиента выставление 
    заявки в очередь. потоки изымают заявку из очереди и отвечают клиенту. 
*/

//setrlimit

int main(int argc, char* argv[]){

    int server_fd, new_socket;
    char state = 0 ;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };


    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("socket not opened");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.0.136");
    address.sin_port = htons(8080);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    pid_t newfork;
    int i = 0; 
    while(newfork){
        printf("created %d\n", i);
        if(i++ == 2) //nums of pool + 1
            break;
        newfork = fork();       
    }

    if(listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    if((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //printf("connected %d\n", new_socket);
    while(1){
        for(int i = 0; i < sizeof buffer; i++){
            buffer[i] = 0;
        }
        if(read(new_socket, buffer, 1024) < 0){
            perror("read");
            break;
        }
        printf("%s\n", buffer);
        strcat(buffer, "+ ");
        if(send(new_socket, buffer, strlen(buffer), 0) < 0){
            perror("send");
            break;
        }
    }
    printf("[%d]: socket closed", new_socket);
    close(new_socket);
    return 0;
}
