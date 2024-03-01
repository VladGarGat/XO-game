#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#pragma warning(disable: 4996)

int countdown = 1000;
SOCKADDR_IN address;
SOCKET sListen;

const int MAX_CONNECTONS = 10;
int cur_connections = 0;
SOCKET Connections[10];

int cur_profiles = 0;
const int MAX_PROFILES = 10;
std::string logins[MAX_PROFILES];
std::string passwords[MAX_PROFILES];

char pst[10] = { '0','1','2','3','4','5','6','7','8','9' };

std::ofstream logfile;

void logger(std::string str) {
	time_t now = time(nullptr);
	std::string ltime = ctime(&now);
	logfile << ">>" << ltime << " " << str << std::endl;
}

void readCfg()
{
	std::ifstream cfg("../cfg.txt");
	if (!cfg) { logger("config file can't be opened!");}

	std::string line;
	std::getline(cfg, line);
	countdown = std::stoi(line);
	std::getline(cfg, line);
	cur_profiles = std::stoi(line);

	for (int i = 0; i < cur_profiles || cfg.peek() != EOF; i++)
	{
		std::getline(cfg, line);
		std::istringstream ss(line);
		std::string login, password;
		if (!(ss >> login >> password)) {
			logger("can't read profiles!");
		}

		logins[i] = login;
		passwords[i] = password;
	}
	logger("data from config's been read");
}

bool setListener()
{

	WSADATA wsadata;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsadata) != 0) {
		logger("unable to execute WSAStartup");
	};

	address; // SOCKADDR_IN
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(1111);
	address.sin_family = AF_INET;

	sListen = socket(AF_INET, SOCK_STREAM, NULL); // SOCKET
	bind(sListen, (SOCKADDR*)&address, sizeof(address));
	listen(sListen, MAX_CONNECTONS);

	logger("Listener's been successfully set");
	return true;
}

bool isRegistered(SOCKET connection)
{
	// здесь мы должны получить логин и пароль от клиента и проверить их

	char ch_login[32];
	char ch_password[32];
	recv(connection, ch_login, sizeof(ch_login), NULL);
	recv(connection, ch_password, sizeof(ch_password), NULL);

	std::string login(ch_login);
	std::string password(ch_password);

	for (int i = 0; i < cur_profiles; i++)
	{
		if ((logins[i] == login) && (passwords[i] == password)) {
			send(connection, "%succeed%",10,0);
			logger("user's logged in");
			return true;
		}
	}
	send(connection, "%failed_%", 10, 0);
	logger("user isn't whitelisted");
	return false;
}

void acceptConnections(int conns_needed)
{
	// accept ждет, пока в sListen не придет подключение
	SOCKET newConnection;
	int sizeofaddress = sizeof(address);
	while ((cur_connections < MAX_CONNECTONS) && (conns_needed > 0))
	{
		newConnection = accept(sListen, (SOCKADDR*)&address, &sizeofaddress);
		logger("connection's been accepted");

		if (newConnection == 0) {
			std::cout << "Can't connect the client\n";
			logger("unable to connect the client");
		}
		else {
			logger("client's been connected");
			std::cout << "Client's connected [" << cur_connections << "]\n";
			// здесь же проведем аутентификацию, чтобы отключить гада сразу же
			if (isRegistered(newConnection)) {
				logger("client's been authorized");
				Connections[cur_connections++] = newConnection;
				conns_needed--;
			}
			else
			{
				closesocket(newConnection);
				logger("connection aborted");
			}
		}
	}
}

void findPair(int& i1, int& i2)
{
	bool f_found = false, s_found = false;

	for (int i = 0; (!f_found || !s_found); i++)
	{
		int r = send(Connections[i], NULL, 0, 0);
		if (r == SOCKET_ERROR) {
			std::cout << i << ": connection doesn't exist\n";
		}
		else {
			std::cout << i << ": connection exists\n";
			if (!f_found) { f_found = true; i1 = i; }
			else { s_found = true; i2 = i; }
		}
	}
	logger("the pair of players is found");
}

bool areEnoughPlayers(int& cons_needed)
{
	// игроков хватает, если сущ-вует минимум 2 сокета из Connections[]
	cons_needed = 2;
	logger("searching for 2 sockets started");
	for (int i = 0; (i < MAX_CONNECTONS) && (cons_needed > 0); i++)
	{
		char go[3];
		int r = recv(Connections[i], go, 3, 0);
		if (r == SOCKET_ERROR) {
			std::cout << i << ": connection doesn't exist\n";
		}
		else {
			std::cout << i << ": connection exists\n";
			logger("connection's found");
			cons_needed--;
		}
	}
	if (cons_needed != 0) { return false; }
	return true;
}

void printBoard()
{
	std::cout << " " << pst[1] << " | " << pst[2] << " | " << pst[3] << "\n";
	std::cout << "___________\n";
	std::cout << " " << pst[4] << " | " << pst[5] << " | " << pst[6] << "\n";
	std::cout << "___________\n";
	std::cout << " " << pst[7] << " | " << pst[8] << " | " << pst[9] << "\n";
}

