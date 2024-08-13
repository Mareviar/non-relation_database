#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#pragma warning(disable: 4996)
using namespace std;
SOCKET Connection;

void ClientHandler() { // функция для принятия сообщений от сервера 
	int msg_size;
	while (true) { // цикл для принятия и вывода ответов от сервера
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		cout << msg << endl;
		delete[] msg;
	}
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Rus");
	//WSAStartup
	// представление информации о сетевом подключении
	WSAData wsaData; 
	WORD DLLVersion = MAKEWORD(2, 1); // выбираем нужную версию
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		cout << "Error" << endl;
		exit(1);
	}
		
	SOCKADDR_IN addr; // адрес сокета 
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");// преобразовываем строку в значение порядка байтов сети
	addr.sin_port = htons(6379);// преобразовать номер порта из порядка байтов хоста в целочисленное значение порядка байтов сети
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);// сокет прослушки инициализируем с параметрами tcp протокола
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) { // проверка на успешное соединение с сервером
		cout << "Error: failed connect to server.\n";
		return 1;
	}
	cout << "Connected!\n";

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL); // создаем поток для текущего соединения

	string msg1;
	while (true) { // цикл для ввода и отправки запроса на сервер
		getline(cin, msg1);
		int msg_size = msg1.size();
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, msg1.c_str(), msg_size, NULL);
		
	}

	system("pause");
	return 0;
}