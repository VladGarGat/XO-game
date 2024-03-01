#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string.h>
#include <fstream>
#pragma warning(disable: 4996)

SOCKET Connection;
std::ofstream logfile;

void logger(std::string str) {
	time_t now = time(nullptr);
	std::string ltime = ctime(&now);
	logfile << ">>" << ltime << " " << str << std::endl;
}

void authorize()
{
	char login[32];
	char password[32];

	std::cout << "Enter your login: ";
	std::cin >> login;
	std::cout << "\nEnter your password: ";
	std::cin >> password;
	std::cout << std::endl;

	send(Connection, login, sizeof(login), NULL);
	send(Connection, password, sizeof(login), NULL);

}

bool setConnection()
{
	WSADATA wsadata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsadata) != 0) {
		logger("unbalbe to execute WSAStartup");
		exit(1);
	};

	SOCKADDR_IN address;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(1111);
	address.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&address, sizeof(address)) != 0) {
		std::cout << "can't connect to the server";
		logger("can't connect to the server");
		return false; }

	std::cout << "Client's connected\n";
	logger("Client's connected");
	return true;
}

int main()
{
	logfile.open("clog.txt");
	logger("log file's been opened");

	if (!setConnection()) {
		std::cout << "error during connection phase \n";
		logger("error during connection phase");
		// exit(0);
		}
	authorize();

	char reply[10]; recv(Connection, reply, 10, 0); std::string s_reply(reply);

	if (s_reply != "%succeed%") {
		std::cout << "Wrong login/password\n";
		logger("wrong login/password");
		// exit(0);
		
	}
	else (std::cout << "Successful authorization!\n");
	logger("Successful authorization!");

	// Здесь можно играть

	while (true)
	{
		char team[20]; recv(Connection, team, 20, NULL); std::cout << team << std::endl;
		while (true)
		{
			char perm[11]; recv(Connection, perm, 11, 0); std::string s_perm(perm);
			if (s_perm == "%can_move%")
			{
				logger("player can move");
				char board[57];
				recv(Connection, board, 57, 0);
				std::string brd(board); brd.resize(57);
				std::cout << brd;

				std::cout << "Choose the field [1-9]\n >>>  ";
				char num[2]; std::cin >> num; std::cout << std::endl;
				send(Connection, num, 2, 0);

				std::string mes = "player went "; mes.push_back(num[0]);
				logger(mes);
			}
			else if (s_perm == "%finising%") { logger("game's over"); break; }
		}

		char result[17];
		recv(Connection, result, 17, NULL);
		std::cout << result << std::endl;

		std::cout << "Do you want to play again? (y/n):\n >>> ";
		std::string ans; std::cin >> ans;
		if (ans == "y")
		{
			logger("player agreed to play again");
			send(Connection, "go", 3, 0); // for areEnoughPlayers();
			//send(Connection, NULL, 0, 0); // for findpair();
			continue;
		}
		else {
			logger("player refused to play again");
			break;
		}
	}
	logger("end session");
	logfile.close();
	system("Pause");
}