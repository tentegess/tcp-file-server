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
#include<vector>
#include <string>
#include <map>
#include <cstring>
#include <cstdio>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)	//w��czenie ob�ugi funkcji fopen

#define BACKLOG 10				//maksymalna liczba zakolejkowanych po��cze�

//struktura dla nag��k�w i plik�w
struct Clients {
	char header[1024];
	int sock = 0;
	int status = 0;			//status headera
							//0-brak
							//1-upload
							//2-download
	size_t wielkosc_headera = 0;
	FILE* fPlik = nullptr;
	

	Clients(int fd) {}; //!< Konstruktor konwertuj�cy.
	Clients(void) { memset(header, '\0', sizeof(header)); }; //!< Domy�lny konstruktor.
	~Clients(void) {};
};

//funkcja por�wnuj�ca ilo�� ramek przes�anych do klienta
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

	WSADATA wsaData;			//inicjalizacja struktury WSADATA

	WORD version = MAKEWORD(2, 2); //wybranie wersji biblioteki WSADATA

	// Start WinSock
	int Result = WSAStartup(version, &wsaData); //Wywo�anie funkcji inicjuj�cej winsock
	if (Result != 0)	//kontrola poprawno�ci inicjalizacji
	{
		std::cout << "Nie uda�o si� rozpocz�� Winsock! " << Result;
		return 1;
	}

	std::vector<pollfd>clients;		//wektor przechowuj�cy adresy klient�w
	

	//----------------------------------------------------------------------------------------------------------------------------------------------

	std::map<int, Clients> client;
	

	//----------------------------------------------------------------------------------------------------------------------------------------------


	int socketServer = socket(AF_INET, SOCK_STREAM, 0);	//utworzenie gniazda sieciowego
	struct sockaddr_in hint;							//inicjalizacja struktury z adresem klienta
	hint.sin_family = AF_INET;							//wybranie rodziny adres�w dla servera (IPV4)
	hint.sin_port = htons(4200);						//wybranie portu na kt�rym dzia�a serwer
	hint.sin_addr.S_un.S_addr = ADDR_ANY;				//wybranie portu nas�uchiwania
	int addr_lengh = sizeof(hint);						//d�ugo�� adresu
	char ipstr[INET_ADDRSTRLEN];
	int bytesIn;
	int des;

	memset(hint.sin_zero, '\0', sizeof hint.sin_zero);	//wype�nienie sin_zero zerami
	pollfd tem;											//zainicjowanie struktury pollfd dla serwera							
	clients.push_back(tem);								//dodanie struktury serwera do wektora
	clients[0].fd = socketServer;						//dodanie gniazda serwera do struktury
	clients[0].events = POLLRDNORM;						//ustawienie ��danej flagi (odczyt bez blokowania)

											//powi�zanie adresu z socketem
	if (bind(socketServer, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {	
		std::cout << "Nie mo�na powi�za� socketu! " << WSAGetLastError() << std::endl;
		closesocket(socketServer);					//zwolnienie socketu
		WSACleanup();								//zwolnienie biblioteki
		return 1;
	}

	if (listen(socketServer, BACKLOG) != 0) {		//nas�uchiwanie po��cze� od klient�w
		std::cout << "nie s�ucham" << std::endl;
		closesocket(socketServer);					//zwolnienie socketu
		WSACleanup();								//zwolnienie biblioteki
		return 1;
	}

	int poll;
	const int sizeOfBuf = 1024;
	char buf[sizeOfBuf];					//bufor na odczyt bliku
	

	int test2 = 0;

	while (1) {
		
													//inicjalizacja funkcja WSAPoll 
		if ((poll = WSAPoll(clients.data(), clients.size(), -1)) == SOCKET_ERROR) {
			std::cout << "poll nie dziaua" << std::endl;	//usuni�cie danych klient�w				
			for (const auto& c : client) {					//i zamkni�cie serwera
				closesocket(c.second.sock);					//przy niepoprawnej inicjalizacji
				if (c.second.fPlik != NULL)					//wsapoll
					fclose(c.second.fPlik);
					printf("ds");
			}
			for (pollfd c : clients) {
				closesocket(c.fd);
			}
			clients.clear();
			client.clear(); 
			closesocket(socketServer);	//zwolnienie socketu
			WSACleanup(); //zwolnienie biblioteki
			return 1;
		}
		
	
		for (int i = 0; i < clients.size(); i++) {
			if (clients[i].revents & POLLRDNORM && clients[0].fd == clients[i].fd) {
				//oczekiwanie i zaakceptowanie po��czenia od klienta
				if ((des = accept(socketServer, (struct sockaddr*)&hint, (socklen_t*)&addr_lengh)) == INVALID_SOCKET) {
					std::cout << "nie dzia�a" << std::endl;
					i--; 
					test2 -= 1;
					continue;
				}
				test2 += 1;
						
				std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;

				if (inet_ntop(AF_INET, &(hint.sin_addr), ipstr, INET_ADDRSTRLEN) == NULL) {//konwerjsa adresu klienta do postaci tekstowej
					std::cout << "nie udalo sie pobrac adresu klienta" << std::endl;
					i--;
					test2 -= 1;
					continue;
				}

				pollfd temp;									//utworzenie struktury zaakcpetowanego klienta
				temp.fd = des;									//przypisania gniazda klienta
				temp.events = POLLRDNORM;						//przypisanie flagi klientowi
				clients.push_back(temp);

				std::string s(ipstr);
				std::cout << "adres klienta: " << s << std::endl;			//wy�wietlenie adresu klienta
				std::cout <<"polaczeni klienci: " << clients.size() - 1 << std::endl;		//wy�wietlenie liczby po��czonych klient�w
				
				Clients temp2;												//utworzenie tymczasowej struktury dla nowego klienta
				client.insert(std::make_pair(des , temp2));					//powi�zanie klienta z desktyptorem
				client[des].sock = des;										//przypisanie deskryptora do socketu
			}											

			else if (clients[i].revents & POLLRDNORM  && client[clients[i].fd].status == 0){	//gdy klient nie ma przypisanego headera	
				char temp[sizeOfBuf];
				memset(temp, '\0', sizeof(temp));
				bytesIn = recv(clients[i].fd, temp, sizeof(temp), 0);		//odebranie headera

				std::strncat(client[clients[i].fd].header, temp, bytesIn);	//do��czenie ci�gu odebranego od klienta do nag��wka	
				client[clients[i].fd].wielkosc_headera += bytesIn;			//ustalenie wielko�ci headera
		
				std::cout << client[clients[i].fd].header << std::endl;

				const auto pos_start = client[clients[i].fd].header; // Pocz�tek bufora
				const auto pos_end = pos_start + client[clients[i].fd].wielkosc_headera; // Koniec bufora
				const auto pos_nl = std::find(pos_start, pos_end, '\n'); // Pozycja nowej linii

				if (pos_nl != pos_end) {							//je�li znak nowej linii nie jest na ko�cu nag��wka
					const auto pos_sp = std::find(pos_start, pos_nl, ' '); // Pozycja spacji

					if (pos_sp != pos_nl) {						//je�li wyst�pi�a spacja
						*pos_nl = *pos_sp = '\0';				//usuni�cie spacji i znaku nowej linii
						std::string name(pos_sp+1);		

						std::cout << name << std::endl;
						const int left = client[clients[i].fd].wielkosc_headera - (pos_nl - pos_start) - 1;	//sprawdzenie co znajduje si�  
																											//po znaku nowej linii

						if (strcmp(pos_start, "UPLOAD") == 0) {												//je�li klient przes�a� UPLOAD
							client[clients[i].fd].status = 1;												//zmiana statusu na 1

							FILE* fPlik = fopen(name.c_str(), "wb");										//otwarcie pliku do zapisu
							if (fPlik == NULL) {															//sprawdzenie czy poprawnie
								std::cout << "Nie uda�o si� otworzy� pliku!" << std::endl;					//utworzono plik
								closesocket(client[clients[i].fd].sock);	//zamkni�cie socketu w mapie
								client.erase(clients[i].fd);				//usuni�cie klienta z mapy
								closesocket(clients[i].fd);			//zamkni�cie socketu klienta
								clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
								i -= 1;
								test2 -= 1;
								continue;
							}
							client[clients[i].fd].fPlik = fPlik;		//dodanie pliku do mapy
							if (left > 0) {
								fwrite(pos_nl + 1, 1, left, client[clients[i].fd].fPlik);	//zapis do pliku
								printf(pos_nl);
							}					
						}
						else if (strcmp(pos_start, "DOWNLOAD") == 0) {		//je�li klient przes�a� DOWNLOAD
							client[clients[i].fd].status = 2;

							FILE* fPlik = fopen(name.c_str(), "rb");	//otwarcie pliku do odczytu
							if (fPlik == NULL) {						//sprawdzenie czy plik istnieje
								std::cout << "Nie uda�o si� otworzy� pliku!" << std::endl;
								closesocket(client[clients[i].fd].sock);
								client.erase(clients[i].fd);
								closesocket(clients[i].fd);			//zamkni�cie socketu klienta
								clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
								i -= 1;
								test2 -= 1;
								continue;
							}
							client[clients[i].fd].fPlik = fPlik;
							clients[i].events |= POLLWRNORM;	//zmiana flagi na flage do odczytu
						}
						else {
							std::cout << "Nie poprawne polecenie" << std::endl;		//gdy nag��wek przes�any przez klienta jest niepoprawny
							closesocket(client[clients[i].fd].sock);
							client.erase(clients[i].fd);
							closesocket(clients[i].fd);			//zamkni�cie socketu klienta
							clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
							i -= 1;
							test2 -= 1;
							continue;
						}
					}
					else {
						std::cout << "Nie poprawne polecenie" << std::endl;		//gdy format nag��wla jest niepoprawny
						closesocket(client[clients[i].fd].sock);
						client.erase(clients[i].fd);
						closesocket(clients[i].fd);			//zamkni�cie socketu klienta
						clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
						i -= 1;
						test2 -= 1;
						continue;
					}
				}
				else {
					if (client[clients[i].fd].wielkosc_headera >= 128) {	//gdy nag��wek jest wi�kszy ni� 128
						std::cout << "Nie poprawne polecenie" << std::endl;
						closesocket(client[clients[i].fd].sock);
						client.erase(clients[i].fd);
						closesocket(clients[i].fd);			//zamkni�cie socketu klienta
						clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
						i -= 1;
						test2 -= 1;
						continue;
					}
				}
				std::cout << client[clients[i].fd].status << std::endl;
			}

			else if (clients[i].revents & POLLRDNORM && client[clients[i].fd].status == 1) {		//je�li klient posiada flage do odczytu

				bytesIn = recv(clients[i].fd, buf, sizeof(buf), 0);		//otrzymanie ramki od klienta


				if (bytesIn == SOCKET_ERROR)				 //sprawdzenie poprawno�ci otrzymania danych					
				{
					std::cout << "Error receiving from client " << WSAGetLastError() << std::endl;
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkni�cie socketu klienta
					clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}

				if (client[clients[i].fd].fPlik == NULL) {		//sprawdzenie czy plik isnieje
					std::cout << "Podano z�y plik ";
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkni�cie socketu klienta
					clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}

				fwrite(buf, 1, bytesIn, client[clients[i].fd].fPlik);		//zapisanie danych do pliku
				fflush(client[clients[i].fd].fPlik);						//wyczyszczenie bufora zapisu do pliku

				if (bytesIn == 0) {
					std::cout << "otrzymano plik" << std::endl;
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkni�cie socketu klienta
					clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}
			}

			else if (clients[i].revents & POLLWRNORM && client[clients[i].fd].status == 2) {		//je�li klient posiada flage do zapisu
				int count;
				count = fread(&buf, 1, sizeof(buf), client[clients[i].fd].fPlik);	//odczyt pliku

				int Send = send_all(clients[i].fd, buf, count);			//wys�anie ramki o zadanej wielko�ci
				if (Send == SOCKET_ERROR)								//sprawdzenie czy ramka si� wys�a�a
				{
					std::cout << "That didn't work! " << WSAGetLastError() << std::endl;
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkni�cie socketu klienta
					clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}

				if (count <= 0) {						//gdy przes�ano wszystkie dane
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);
					closesocket(clients[i].fd);			//zamkni�cie socketu klienta
				}

			}

			else if (clients[i].revents & POLLHUP) {
				if(client[clients[i].fd].fPlik!=NULL)
					fclose(client[clients[i].fd].fPlik);	//zamkni�cie pliku gdy klient si� roz��czy�
				closesocket(client[clients[i].fd].sock);
				client.erase(clients[i].fd);
				closesocket(clients[i].fd);			//zamkni�cie socketu klienta
				clients.erase(clients.begin() + i); //usuni�cie klienta z wektora
				i -= 1;
				test2 -= 1;
				std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;
				std::cout << "polaczeni klienci: " << clients.size() - 1 << std::endl;		//wy�wietlenie liczby po��czonych klient�w
				continue;
			}
		}
	}
	
	closesocket(socketServer);					//zwolnienie socketu
	WSACleanup();								//zwolnienie biblioteki
	return 0;
}
