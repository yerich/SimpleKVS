import socket

def open_connection(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', port))
    return s

def main():
    socket = open_connection(6278)
    # send message via socket
    socket.send(b'test\n')

    # recieve and print out message
    message = socket.recv(1024).decode()
    print(message)

if __name__ == '__main__':
    main()
