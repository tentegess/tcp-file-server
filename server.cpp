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
#pragma warning(disable:4996)	//w³¹czenie ob³ugi funkcji fopen

#define BACKLOG 10				//maksymalna liczba zakolejkowanych po³¹czeñ

//struktura dla nag³ó³ków i plików
struct Clients {
	char header[1024];
	int sock = 0;
	int status = 0;			//status headera
							//0-brak
							//1-upload
							//2-download
	size_t wielkosc_headera = 0;
	FILE* fPlik = nullptr;
	

	Clients(int fd) {}; //!< Konstruktor konwertuj¹cy.
	Clients(void) { memset(header, '\0', sizeof(header)); }; //!< Domyœlny konstruktor.
	~Clients(void) {};
};

//funkcja porównuj¹ca iloœæ ramek przes³anych do klienta
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

	WSADATA wsaData;			//inicjalizacja struktury WSADATA

	WORD version = MAKEWORD(2, 2); //wybranie wersji biblioteki WSADATA

	// Start WinSock
	int Result = WSAStartup(version, &wsaData); //Wywo³anie funkcji inicjuj¹cej winsock
	if (Result != 0)	//kontrola poprawnoœci inicjalizacji
	{
		std::cout << "Nie uda³o siê rozpocz¹æ Winsock! " << Result;
		return 1;
	}

	std::vector<pollfd>clients;		//wektor przechowuj¹cy adresy klientów
	

	//----------------------------------------------------------------------------------------------------------------------------------------------

	std::map<int, Clients> client;
	

	//----------------------------------------------------------------------------------------------------------------------------------------------


	int socketServer = socket(AF_INET, SOCK_STREAM, 0);	//utworzenie gniazda sieciowego
	struct sockaddr_in hint;							//inicjalizacja struktury z adresem klienta
	hint.sin_family = AF_INET;							//wybranie rodziny adresów dla servera (IPV4)
	hint.sin_port = htons(4200);						//wybranie portu na którym dzia³a serwer
	hint.sin_addr.S_un.S_addr = ADDR_ANY;				//wybranie portu nas³uchiwania
	int addr_lengh = sizeof(hint);						//d³ugoœæ adresu
	char ipstr[INET_ADDRSTRLEN];
	int bytesIn;
	int des;

	memset(hint.sin_zero, '\0', sizeof hint.sin_zero);	//wype³nienie sin_zero zerami
	pollfd tem;											//zainicjowanie struktury pollfd dla serwera							
	clients.push_back(tem);								//dodanie struktury serwera do wektora
	clients[0].fd = socketServer;						//dodanie gniazda serwera do struktury
	clients[0].events = POLLRDNORM;						//ustawienie ¿¹danej flagi (odczyt bez blokowania)

											//powi¹zanie adresu z socketem
	if (bind(socketServer, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {	
		std::cout << "Nie mo¿na powi¹zaæ socketu! " << WSAGetLastError() << std::endl;
		closesocket(socketServer);					//zwolnienie socketu
		WSACleanup();								//zwolnienie biblioteki
		return 1;
	}

	if (listen(socketServer, BACKLOG) != 0) {		//nas³uchiwanie po³¹czeñ od klientów
		std::cout << "nie s³ucham" << std::endl;
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
			std::cout << "poll nie dziaua" << std::endl;	//usuniêcie danych klientów				
			for (const auto& c : client) {					//i zamkniêcie serwera
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
				//oczekiwanie i zaakceptowanie po³¹czenia od klienta
				if ((des = accept(socketServer, (struct sockaddr*)&hint, (socklen_t*)&addr_lengh)) == INVALID_SOCKET) {
					std::cout << "nie dzia³a" << std::endl;
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
				std::cout << "adres klienta: " << s << std::endl;			//wyœwietlenie adresu klienta
				std::cout <<"polaczeni klienci: " << clients.size() - 1 << std::endl;		//wyœwietlenie liczby po³¹czonych klientów
				
				Clients temp2;												//utworzenie tymczasowej struktury dla nowego klienta
				client.insert(std::make_pair(des , temp2));					//powi¹zanie klienta z desktyptorem
				client[des].sock = des;										//przypisanie deskryptora do socketu
			}											

			else if (clients[i].revents & POLLRDNORM  && client[clients[i].fd].status == 0){	//gdy klient nie ma przypisanego headera	
				char temp[sizeOfBuf];
				memset(temp, '\0', sizeof(temp));
				bytesIn = recv(clients[i].fd, temp, sizeof(temp), 0);		//odebranie headera

				std::strncat(client[clients[i].fd].header, temp, bytesIn);	//do³¹czenie ci¹gu odebranego od klienta do nag³ówka	
				client[clients[i].fd].wielkosc_headera += bytesIn;			//ustalenie wielkoœci headera
		
				std::cout << client[clients[i].fd].header << std::endl;

				const auto pos_start = client[clients[i].fd].header; // Pocz¹tek bufora
				const auto pos_end = pos_start + client[clients[i].fd].wielkosc_headera; // Koniec bufora
				const auto pos_nl = std::find(pos_start, pos_end, '\n'); // Pozycja nowej linii

				if (pos_nl != pos_end) {							//jeœli znak nowej linii nie jest na koñcu nag³ówka
					const auto pos_sp = std::find(pos_start, pos_nl, ' '); // Pozycja spacji

					if (pos_sp != pos_nl) {						//jeœli wyst¹pi³a spacja
						*pos_nl = *pos_sp = '\0';				//usuniêcie spacji i znaku nowej linii
						std::string name(pos_sp+1);		

						std::cout << name << std::endl;
						const int left = client[clients[i].fd].wielkosc_headera - (pos_nl - pos_start) - 1;	//sprawdzenie co znajduje siê  
																											//po znaku nowej linii

						if (strcmp(pos_start, "UPLOAD") == 0) {												//jeœli klient przes³a³ UPLOAD
							client[clients[i].fd].status = 1;												//zmiana statusu na 1

							FILE* fPlik = fopen(name.c_str(), "wb");										//otwarcie pliku do zapisu
							if (fPlik == NULL) {															//sprawdzenie czy poprawnie
								std::cout << "Nie uda³o siê otworzyæ pliku!" << std::endl;					//utworzono plik
								closesocket(client[clients[i].fd].sock);	//zamkniêcie socketu w mapie
								client.erase(clients[i].fd);				//usuniêcie klienta z mapy
								closesocket(clients[i].fd);			//zamkniêcie socketu klienta
								clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
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
						else if (strcmp(pos_start, "DOWNLOAD") == 0) {		//jeœli klient przes³a³ DOWNLOAD
							client[clients[i].fd].status = 2;

							FILE* fPlik = fopen(name.c_str(), "rb");	//otwarcie pliku do odczytu
							if (fPlik == NULL) {						//sprawdzenie czy plik istnieje
								std::cout << "Nie uda³o siê otworzyæ pliku!" << std::endl;
								closesocket(client[clients[i].fd].sock);
								client.erase(clients[i].fd);
								closesocket(clients[i].fd);			//zamkniêcie socketu klienta
								clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
								i -= 1;
								test2 -= 1;
								continue;
							}
							client[clients[i].fd].fPlik = fPlik;
							clients[i].events |= POLLWRNORM;	//zmiana flagi na flage do odczytu
						}
						else {
							std::cout << "Nie poprawne polecenie" << std::endl;		//gdy nag³ówek przes³any przez klienta jest niepoprawny
							closesocket(client[clients[i].fd].sock);
							client.erase(clients[i].fd);
							closesocket(clients[i].fd);			//zamkniêcie socketu klienta
							clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
							i -= 1;
							test2 -= 1;
							continue;
						}
					}
					else {
						std::cout << "Nie poprawne polecenie" << std::endl;		//gdy format nag³ówla jest niepoprawny
						closesocket(client[clients[i].fd].sock);
						client.erase(clients[i].fd);
						closesocket(clients[i].fd);			//zamkniêcie socketu klienta
						clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
						i -= 1;
						test2 -= 1;
						continue;
					}
				}
				else {
					if (client[clients[i].fd].wielkosc_headera >= 128) {	//gdy nag³ówek jest wiêkszy ni¿ 128
						std::cout << "Nie poprawne polecenie" << std::endl;
						closesocket(client[clients[i].fd].sock);
						client.erase(clients[i].fd);
						closesocket(clients[i].fd);			//zamkniêcie socketu klienta
						clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
						i -= 1;
						test2 -= 1;
						continue;
					}
				}
				std::cout << client[clients[i].fd].status << std::endl;
			}

			else if (clients[i].revents & POLLRDNORM && client[clients[i].fd].status == 1) {		//jeœli klient posiada flage do odczytu

				bytesIn = recv(clients[i].fd, buf, sizeof(buf), 0);		//otrzymanie ramki od klienta


				if (bytesIn == SOCKET_ERROR)				 //sprawdzenie poprawnoœci otrzymania danych					
				{
					std::cout << "Error receiving from client " << WSAGetLastError() << std::endl;
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkniêcie socketu klienta
					clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}

				if (client[clients[i].fd].fPlik == NULL) {		//sprawdzenie czy plik isnieje
					std::cout << "Podano z³y plik ";
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkniêcie socketu klienta
					clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
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
					closesocket(clients[i].fd);			//zamkniêcie socketu klienta
					clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}
			}

			else if (clients[i].revents & POLLWRNORM && client[clients[i].fd].status == 2) {		//jeœli klient posiada flage do zapisu
				int count;
				count = fread(&buf, 1, sizeof(buf), client[clients[i].fd].fPlik);	//odczyt pliku

				int Send = send_all(clients[i].fd, buf, count);			//wys³anie ramki o zadanej wielkoœci
				if (Send == SOCKET_ERROR)								//sprawdzenie czy ramka siê wys³a³a
				{
					std::cout << "That didn't work! " << WSAGetLastError() << std::endl;
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);			//zamkniêcie socketu klienta
					clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
					i -= 1;
					test2 -= 1;
					continue;
				}

				if (count <= 0) {						//gdy przes³ano wszystkie dane
					fclose(client[clients[i].fd].fPlik);
					closesocket(client[clients[i].fd].sock);
					client.erase(clients[i].fd);
					closesocket(clients[i].fd);
					closesocket(clients[i].fd);			//zamkniêcie socketu klienta
				}

			}

			else if (clients[i].revents & POLLHUP) {
				if(client[clients[i].fd].fPlik!=NULL)
					fclose(client[clients[i].fd].fPlik);	//zamkniêcie pliku gdy klient siê roz³¹czy³
				closesocket(client[clients[i].fd].sock);
				client.erase(clients[i].fd);
				closesocket(clients[i].fd);			//zamkniêcie socketu klienta
				clients.erase(clients.begin() + i); //usuniêcie klienta z wektora
				i -= 1;
				test2 -= 1;
				std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;
				std::cout << "polaczeni klienci: " << clients.size() - 1 << std::endl;		//wyœwietlenie liczby po³¹czonych klientów
				continue;
			}
		}
	}
	
	closesocket(socketServer);					//zwolnienie socketu
	WSACleanup();								//zwolnienie biblioteki
	return 0;
}
