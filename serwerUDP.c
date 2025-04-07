#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

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

bool authoma(const char* text, int *count1, int *count2){
    *count1 = 0; 
    *count2 = 0;
    
    char* begin = NULL;
    char* end = NULL;
    int len = strlen(text);
    char state = 'I';
    
    if(len == 0){
        return true;
    }
    if(isspace(text[0]) || isspace(text[len - 1])){
        return false; 
    }
    
    for(int i = 0; i < len; ++i){
        char letter = text[i];
        
        if(!isalpha(letter) && !isspace(letter)){
            return false;
        }

        if(state == 'I'){
            if(isalpha(letter)){
                state = 'L'; 
                begin = (char*)&text[i];
            }else{
                state = 'E'; 
                break;
            }
        }else if (state == 'L'){
            if(isspace(letter)){ 
                end = (char*)&text[i - 1];
                if(is_palindrom(begin, end)){
                    (*count2)++; 
                }
                (*count1)++; 
                state = 'S'; 
            }
        }else if (state == 'S'){
            if(isalpha(letter)){
                begin = (char*)&text[i];
                state = 'L'; 
            }else if (!isspace(letter)){
                state = 'E'; 
                break;
            }
        }
    }

    if(state == 'L'){
        end = (char*)&text[len-1];
        (*count1)++;
        if (is_palindrom(begin, end)){
            (*count2)++;
        }
    }
    
    return state != 'E'; 
}


int main(int argc, char *argv[]){
    int sock;
    int rc;
    ssize_t cnt;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1){
        perror("Error, socket");
        exit(1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(2020)
    };

    rc = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(rc == -1){
        perror("Error, bind");
        close(sock);
        exit(2);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];
    char response[20];  
    bool keep_on_handling_clients = true;

    while(keep_on_handling_clients){
        memset(buffer, 0, sizeof(buffer));
        cnt = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if(cnt == -1){
            perror("Error, recvfrom");
            break;
        }
        bool valid_ending = false;
        if(cnt > 0 && buffer[cnt-1] == '\n'){
            if(cnt > 1 && buffer[cnt-2] == '\r'){
                buffer[cnt-2] = '\0';
                cnt -= 2;
                valid_ending = true;
            }else{
                buffer[cnt-1] = '\0';
                cnt--;
                valid_ending = true;
            }
        }else if(isalpha(buffer[cnt-1])){
            valid_ending = true;
        }
        
        buffer[cnt] = '\0';
        
        if(!valid_ending){
            snprintf(response, sizeof(response), "ERROR");
        }else if(cnt == 0){
            snprintf(response, sizeof(response), "0/0");
        }else{
            int count1 = 0;
            int count2 = 0;
            bool results = authoma(buffer, &count1, &count2);
            
            if(results == false){
                snprintf(response, sizeof(response), "ERROR");
            }else if(count1 == 0){
                snprintf(response, sizeof(response), "0/0");
            }else{
                snprintf(response, sizeof(response), "%d/%d", count2, count1);
            }
        }
        
        cnt = sendto(sock, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_addr_len);
        if(cnt == -1){
            perror("Error, sendto");
        }
    }
        
    rc = close(sock);
    if(rc == -1){
        perror("Error, close");
        exit(3);
    }
    return 0;
}
