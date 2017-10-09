#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<string>
#include<unistd.h>
#include<iostream>

using namespace std;

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int fd;

void writeMessage(int option) {
    string command;
    string stringToSend;
    
    switch (option) {
        case 1:
            command = "c";
            break;
        case 2:
            command = "d";
            break;
        default:
            command = "h";
            break;
    }
    
    
    printf("Type in a short string to send to the crypto module:\n");
    getchar();
    getline(cin, stringToSend);
    command += stringToSend;
    printf("Writing message to the device [%s].\n", command.c_str());
    
    
    int ret = write(fd, command.c_str(), command.size()); // Send the string to the LKM
    if (ret < 0){
        perror("Failed to write the message to the device.");
        return;
    }
}

void readMessage() {
    printf("Reading from the device...\n");
    int ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
    if (ret < 0){
        perror("Failed to read the message from the device.");
        return;
    }
    printf("The received message is: [%s]\n", receive);
    printf("End of the program\n");
}

int main(){
    int option;
    printf("Starting device hyagoDev...\n");
    fd = open("/dev/hyagoDev", O_RDWR);             // Open the device with read/write access
    if (fd < 0){
        perror("Failed to open the device...");
        return errno;
    }
    
    printf("1 - Cipher\n2 - Decipher\n3 - Hash\n4-Read\n");
    scanf("%d", &option);
    if (option == 4) {
        readMessage();
    } else {
        writeMessage(option);
    }
    
    return 0;
}
