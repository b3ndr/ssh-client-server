//
// Created by Роман Фурсов on 27/11/15.
//

#include "main.h"

using namespace std;


bool checkCredentials(string credsString) {
//  [ABSOLUTE PATH TO YOUR credentials.txt FILE]
    ifstream credsFile ("/home/b3ndr/Downloads/ssh-client-server/ssh-client-server/SSH-server/credentials.txt");

    if (credsFile.is_open()){
        string line;
        while(getline(credsFile,line)){
            line.append("\n");
            if (line.compare(credsString)==0){
                credsFile.close(); // found matched
                return true;
            }
        }
    } else {
        cout << "Can't open credentials file" << endl;
    }
    credsFile.close();
    return false;
}

void noBlockSocket(int id) {
    int flags, socketId;


    if (flags == -1) {
        cout << "fcntl error" << endl;
    }
    flags = fcntl(id, F_GETFL, 0);
//    flags |= flags or O_NONBLOCK;
    flags |= O_NONBLOCK;

    socketId = fcntl(id, F_SETFL, flags);
    if (flags == -1) {
        cout << "fcntl error" << endl;
    }
}

int createSocket(int port) {
    int socketId;

    sockaddr_in saddr;

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;

    socketId = socket(AF_INET, SOCK_STREAM, 0);
    if (socketId == -1) {
        cout << "Could not create socket" << endl;
    } else {
        if(bind(socketId, (sockaddr *)&saddr, sizeof(saddr)) < 0) {
            cout << "Port is unavailable" << endl;
            exit(1);
        }
    }

    noBlockSocket(socketId);

    return socketId;
}

string executeCommand(string command) {
    FILE* file = popen(command.c_str(), "r");
    char buffer[128];
    string result;

    if (!file) {
        cout << "Execution error" << endl;
    } else {
        while (fgets(buffer, sizeof(buffer), file)) {
            cout << buffer << endl;
            result += buffer;
        }
    }

    pclose(file);
    return result;
}

void proceedRequest(int request) {

    int numberOfString = 0;
    FILE* file_descriptor = fdopen(request, "r");
    size_t length;
    char* str;

    while (getline(&str, &length, file_descriptor) != -1) {
        // go trough request
        if (numberOfString == 0) {
            // login and password string
            if (!checkCredentials(str)) {
                cout << str << endl;
                string result = "Login failed";
                cout << result << endl;
                ssize_t response = send(request, result.c_str(), result.length(), 0); // send to client

                break;
            } else {
                cout << "Login successful" << endl;
            }
        } else {
            // command string
            string result = executeCommand(str);
            ssize_t response = send(request, result.c_str(), result.length(), 0);
        }

        numberOfString++;
    }

    fclose(file_descriptor);
    close(request);
}

int main() {
    epoll_event ev;
    epoll_event *events;
    int epoll_file_desc = epoll_create1(0);
    int file_desc = createSocket(8080);
    int socket = listen(file_desc, SOMAXCONN);

    ev.data.fd = file_desc;
    ev.events = EPOLLIN | EPOLLET;
    socket = epoll_ctl(epoll_file_desc, EPOLL_CTL_ADD, file_desc, &ev);

    events = (epoll_event*)calloc(64, sizeof(ev)); // 64 max events to retrieve

    cout << "Server is running" << endl;

    while (true) {
        int waiter;
        sockaddr sockaddr;
        socklen_t length;

        waiter = epoll_wait(epoll_file_desc, events, 64, -1);

        for (int i = 0; i < waiter; i++) {
            if ( (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)) || (events[i].events & EPOLLERR) ) {
                cout << "epoll error" << endl;
                close(events[i].data.fd);
            } else if (file_desc == events[i].data.fd) {
                while (true) {

                    length = sizeof(&sockaddr);

                    int id = accept(file_desc, &sockaddr, &length);

                    if (id == -1) {
                        break;
                    } else {
                        noBlockSocket(id);
                        ev.data.fd = id;
                        ev.events = EPOLLIN | EPOLLET;

                        socket = epoll_ctl(epoll_file_desc, EPOLL_CTL_ADD, id, &ev);
                        cout << "Socket connection is ready" << endl;
                    }
                }
            } else {

                // fork and execute
                int pid = fork();

                if (pid == 0) {
                    proceedRequest(events[i].data.fd);
                    return false;
                }
            }
        }
    }
}