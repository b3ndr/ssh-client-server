#include "main.h"

using namespace std;

// just for easier input-output
struct IO {
    template <typename T>
    const IO & operator << (const T & t) const {
        std :: cout << t;
        return *this;
    }

    template <typename T>
    const IO & operator >> (T & t) const {
        std :: cin >> t;
        return *this;
    }
};

int main() {
    string login, password, command, message;
    char result[500];
    int socket_descriptor;
    sockaddr_in server;

    // set server config
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost
    server.sin_port = htons(8080); // port
    server.sin_family = AF_INET;

    // create socket and connect
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    connect(socket_descriptor, (sockaddr*)&server, sizeof(server));

    IO () << "Login:" >> login;
    IO () << "Password:" >> password;
    IO () << "Command:" >> command;

    message = login + " "+ password + "\n" + command;

    // send data to server
    send(socket_descriptor, message.c_str(), message.length(), 0);

    // get response
    recv(socket_descriptor, result, 1000, 0);
    cout << "Response:" << result << endl;

    return false;
}