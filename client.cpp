#include <iostream>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)	//w��czenie ob�ugi funkcji fopen

//funkcja por�wnuj�ca ilo�� ramek przes�anych do serwera
//z zadan� liczb� ramek
//funkcja zosta�a zapo�yczona z github.com/Kuszki/PWSS-TPK-Example
bool send_all(int sock, const char* data, size_t size)
{
	// Gdy s� jeszcze dane do wys�ania
	while (size > 0)
	{
		// Wy�lij brakuj�ce dane
		const int sc = send(sock, data, size, 0);

		// W przypadku b��du przerwij dzia�anie
		if (sc <= 0) return false;
		else
		{
			data += sc; // Przesu� wska�nik na dane
			size -= sc; // Zmniejsz liczb� pozosta�ych danych
		}
	}

	return true;
}



int main() {
	setlocale(LC_CTYPE, "Polish");
	WSADATA wsaData;			//inicjalizacja struktury WSADATA

	WORD version = MAKEWORD(2, 2); //wybranie wersji biblioteki WSADATA

	// Start WinSock
	int Result = WSAStartup(version, &wsaData); //Wywo�anie funkcji inicjuj�cej winsock
	if (Result != 0)	//kontrola poprawno�ci inicjalizacji
	{
		std::cout << "Nie uda�o si� rozpocz�� Winsock! " << Result;
		return 1;
	}


	int socketClient = socket(AF_INET, SOCK_STREAM, 0); //utworzenie gniazda sieciowego
	sockaddr_in socketServer;				//inicjalizacja struktury z adresem serwera
	socketServer.sin_family = AF_INET;		//wybranie rodziny adres�w dla servera (IPV4)
	socketServer.sin_port = htons(4200);	//wybranie portu na kt�rym dzia�a serwer
	memset(socketServer.sin_zero, '\0', sizeof socketServer.sin_zero); //wype�nienie sin_zero zerami
	inet_pton(AF_INET, "127.0.0.1", &socketServer.sin_addr);	//konwersja adresu i przekazanie do struktury
	if (connect(socketClient, (struct sockaddr*)&socketServer, sizeof(socketServer)) != 0) {	//nawi�zanie po��czenia z serwerem
		std::cout << "nie dzia�a " << std::endl;	//sprawdzenie poprawno�ci po��czenia
		closesocket(socketClient);
		WSACleanup();
		return 1;
	}

	int des;
	char buf[1024];

	std::string polecenie;		//wybranie akcji jak� ma wykona� klient
	std::cout << "Co chcesz zrobi�: [DOWNLOAD/UPLOAD]";
	std::cin >> polecenie;
	std::string nazwa;
	std::cout << "Podaj nazw� pliku: ";	//wybranie nazwy pliku
	std::cin >> nazwa;

	if (polecenie == "DOWNLOAD") {		//je�li DOWNLOAD
		
		FILE* hPlik = fopen(nazwa.c_str(), "wb");	//otwarcie pliku do przes�ania w trybie binarnym
		if (hPlik == NULL) {	//sprawdzenie czy plik istnieje
			std::cout << "Podano z�y plik ";
			closesocket(socketClient);		//zamkni�cie gniazda
			WSACleanup();					//zwolnienie WsaData
			return 1;
		}
		nazwa = polecenie + " " + nazwa + '\n';		//skompletowanie nag��wka sk�adaj�cego si� z akcji, nazwy pliku i znaku nowej linii
		int Send = send_all(socketClient, nazwa.c_str(), nazwa.size());		//wys�anie nag��wka do serwera

		int bytesIn;
		while ((bytesIn = recv(socketClient, buf, sizeof(buf), 0)) > 0) {//odbieranie ramek z serwera
			if (bytesIn == SOCKET_ERROR)				 //sprawdzenie poprawno�ci otrzymania danych					
			{
				std::cout << "Error receiving from client " << WSAGetLastError() << std::endl;
				break;
			}

			if (hPlik == NULL) {		//sprawdzenie czy plik isnieje
				std::cout << "Podano z�y plik ";
				closesocket(socketClient);					//zwolnienie socketu
				WSACleanup();								//zwolnienie biblioteki
				return 1;
			}

			fwrite(buf, 1, bytesIn, hPlik);		//zapisanie danych do pliku
			fflush(hPlik);						//wyczyszczenie bufora zapisu do pliku

			if (bytesIn == 0) {
				std::cout << "otrzymano plik" << std::endl;
			}
		}
		fclose(hPlik);
	}
	else if (polecenie == "UPLOAD") {	//je�li UPLOAD
		clock_t start, end;
		FILE* hPlik = fopen(nazwa.c_str(), "rb");	//otwarcie pliku do przes�ania w trybie binarnym

		if (hPlik == NULL) {	//sprawdzenie czy plik istnieje
			std::cout << "Podano z�y plik ";
			closesocket(socketClient);
			WSACleanup();
			return 1;
		}
		int count;
		nazwa = polecenie + " " + nazwa + '\n';

		int Send = send_all(socketClient, nazwa.c_str(), nazwa.size());

		start = clock();	//pomiar czasu przesy�ania pliku;	
		while ((count = fread(&buf, 1, sizeof(buf), hPlik)) > 0)	//odczyt pliku w p�tli
		{
			int Send = send_all(socketClient, buf, count);			//wys�anie ramki o zadanej wielko�
			if (Send == SOCKET_ERROR)								//sprawdzenie czy ramka si� wys�a�
			{
				std::cout << "That didn't work! " << WSAGetLastError() << std::endl;
				fclose(hPlik);
				closesocket(socketClient);
				WSACleanup();
				return 1;
			}


		}
		end = clock();	//zako�czenie pomiaru
		double czas = (end - start) / (double)CLOCKS_PER_SEC;
		std::cout << "czas przesylu pliku: " << czas << " s" << std::endl;

		fclose(hPlik);													//zamkni�cie pliku

	}

	closesocket(socketClient);										//zwolnienie socketu

	WSACleanup();													//zwolnienie biblioteki

	return 0;
}