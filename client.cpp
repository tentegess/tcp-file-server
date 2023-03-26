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
#pragma warning(disable:4996)	//w³¹czenie ob³ugi funkcji fopen

//funkcja porównuj¹ca iloœæ ramek przes³anych do serwera
//z zadan¹ liczb¹ ramek
//funkcja zosta³a zapo¿yczona z github.com/Kuszki/PWSS-TPK-Example
bool send_all(int sock, const char* data, size_t size)
{
	// Gdy s¹ jeszcze dane do wys³ania
	while (size > 0)
	{
		// Wyœlij brakuj¹ce dane
		const int sc = send(sock, data, size, 0);

		// W przypadku b³êdu przerwij dzia³anie
		if (sc <= 0) return false;
		else
		{
			data += sc; // Przesuñ wskaŸnik na dane
			size -= sc; // Zmniejsz liczbê pozosta³ych danych
		}
	}

	return true;
}



int main() {
	setlocale(LC_CTYPE, "Polish");
	WSADATA wsaData;			//inicjalizacja struktury WSADATA

	WORD version = MAKEWORD(2, 2); //wybranie wersji biblioteki WSADATA

	// Start WinSock
	int Result = WSAStartup(version, &wsaData); //Wywo³anie funkcji inicjuj¹cej winsock
	if (Result != 0)	//kontrola poprawnoœci inicjalizacji
	{
		std::cout << "Nie uda³o siê rozpocz¹æ Winsock! " << Result;
		return 1;
	}


	int socketClient = socket(AF_INET, SOCK_STREAM, 0); //utworzenie gniazda sieciowego
	sockaddr_in socketServer;				//inicjalizacja struktury z adresem serwera
	socketServer.sin_family = AF_INET;		//wybranie rodziny adresów dla servera (IPV4)
	socketServer.sin_port = htons(4200);	//wybranie portu na którym dzia³a serwer
	memset(socketServer.sin_zero, '\0', sizeof socketServer.sin_zero); //wype³nienie sin_zero zerami
	inet_pton(AF_INET, "127.0.0.1", &socketServer.sin_addr);	//konwersja adresu i przekazanie do struktury
	if (connect(socketClient, (struct sockaddr*)&socketServer, sizeof(socketServer)) != 0) {	//nawi¹zanie po³¹czenia z serwerem
		std::cout << "nie dzia³a " << std::endl;	//sprawdzenie poprawnoœci po³¹czenia
		closesocket(socketClient);
		WSACleanup();
		return 1;
	}

	int des;
	char buf[1024];

	std::string polecenie;		//wybranie akcji jak¹ ma wykonaæ klient
	std::cout << "Co chcesz zrobiæ: [DOWNLOAD/UPLOAD]";
	std::cin >> polecenie;
	std::string nazwa;
	std::cout << "Podaj nazwê pliku: ";	//wybranie nazwy pliku
	std::cin >> nazwa;

	if (polecenie == "DOWNLOAD") {		//jeœli DOWNLOAD
		
		FILE* hPlik = fopen(nazwa.c_str(), "wb");	//otwarcie pliku do przes³ania w trybie binarnym
		if (hPlik == NULL) {	//sprawdzenie czy plik istnieje
			std::cout << "Podano z³y plik ";
			closesocket(socketClient);		//zamkniêcie gniazda
			WSACleanup();					//zwolnienie WsaData
			return 1;
		}
		nazwa = polecenie + " " + nazwa + '\n';		//skompletowanie nag³ówka sk³adaj¹cego siê z akcji, nazwy pliku i znaku nowej linii
		int Send = send_all(socketClient, nazwa.c_str(), nazwa.size());		//wys³anie nag³ówka do serwera

		int bytesIn;
		while ((bytesIn = recv(socketClient, buf, sizeof(buf), 0)) > 0) {//odbieranie ramek z serwera
			if (bytesIn == SOCKET_ERROR)				 //sprawdzenie poprawnoœci otrzymania danych					
			{
				std::cout << "Error receiving from client " << WSAGetLastError() << std::endl;
				break;
			}

			if (hPlik == NULL) {		//sprawdzenie czy plik isnieje
				std::cout << "Podano z³y plik ";
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
	else if (polecenie == "UPLOAD") {	//jeœli UPLOAD
		clock_t start, end;
		FILE* hPlik = fopen(nazwa.c_str(), "rb");	//otwarcie pliku do przes³ania w trybie binarnym

		if (hPlik == NULL) {	//sprawdzenie czy plik istnieje
			std::cout << "Podano z³y plik ";
			closesocket(socketClient);
			WSACleanup();
			return 1;
		}
		int count;
		nazwa = polecenie + " " + nazwa + '\n';

		int Send = send_all(socketClient, nazwa.c_str(), nazwa.size());

		start = clock();	//pomiar czasu przesy³ania pliku;	
		while ((count = fread(&buf, 1, sizeof(buf), hPlik)) > 0)	//odczyt pliku w pêtli
		{
			int Send = send_all(socketClient, buf, count);			//wys³anie ramki o zadanej wielkoœ
			if (Send == SOCKET_ERROR)								//sprawdzenie czy ramka siê wys³a³
			{
				std::cout << "That didn't work! " << WSAGetLastError() << std::endl;
				fclose(hPlik);
				closesocket(socketClient);
				WSACleanup();
				return 1;
			}


		}
		end = clock();	//zakoñczenie pomiaru
		double czas = (end - start) / (double)CLOCKS_PER_SEC;
		std::cout << "czas przesylu pliku: " << czas << " s" << std::endl;

		fclose(hPlik);													//zamkniêcie pliku

	}

	closesocket(socketClient);										//zwolnienie socketu

	WSACleanup();													//zwolnienie biblioteki

	return 0;
}