#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <mutex>
#pragma warning(disable: 4996)
using namespace std;
SOCKET Connections[100];
int Counter = 0;

mutex mute;
//-------------------------------------------------------------------------БАЗА ДАННЫХ
//-------------------------------------------СТЕК
struct stnode {
	string val;
	stnode* next;
};

struct Stack {
	stnode* head;
	int size = 0;
};

void initstack(Stack& st) {
	st.head = new stnode;
	st.head->val = "";
	st.size = 0;
}

void push(Stack& st, string val) {
	stnode* node = new stnode;
	node->val = val;
	if (st.size == 0) {
		st.head = node;
		st.size++;
		cout << "Oбработан запрос SPUSH." << " " << endl;
	}
	else {
		stnode* node = new stnode;
		node->val = val;
		node->next = st.head;
		st.head = node;
		st.size++;
		cout << "Oбработан запрос SPUSH." << " " << endl;
	}
}

string pop(Stack& st, int index) {
	if (st.size == 0) {
		string msg = "стек пуст";
		int msg_size = msg.size();
		send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[index], msg.c_str(), msg_size, NULL);
		cout << "Oбработан запрос SPOP." << " " << endl;
	}
	else {
		string msg = st.head->val;
		st.head = st.head->next;
		st.size--;
		int msg_size = msg.size();
		send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[index], msg.c_str(), msg_size, NULL);
		cout << "Oбработан запрос SPOP." << " " << endl;
		return msg;
	}
}


//-----------------------------------------------queue
struct elementqueue {
	string value;
	elementqueue* prev;
	elementqueue* next;
};

struct Queue {
	elementqueue* beginBlock;
	elementqueue* endBlock;
	int size;
};

void createQueue(Queue& obj) {
	obj.beginBlock = new elementqueue;
	obj.endBlock = new elementqueue;
	(*obj.beginBlock).next = obj.endBlock;
	(*obj.endBlock).prev = obj.beginBlock;
	obj.size = 0;
}

void pushToQueue(Queue& obj, string element) {
	elementqueue* newElement = new elementqueue;
	elementqueue* wasElement = (*obj.endBlock).prev;
	(*obj.endBlock).prev = newElement;
	(*wasElement).next = newElement;
	(*newElement).next = obj.endBlock;
	(*newElement).value = element;
	obj.size++;
	cout << "Oбработан запрос QPUSH." << " " << endl;
}

void popFromQueue(Queue& obj, int index) {
	if (obj.size == 0) {
		string mess = "ОЧЕРЕДЬ ПУСТА";
		int msg_size = mess.size();
		send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[index], mess.c_str(), msg_size, NULL);
		cout << "Oбработан запрос QPOP." << " " << endl;
	}
	else {
		elementqueue* nowElement = (*obj.beginBlock).next;
		string neededElement = (*nowElement).value;
		elementqueue* nextElement = (*nowElement).next;
		(*nextElement).prev = obj.beginBlock;
		(*obj.beginBlock).next = nextElement;
		delete nowElement;
		obj.size--;
		int msg_size = neededElement.size();
		send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[index], neededElement.c_str(), msg_size, NULL);
		cout << "Oбработан запрос QPOP." << " " << endl;
	}
}

//-------------------------set
typedef struct {
	string key;

}Node_map;

typedef struct {
	string keys[512];

}map;


int hash_function(string str) {
	int i = 0;
	for (int j = 0; j < str.length(); j++) {
		i += str[j];
	}
	i = i % 512;
	return i;
}


void add_map(string key, map* table, int ind) {

	int index = hash_function(key);
	if (table->keys[index] == "") {

		table->keys[index] = key;
	}
	else if (table->keys[index] == key)
	{
		string mess = "Элемент уже есть";
		int msg_size = mess.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], mess.c_str(), msg_size, NULL);
		
	}
	else if (table->keys[index] != key && table->keys[index] != "")
	{

		for (int i = index + 1; i <= 512; i++) {
			if (table->keys[i] == "") {
				table->keys[i] = key;

				break;
			}
		}
	}
	cout << "Oбработан запрос SADD." << " " << endl;
}

