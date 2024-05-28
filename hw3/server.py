import socket
import threading
import json

USER_FILE = 'users.json'
online_users = {}
client_sockets = []

def load_users():
    try:
        with open(USER_FILE, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        return {}

def save_users():
    with open(USER_FILE, 'w') as f:
        json.dump(online_users, f)

def broadcast_online_users():
    global online_users
    online_users_json = json.dumps(online_users).encode('utf-8')
    for sock in client_sockets:
        try:
            sock.send(online_users_json)
        except Exception as e:
            print(f"온라인 유저 정보 전송 실패 : {e}")

def handle_client(client_socket):
    global online_users, client_sockets
    client_sockets.append(client_socket)
    try:
        data = client_socket.recv(1024).decode('utf-8')
        user_info = json.loads(data)
        user_id = user_info['id']
        online_users[user_id] = user_info

        save_users()
        broadcast_online_users()

        while True:
            message = client_socket.recv(1024).decode('utf-8')
            if message == 'exit':
                break

        del online_users[user_id]
        save_users()
        broadcast_online_users()
    finally:
        client_sockets.remove(client_socket)
        client_socket.close()

def main():
    global online_users
    online_users = load_users()

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', 12345))
    server_socket.listen(5)

    print("로그인 서버가 실행중입니다 ... ")

    try:
        while True:
            client_socket, addr = server_socket.accept()
            print(f"{addr}로부터의 연결 허용")
            client_handler = threading.Thread(target=handle_client, args=(client_socket,))
            client_handler.start()
    finally:
        server_socket.close()

if __name__ == "__main__":
    main()
