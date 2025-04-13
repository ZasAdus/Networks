#define _GNU_SOURCE
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define MAX_CLIENTS 255
#define MAX_LINE 1024


bool is_palindrom(char* begin, char* end){
    while(begin < end){
        if(tolower(*begin) != tolower(*end)){
            return false;
        }
        begin++;
        end--;
    }
    return true;
}


bool authoma(const char* text, int *count1, int *count2, size_t real_size){
    *count1 = 0; 
    *count2 = 0;
    char* begin = NULL;
    char* end = NULL;
    size_t len = strlen(text);
    char state = 'I';
    if(real_size > len){
        return false;
    }
    if(len == 0){
        return true;
    }
    if(isspace(text[0]) || isspace(text[len - 1])){
        return false; 
    }
    for(size_t i = 0; i < len; ++i){
        char letter = text[i];
        if((!isalpha(letter) && !isspace(letter))){
            return false;
        }
        if(state == 'I'){
            if(isalpha(letter)){
                state = 'L'; 
                begin =(char*)&text[i];
            }else{
                state = 'E'; 
                break;
            }
        }else if(state == 'L'){
            if(isspace(letter)){ 
                end =(char*)&text[i - 1];
                if(is_palindrom(begin, end)){
                  (*count2)++; 
                }
                (*count1)++; 
                state = 'S'; 
            }
        }else if(state == 'S'){
            if(isalpha(letter)){
                begin =(char*)&text[i];
                state = 'L'; 
            }else if(!isspace(letter)){
                state = 'E'; 
                break;
            }
        }
    }
    if(state == 'L'){
        end =(char*)&text[len-1];
      (*count1)++;
        if(is_palindrom(begin, end)){
          (*count2)++;
        }
    }
    return state != 'E'; 
}

int listening_socket_tcp_ipv4(){
    int s;
    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Error, socket");
        exit(1);
    }
    struct sockaddr_in a ={
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(2020)
    };
    if(bind(s,(struct sockaddr *) &a, sizeof(a)) == -1){
        perror("Error, bind");
        exit(1);
    }
    if(listen(s, 10) == -1){
        perror("Error, listen");
        exit(1);
    }
    return s;
}

typedef struct{
    int fd;
    char buffer[MAX_LINE];
    int buffer_len;
}Client;



void safe_close(int fd){
    if(close(fd) == -1){
        perror("Error, close");
    }
}


int main(){
    Client clients[MAX_CLIENTS];
    int server_fd = listening_socket_tcp_ipv4();
    int epfd = epoll_create1(0);
    if(epfd == -1){
        perror("Error, epoll_create1");
        close(server_fd);
        exit(1);
    }
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1){
        perror("Error, epoll_ctl");
        safe_close(epfd);
        safe_close(server_fd);
        exit(1);
    }
    while(1){
        int number_of_fds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if(number_of_fds == -1){
            perror("Error, epoll_wait");
            break;
        }
        for(int i = 0; i < number_of_fds; ++i){
            int fd = events[i].data.fd;
            if(fd == server_fd){
                int client_fd = accept(server_fd, NULL, NULL);
                if(client_fd < 0){
                    perror("Error, accept");
                    continue;
                }
                if(fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1){
                    perror("Error, fcntl");
                    safe_close(client_fd);
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if(epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev) == -1){
                    perror("Error, epoll_ctl");
                    safe_close(client_fd);
                    continue;
                }
                clients[client_fd].fd = client_fd;
                clients[client_fd].buffer_len = 0;
            }else{
                char recv_buf[512];
                while(1){
                    ssize_t n = read(fd, recv_buf, sizeof(recv_buf));
                    if(n == -1){
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            break;
                        }else{
                            perror("Error, read");
                            safe_close(fd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            clients[fd].fd = -1;
                            break;
                        }
                    }else if(n == 0){
                        safe_close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        clients[fd].fd = -1;
                        break;
                    }
                    Client* c = &clients[fd];
                    if(c->buffer_len + n >= sizeof(c->buffer)){
                        write(fd, "ERROR\r\n", 7);
                        c->buffer_len = 0;
                        continue;
                    }
                    memcpy(c->buffer + c->buffer_len, recv_buf, n);
                    c->buffer_len += n;
                    int processed = 0;
                    while(1){
                        char* newline = memmem(c->buffer + processed, c->buffer_len - processed, "\r\n", 2);
                        if(!newline)
                            break;
                        int line_len = newline -(c->buffer + processed);
                        if(line_len >= MAX_LINE){
                            write(fd, "ERROR\r\n", 7);
                            processed += line_len + 2;
                            continue;
                        }
                        char temp[MAX_LINE];
                        memcpy(temp, c->buffer + processed, line_len);
                        temp[line_len] = '\0';
                        char response[32];
                        int count1, count2;
                        if(!authoma(temp, &count1, &count2, line_len)){
                            snprintf(response, sizeof(response), "ERROR\r\n");
                        }else if(count1 == 0){
                            snprintf(response, sizeof(response), "0/0\r\n");
                        }else{
                            snprintf(response, sizeof(response), "%d/%d\r\n", count2, count1);
                        }
                        if(write(fd, response, strlen(response)) == -1){
                            perror("Error, write");
                            break;
                        }
                        processed += line_len + 2;
                    }
                    if(processed > 0){
                        memmove(c->buffer, c->buffer + processed, c->buffer_len - processed);
                        c->buffer_len -= processed;
                    }
                }
            }
        }
    }
    safe_close(server_fd);
    safe_close(epfd);
    return 0;
}