std::string makeStringBoard()
{

	std::string s1(1, pst[1]), s2(1, pst[2]), s3(1, pst[3]);
	std::string s4(1, pst[4]), s5(1, pst[5]), s6(1, pst[6]);
	std::string s7(1, pst[7]), s8(1, pst[8]), s9(1, pst[9]);
	std::string str =
		" " + s1 + " | " + s2 + " | " + s3 + "\n"
		+ "___________\n" +
		+" " + s4 + " | " + s5 + " | " + s6 + "\n"
		+ "___________\n" +
		+" " + s7 + " | " + s8 + " | " + s9 + "\n";
	return str;
}

bool hasWinner(char& win_symb)
{
	if ((pst[1] == 'X' && pst[2] == 'X' && pst[3] == 'X') ||
		(pst[4] == 'X' && pst[5] == 'X' && pst[6] == 'X') ||
		(pst[7] == 'X' && pst[8] == 'X' && pst[9] == 'X') ||

		(pst[1] == 'X' && pst[4] == 'X' && pst[7] == 'X') ||
		(pst[2] == 'X' && pst[5] == 'X' && pst[8] == 'X') ||
		(pst[3] == 'X' && pst[6] == 'X' && pst[9] == 'X') ||

		(pst[1] == 'X' && pst[5] == 'X' && pst[9] == 'X') ||
		(pst[3] == 'X' && pst[5] == 'X' && pst[7] == 'X')  )
	{
		logger("X won"); win_symb = 'X'; return true;
	}

	if ((pst[1] == 'O' && pst[2] == 'O' && pst[3] == 'O') ||
		(pst[4] == 'O' && pst[5] == 'O' && pst[6] == 'O') ||
		(pst[7] == 'O' && pst[8] == 'O' && pst[9] == 'O') ||

		(pst[1] == 'O' && pst[4] == 'O' && pst[7] == 'O') ||
		(pst[2] == 'O' && pst[5] == 'O' && pst[8] == 'O') ||
		(pst[3] == 'O' && pst[6] == 'O' && pst[9] == 'O') ||

		(pst[1] == 'O' && pst[5] == 'O' && pst[9] == 'O') ||
		(pst[3] == 'O' && pst[5] == 'O' && pst[7] == 'O'))
	{
		logger("O won"); win_symb = 'O'; return true;
	}

	return false;
}

bool draw(int& made_moves)
{
	if (made_moves == 9) {
		logger("the game finished with a draw");
		return true;
	}
	return false;
}

void make_move(SOCKET player, char symb)
{
	// Игрок не может сделать ход на клетку, в котрой уже сделан ход
	send(player, "%can_move%", 11, 0);

	std::string str = makeStringBoard();
	const char* board = str.c_str();
	send(player, board, 57, NULL);

	char chnum[2]; recv(player, chnum, 2, 0);
	int num = chnum[0] - 48;
	if (pst[num] != 'X' && pst[num] != 'O') { pst[num] = symb; }

	std::string mes = "player went " + std::to_string(num);
	logger(mes);
}

void roll_the_dice(SOCKET& first_player, SOCKET& second_player, int i1, int i2)
{
	srand(unsigned int(time(NULL)));
	int value = rand() % 2;
	if (value == 0)
	{
		first_player = Connections[i1];
		second_player = Connections[i2];
	}
	else
	{
		first_player = Connections[i2];
		second_player = Connections[i1];
	}
	send(first_player, "You play for X-team", 20, 0);
	send(second_player, "You play for O-team", 20, 0);

	logger("splitting up into teams is finished");
}

void startGame(int i1, int i2)
{
	for (int i = 0; i < 10; i++) { pst[i] = (i+48); }

	char win_symb = 'S';
	int made_moves = 0;
	SOCKET first_player = 0, second_player = 0;

	roll_the_dice(first_player, second_player, i1, i2);

	while (!hasWinner(win_symb) && !draw(made_moves))
	{
		// making moves
		if ((made_moves % 2) == 0) {
			logger("first player can move");
			make_move(first_player, 'X');
		}
		else {
			logger("second player can move");
			make_move(second_player, 'O');
		}

		made_moves++;
	}
	// Здесь в клиентском коде надо выгнать обоих игроков из while(true)
	send(first_player, "%finising%", 11, 0);
	send(second_player, "%finising%", 11, 0);
	if (hasWinner(win_symb))
	{
		// если победил Х, то это всегда первый игрок, О - второй
		if (win_symb == 'X')
		{
			send(first_player, "You triumphed!!!", 17, 0);
			send(second_player, "You are defeated", 17, 0);
		}
		else
		{
			send(second_player, "You triumphed!!!", 17, 0);
			send(first_player, "You are defeated...", 17, 0);
		}
	}
};


int main()
{
	logfile.open("log.txt");\
	logger("logger's opened");

	int i1 = 0, i2 = 0, cons_needed = 2;

	readCfg();
	if (!setListener())
	{
		std::cout << "error during connection phase\n";
		logger("some error during connection phase");
		// exit(1);
	}

	while (true)
	{
		if (!areEnoughPlayers(cons_needed)) { acceptConnections(cons_needed); }
		findPair(i1, i2); // здесь i1, i2 меняются
		startGame(i1, i2);
	}

	logger("end session");
	logfile.close();
}