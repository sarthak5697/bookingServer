#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <windows.h>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

// Function to read database credentials from the configuration file
bool readConfigFile(const string& fileName, string& host, string& user, string& password, string& dbName) {
	ifstream configFile(fileName);
	if (!configFile.is_open()) {
		cerr << "Error: Unable to open the configuration file." << endl;
		return false;
	}
	configFile >> host >> user >> password >> dbName;
	configFile.close();
	return true;
}

class Seats {
private:
	int Seat[5][10];
public:
	Seats() {
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 10; j++) {
				Seat[i][j] = 1;
			}
		}
	}

	int getSeatStatus(int row, int seatNumber) {
		if (row < 1 || row >5 || seatNumber < 1 || seatNumber >10) {
			return -1;
		}
		return Seat[row - 1][seatNumber - 1];
	}

	void reserveSeat(int row, int seatNumber) {
		if (row < 1 || row >5 || seatNumber < 1 || seatNumber >10) {
			return;
		}
		Seat[row - 1][seatNumber - 1] = 0;
	}

	void display() {
		cout << "			";
		for (int i = 0; i < 10; i++) {
			cout << " " << i + 1;
		}
		cout << endl;

		for (int row = 0; row < 5; row++) {
			cout <<"			"<< row + 1 << " ";
			for (int col = 0; col < 10; col++) {
				if (Seat[row][col] == 1) {
					cout << "- ";
				}
				else {
					cout << "X ";
				}
			}
			cout << "|" << endl;
		}
		cout << "			"<<"-----------------------" << endl;
	}

	int getDB(MYSQL* conn) {
		string query = "SELECT RowNumber, SeatNumber, Seat FROM Ticket";
		if (mysql_query(conn, query.c_str())) {
			cout << "Error: " << mysql_error(conn) << endl;
		}

		MYSQL_RES* result;
		result = mysql_store_result(conn);
		if (!result) {
			cout << "Error: " << mysql_error(conn) << endl;
		}
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(result))) {
			int rowNumber = atoi(row[0]);
			int seatNumber = atoi(row[1]);
			int seatStatus = atoi(row[2]);
			Seat[rowNumber - 1][seatNumber - 1] = seatStatus;
		}
		mysql_free_result(result);
	}
};

int main() {
	string host, user, password, dbName;
	if (!readConfigFile("config.ini", host, user, password, dbName)) {
		return 1; // Exit if unable to read config file
	}

	Seats s;
	MYSQL* conn;
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), dbName.c_str(), 3306, NULL, 0)) {
		cout << "Error: " << mysql_error(conn) << endl;
		return 1; // Exit if unable to connect to the database
	}
	else {
		cout << "Logged In Database!" << endl;
	}
	Sleep(1000);

	if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS Ticket (RowNumber INT, SeatNumber INT, Seat INT)")) {
		cout << "Error: " << mysql_error(conn) << endl;
	}

	for (int row = 1; row <= 5; row++) {
		for (int seatNumber = 1; seatNumber <= 10; seatNumber++) {
			stringstream ss;
			ss << "INSERT INTO Ticket (RowNumber,SeatNumber,Seat)"
				<< "SELECT '" << row << "', '" << seatNumber << "','1' "
				<< "WHERE NOT EXISTS (SELECT * FROM Ticket WHERE RowNumber = '" << row << "' AND SeatNumber = '" << seatNumber << "')";
			string insertQuery = ss.str();
			if (mysql_query(conn, insertQuery.c_str())) {
				cout << "Error: " << mysql_error(conn);
			}
		}
	}
	Sleep(1000);

	bool exit = false;
	while (!exit) {
		system("cls");
		cout << endl;
		cout << "		Welcome To Movie Ticket Booking System" << endl;
		cout << "		******************************************" << endl;
		cout << "		1. Reserve a Ticket" << endl;
		cout << "		2. Exit" << endl;
		cout << "		3. Print Ticket" << endl;
		cout << "		Enter your choice: ";

		int val;
		cin >> val;
		cout << endl;

		if (val == 1) {
			s.getDB(conn);
			s.display();

			int row, col;
			cout << "		Enter Row (1-5): ";
			cin >> row;
			cout << "		Enter Seat Number (1-10): ";
			cin >> col;

			if (row < 1 || row > 5 || col < 1 || col > 10) {
				cout << "		Invalid Row or Seat Number!" << endl;
				Sleep(1000);
				continue;
			}
			int seatStatus = s.getSeatStatus(row, col);
			if (seatStatus == -1) {
				cout << "		Invalid Row or Seat Number!" << endl;
				Sleep(1000);
				continue;
			}

			if (seatStatus == 0) {
				cout << "		Sorry: Seat is already reserved!" << endl;
				Sleep(1000);
				continue;
			}

			s.reserveSeat(row, col);
			stringstream ss;
			ss << "		UPDATE Ticket SET Seat = 0 WHERE RowNumber = " << row << "		AND SeatNumber =" << col;
			string update = ss.str();
			if (mysql_query(conn, update.c_str())) {
				cout << "		Error: " << mysql_error(conn) << endl;
			}
			else {
				cout << "			Seat Is Reserved Successfully in Row " << row << " and Seat Number " << col << endl;
			}
			Sleep(1000);
		}//if1

		else if (val == 2) {
			exit = true;
			cout << "		Good Luck!" << endl;
			Sleep(1000);
		}
		else if (val == 3) {
			// Add code to print the ticket
			cout << "		Printing Ticket..." << endl;

			// Retrieve ticket information from the database
			string query = "		SELECT RowNumber, SeatNumber FROM Ticket WHERE Seat = 0";
			if (mysql_query(conn, query.c_str())) {
				cout << "		Error: " << mysql_error(conn) << endl;
				Sleep(1000);
				continue;
			}
			MYSQL_RES* result;
			result = mysql_store_result(conn);
			if (!result) {
				cout << "		Error: " << mysql_error(conn) << endl;
				Sleep(1000);
				continue;
			}

			// Print ticket information
			cout << "		-------------------" << endl;
			cout << "		Ticket Information:" << endl;
			MYSQL_ROW row;
			cout << "		-------------------------------------" << endl;
			while ((row = mysql_fetch_row(result))) {
				cout << "		-------------------------------------" << endl;

				cout << "		|		Row: " << row[0] << ", Seat: " << row[1] << "   |" << endl;
				cout << "		-------------------------------------" << endl;

			}
			cout << "		-------------------------------------" << endl;
			mysql_free_result(result);

			cout << "		-------------------" << endl;
			cout << "		Ticket Printed Successfully!" << endl;
			Sleep(3000);
		}

		else {
			cout << "		Invalid Input" << endl;
			Sleep(1000);
		}
	}
	mysql_close(conn);
	return 0;
}

