#include "main.h"
#define NUMOFTREAD 2
/*
    1. прямое создание форка при подключении и переключение клиента на него
    2. создание пула заранее и сообщение эндпоинта клиенту
    3. сервер создает очередь заявок (связанный список или ipc). при коннекте клиента выставление 
    заявки в очередь. потоки изымают заявку из очереди и отвечают клиенту. 
*/

sem_t lock;

void* receiver(void* argc){
    char buffer[1024];
    int sd;
    while(1){
        sd = 0;
        list* tmp;
        while(1){
            sem_wait(&lock);
            if(tmp = getAt(0)) {
                sd = tmp->bk.sockdes;
                remove_at(0);
                sem_post(&lock);
                break;
            }   
            sem_post(&lock);
        }
        while(sd){
            for(int i = 0; i < sizeof buffer; i++){
                buffer[i] = 0;
            }
            ssize_t sz = read(sd, buffer, 1024);
            if(sz < 0){
                perror("read");
                break;
            }
            else if(sz == 0){ //костыль
                perror("read");
                break;
            }
            strcat(buffer, "+ ");
            if(send(sd, buffer, strlen(buffer), 0) < 0){
                perror("send");
                break;
            }
        }
        shutdown(sd, SHUT_RDWR);
        close(sd);
    }    
}

//setrlimit

int main(int argc, char* argv[]){
    pthread_t thrd[NUMOFTREAD];
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket not opened");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) < 0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.0.136");
    address.sin_port = htons(8090);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    sem_init(&lock, 0, 1);

    for(int i = 0; i < NUMOFTREAD; i++){
        pthread_create(&thrd[i], NULL, receiver, NULL);
    }

    if(listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while(1){
        book bk = {0};
        if((bk.sockdes = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pushBack(&bk);
    }    
    return 0;
}