void remove_map(string key, map* table, int ind) {
	int index = hash_function(key);
	if (table->keys[index] == key) {
		table->keys[index] = "";
		string msg = "deleted";
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	else if ((table->keys[index] != key) && (table->keys[index] != "")) {
		for (int i = index + 1; i <= 512; i++) {
			if (table->keys[i] == key) {
				table->keys[i] = "";
				string msg = "deleted";
				int msg_size = msg.size();
				send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
				send(Connections[ind], msg.c_str(), msg_size, NULL);
				break;
			}
		}
	}
	cout << "Oбработан запрос SREM." << " " << endl;
}


void find_map(map* table, string key, int ind) {
	int index = hash_function(key);
	if (table->keys[index] == key) {
		string msg = "элемент есть";
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	else if ((table->keys[index] != key)) {
		for (int i = index + 1; i <= 512; i++) {
			if (table->keys[i] == key) {
				string msg = "элемент есть";
				int msg_size = msg.size();
				send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
				send(Connections[ind], msg.c_str(), msg_size, NULL);
				break;
			}
			if (i == 512) {
				//cout << "элемент не существует" << endl;
				string msg = "no such element";
				int msg_size = msg.size();
				send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
				send(Connections[ind], msg.c_str(), msg_size, NULL);
			}
		}
	}
	else if (table->keys[index] == "") {
		string msg = "no such element";
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	cout << "Oбработан запрос SISMEMBER." << " " << endl;
}


//-------------------------------------------------------hash-table
struct Node {
	string key;
	string value;
};

struct hasht {
	string keys[512];
	string values[512];

};


void insert(string key, string value, hasht* table, int ind) {

	int index = hash_function(key);
	if (table->keys[index] == "") {
		table->values[index] = value;
		table->keys[index] = key;
	}
	else if ((table->keys[index] != "") && (table->keys[index] != key)) {
		for (int i = index + 1; i <= 512; i++) {
			if (table->keys[i] == "") {
				table->keys[i] = key;
				table->values[i] = value;
				break;
			}
		}
	}
	else if (table->keys[index] == key)
	{
		//cout << "ключ  " << key << " уже существует" << endl;
		//table->values[index] = value;
		string msg = "ключ уже существует";
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	cout << "Oбработан запрос HSET." << " " << endl;
}

void remove(string key, hasht* table, int ind) {
	int index = hash_function(key);
	if (table->keys[index] == key) {
		table->keys[index] = "";
		table->values[index] = "";
		string msg = "deleted";
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	else if ((table->keys[index] != key) && (table->keys[index] != "")) {
		for (int i = index + 1; i <= 512; i++) {
			if (table->keys[i] == key) {
				table->keys[i] = "";
				table->values[i] = "";
				string msg = "deleted";
				int msg_size = msg.size();
				send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
				send(Connections[ind], msg.c_str(), msg_size, NULL);
				break;
			}
		}
	}
	cout << "Oбработан запрос HDEL." << " " << endl;
}

void find(string key, hasht* table, int ind) {
	int index = hash_function(key);
	if (table->keys[index] == key) {
		string msg = table->values[index];
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	else if ((table->keys[index] != key)) {
		for (int i = index + 1; i <= 512; i++) {
			if (table->keys[i] == key) {
				string msg = table->values[i];
				int msg_size = msg.size();
				send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
				send(Connections[ind], msg.c_str(), msg_size, NULL);
				break;
			}
			if (i == 512) {
				string msg = "ключ не существует";
				int msg_size = msg.size();
				send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
				send(Connections[ind], msg.c_str(), msg_size, NULL);
			}
		}
	}
	else if (table->keys[index] == "") {
		string msg = "ключ не существует";
		int msg_size = msg.size();
		send(Connections[ind], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[ind], msg.c_str(), msg_size, NULL);
	}
	cout << "Oбработан запрос HGET." << " " << endl;
}

//-------------------------------------------------------------------server
Stack* st = new Stack;
Queue* q = new Queue;
map* set = new map;
hasht* ht = new hasht;

string command[10];

void Parsing(char* cmd) { // парсинг введенной клентом строки
	int j = 0, size = 0;
	for (int i = 0; i < 10; i++) {
		string part = "";
		for (j; cmd[j] != '\0'; j++) {
			if (cmd[j] == ' ') {
				j++;
				break;
			}
			else {
				part += cmd[j];
			}
		}
		command[i] = part;
	}
}

void Errors(int index) {	// функция для вывода частой ошибки
	string error = "Недостаточно агрументов";
	int err_size = error.size();
	send(Connections[index], (char*)&err_size, sizeof(int), NULL);
	send(Connections[index], error.c_str(), err_size, NULL);
}


void ClientHandler(int index) { // функция для общения с клиентом
	while (index >= 0) {
		int msg_size = 1024;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		Parsing(msg);
		if (index > 0) {	// для многопоточности
			mute.lock(); // мьютекс лок
			if (command[0] == "SPUSH") {
				if (command[1] == "") {
					Errors(index);
				}
				push(*st, command[1]);
			}
			else if (command[0] == "SPOP") {
				string msg = pop(*st, index);
			}
			else if (command[0] == "QPUSH") {
				if (command[1] == "") {
					cout << "Недостаточно агрументов" << endl;
				}
				pushToQueue(*q, command[1]);
			}
			else if (command[0] == "QPOP") {
				popFromQueue(*q, index);
			}
			else if (command[0] == "SADD") {
				if (command[1] == "") {
					Errors(index);
				}
				add_map(command[1], set, index);
			}
			else if (command[0] == "SREM") {
				remove_map(command[1], set, index);
			}
			else if (command[0] == "SISMEMBER") {
				if (command[1] == "") {
					Errors(index);
				}
				find_map(set, command[1], index);
			}
			else if (command[0] == "HSET") {
				if (command[2] == "") {
					Errors(index);
				}
				else {
					insert(command[1], command[2], ht, index);
				}
			}
			else if (command[0] == "HDEL") {
				if (command[1] == "") {
					Errors(index);
				}
				remove(command[1], ht, index);
			}
			else if (command[0] == "HGET") {
				if (command[1] == "") {
					Errors(index);
				}
				find(command[1], ht, index);
			}
			else {
				string error = "неверная команда";
				int err_size = error.size();
				send(Connections[index], (char*)&err_size, sizeof(int), NULL);
				send(Connections[index], error.c_str(), err_size, NULL);
			}
			delete[] msg;
			mute.unlock(); // разблокирование бд для остальных
		}

		if (index == 0) { // для одного клиента
			if (command[0] == "SPUSH") {
				if (command[1] == "") {
					Errors(index);
				}
				push(*st, command[1]);
			}
			else if (command[0] == "SPOP") {
				string msg = pop(*st, index);
			}
			else if (command[0] == "QPUSH") {
				if (command[1] == "") {
					cout << "Недостаточно агрументов" << endl;
				}
				pushToQueue(*q, command[1]);
			}
			else if (command[0] == "QPOP") {
				popFromQueue(*q, index);
			}
			else if (command[0] == "SADD") {
				if (command[1] == "") {
					Errors(index);
				}
				add_map(command[1], set, index);
			}
			else if (command[0] == "SREM") {
				remove_map(command[1], set, index);
			}
			else if (command[0] == "SISMEMBER") {
				if (command[1] == "") {
					Errors(index);
				}
				find_map(set, command[1], index);
			}
			else if (command[0] == "HSET") {
				if (command[2] == "") {
					Errors(index);
				}
				else {
					insert(command[1], command[2], ht, index);
				}
			}
			else if (command[0] == "HDEL") {
				if (command[1] == "") {
					Errors(index);
				}
				remove(command[1], ht, index);
			}
			else if (command[0] == "HGET") {
				if (command[1] == "") {
					Errors(index);
				}
				find(command[1], ht, index);
			}
			else {
				string error = "неверная команда";
				int err_size = error.size();
				send(Connections[index], (char*)&err_size, sizeof(int), NULL);
				send(Connections[index], error.c_str(), err_size, NULL);
			}
			delete[] msg;
		}
	}
}


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Rus");
	initstack(*st);
	createQueue(*q);

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
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // преобразовываем строку в значение порядка байтов сети
	addr.sin_port = htons(6379); // преобразовать номер порта из порядка байтов хоста в целочисленное значение порядка байтов сети
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); // сокет прослушки инициализируем с параметрами tcp протокола
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); // биндим адрес на прослушку
	listen(sListen, SOMAXCONN); // запускаем прослушку с возможностью нескольких потоков

	SOCKET newConnection; // новый сокет на новое подключение
	for (int i = 0; i < 100; i++) { // цикл для новых подключений
		cout << endl << "Waiting for activity... ";
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); // ждем подключения

		if (newConnection == 0) {
			cout << "Error \n";
		}
		else {
			cout << "Client Connected!\n";
			string msg = "Hello. Type a command and value.\n For stack: SPOP/SPUSH\n For queue: QPUSH/QPOP\n For set: SADD/SREM/SISMEMBER\n For hash table: HSET/HDEL/HGET\n";
			int msg_size = msg.size();
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			Connections[i] = newConnection; // присваеваем элементу массива сокетов текущее подключение
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL); // создаем поток для текущего соединения
		}
	}
	system("pause");
	return 0;
}