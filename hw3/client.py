import socket
import threading
import json

online_users = {}
invited_users = {}

def receive_messages(sock):
    global online_users
    while True:
        data = sock.recv(1024).decode('utf-8')
        if not data:
            break
        try:
            online_users = json.loads(data)
            print("온라인 유저 업데이트 : ", online_users)
        except json.JSONDecodeError:
            print(f"받은 메세지 : {data}")

def connect_to_user(user_info):
    user_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    user_socket.connect((user_info['ip'], user_info['port']))
    
    threading.Thread(target=receive_messages, args=(user_socket,)).start()
    return user_socket

def main():
    global online_users
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('localhost', 12345))

    threading.Thread(target=receive_messages, args=(client_socket,)).start()

    user_id = input("아이디를 입력하세요 : ")
    user_ip = '127.0.0.1'
    user_port = int(input("포트 번호를 입력하세요 : "))

    user_info = {
        'id': user_id,
        'ip': user_ip,
        'port': user_port
    }

    client_socket.send(json.dumps(user_info).encode('utf-8'))

    listener_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener_socket.bind((user_ip, user_port))
    listener_socket.listen(5)

    def listen_for_messages():
        while True:
            conn, addr = listener_socket.accept()
            threading.Thread(target=receive_messages, args=(conn,)).start()

    threading.Thread(target=listen_for_messages).start()

    user_sockets = {}

    while True:
        command = input("커맨드를 입력하세요 - (invite/send/exit): ")
        if command == 'exit':
            client_socket.send(command.encode('utf-8'))
            break
        elif command.startswith('invite'):
            _, user_id_to_invite = command.split()
            if user_id_to_invite in online_users:
                if user_id_to_invite not in user_sockets:
                    user_sockets[user_id_to_invite] = connect_to_user(online_users[user_id_to_invite])
                invited_users[user_id_to_invite] = user_sockets[user_id_to_invite]
                print(f"{user_id_to_invite}와 연결되었습니다.")
        elif command.startswith('send'):
            _, message = command.split(maxsplit=1)
            for user_id, user_socket in invited_users.items():
                user_socket.send(message.encode('utf-8'))

    client_socket.close()

if __name__ == "__main__":
    main()
