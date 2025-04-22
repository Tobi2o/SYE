#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>

int forfeit = 0, sock_fd;

typedef enum
{
	DEFAULT = 0, BOAT, MISSED_SHOT, SUNKEN_BOAT
}gridState;

typedef enum
{
	SERVER_LISTEN = 0,
	SERVER_CLIENT_CONNECT,
	CLIENT_CONNECTING,
	CLIENT_CONNECTED,
	BOAT_PLACE,
	OPPONENT_WAIT,
	ATTACK_CELL,
	USER_ATTACK,
	OPPONENT_ATTACK,
	MISS,
	HIT,
	USER_GRID,
	OPPONENT_GRID,
	USER_WON,
	USER_LOST,
	INV_CELL_ATTACKED,
	INV_CELL_VAL,
	INV_BOAT_STACK,
	USER_FORFEIT,
	OPPONENT_FORFEIT
}stringsIdx;

const char *strings[] = {
	"Server listening on port",
	"Client connected",
	"Connecting to server",
	"Connected to server successfully",
	"Please enter 3 grid cells to place your boats into",
	"Waiting for opponent...",
	"Please enter a grid cell to attack your opponent",
	"You attacked",
	"Your opponent attacked",
	"MISS",
	"HIT",
	"User grid",
	"Opponent grid",
	"YOU WON ^_^",
	"YOU LOST T_T",
	"Invalid ! You already attacked this cell",
	"Invalid ! Please enter a grid cell index between 0-F",
	"Invalid ! You cannot place 2 boats on the same grid cell",
	"You forfeited the game",
	"Your opponent forfeited the game"
};

const char symbols[] = { '~', '0', 'o', 'X' };

/**
 * @brief Forfeits the game when receiving SIGSTP (Ctrl+Z) signal
 *
 * Signal routine to override default behaviour of SIGSTP. When this routine is called due to the reception of SIGSTP (Ctrl+Z in the shell) it sends a message on the socket to warn that the user wants to forfeit the game and then closes the socket file descriptor before exiting
*/
void quitGame(int sig)
{
    if(sig == SIGTSTP)
    {
        char data = OPPONENT_FORFEIT;
        send(sock_fd, &data, 1, 0); //
        close(sock_fd);
        printf("%s: %s\n", strings[USER_FORFEIT], strings[USER_LOST]);
        exit(0);
    }
}

/**
 * @brief Converts a single hexadecimal character to its decimal value
 *
 * The hexadecimal digit may be lowercase or uppercase
 *
 * @param digit the hexadecimal digit to convert
 *
 * @returns The decimal value of the hexadecimal digit given or -1 if the given digit isn't hexadecimal
*/
int charToHex(const int digit)
{
	if(digit >= '0' && digit <= '9') return digit - '0';

	int lowerDigit = tolower(digit);
	if(lowerDigit >= 'a' && lowerDigit <= 'f') return 10 + (lowerDigit - 'a');

	return -1;
}

/**
 * @brief Recovers a hexadecimal digit representing a grid cell from the user and returns its decimal value
 *
 * Only the first input hexadecimal digit is returned. All other input characters are safely discarded. If the given hex digit is invalid, an error message is displayed and a new input is required
 *
 * @returns The decimal value of the selected grid cell
*/
char promptCell()
{
    char userInput = 0;
    int isValidInput = 0;

    do {
        printf("Enter a cell index (0-F): ");
        scanf(" %c", &userInput); // Use a space before %c to skip whitespace

        while (getchar() != '\n'); // Clear input buffer

        if ((isdigit(userInput) && userInput >= '0' && userInput <= '9') || charToHex(userInput) != -1) {
            isValidInput = 1;
        } else {
            printf("%s\n", strings[INV_CELL_VAL]);
        }
    } while (!isValidInput);

    return charToHex(userInput);
}

/**
 * @brief Prints a grid
 *
 * Prints the given board with the following pattern and symbols:
 * ~ ~ ~ 0
 * ~ o ~ ~
 * ~ 0 ~ o
 * ~ ~ ~ X
 *
 * ~: Default (water)
 * 0: Boat
 * o: Missed shot
 * X: Sunken boat
*/
void printBoard(const char *grid)
{
    for(int i = 0; i < 16; i++)
    {
        printf("%c ", symbols[grid[i]]);

        if((i + 1) % 4 == 0) printf("\n");
    }
}

/**
 * @brief Recovers size bytes from the socket_fd file descriptor and stores them in the buf buffer
 *
 * @returns -1 if an error occured, 0 otherwise
*/
int socketRecv(const int sock_fd, char *buf, const size_t size)
{
    ssize_t n = recv(sock_fd, buf, size, 0); //ssize_t used for a count of bytes or an error indication.
    if(n == -1){
        perror("Recv failed\n");
        return -1;
    }
    return 0;
}

/**
 * @brief Sends size bytes from the buf buffer to the sock_fd file descriptor
 *
 * @returns -1 if an error occured, 0 otherwise
*/
int socketSend(const int sock_fd, char *buf, const size_t size)
{
    int res = send(sock_fd, buf, size, 0);

    if(res == -1){
        perror("Send failed\n");
        return -1;
    }

    return 0;
}

/**
 * @brief Creates a server that waits for a client before returning
 *
 * The server is created and then waits for a client to connect with accept. Once a client is connected, the server's file descriptor is closed and the client's file descriptor is returned
 *
 * @returns -1 if an error occured, the client's file descriptor as returned by accept otherwise
*/
int server_create(const int port)
{
    int server_fd;
    struct sockaddr_in address;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd == -1){
        printf("Could not create socket\n");
        return -1;
    }

     // Set the address structure: AF_INET (IPv4), any incoming interface, and the specified port.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        printf("Bind failed\n");
        return -1;
    }

    if(listen(server_fd, 1) < 0){
        printf("Listen failed\n");
        return -1;
    }

    printf("%s %d\n", strings[SERVER_LISTEN] , port);

    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    if(new_socket == -1){
        printf("Accept failed\n");
        return -1;
    }

    // Return the file descriptor of the newly accepted client socket.
    return new_socket;
}

/**
 * @brief Creates a client that connects to a server before returning
 *
 * @returns -1 if an error occured, the file descriptor of the server as returned by connect otherwise
*/
int client_connect(const char *ip_addr, const int port)
{
    int sock;
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1 ){
        printf("Could not create socket\n");
        return -1;
    }

    // Set the address structure for the server to connect to: AF_INET (IPv4), the specified port, and IP address.
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr) <= 0){
        printf("Invalid address\n");
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Connection failed\n");
        return -1;
    }

	return sock;
}
int main(int argc, char const *argv[])
{
	char userGrid[16], advGrid[16];
	const char *ip_addr = "127.0.0.1";
	int server = 0, fd = 0, userInput, port = 5000;

	// Verify arguments and configure
	for (size_t i = 0; i < argc; i++)
	{
		if(i == 0) continue;
		if(strcmp(argv[i], "-s") == 0) server = 1;
		else if(strcmp(argv[i], "-a") == 0)
		{
			ip_addr = argv[i + 1];
			i++;
		}
		else if(strcmp(argv[i], "-p") == 0)
		{
			port = atoi(argv[i + 1]);
			i++;
		}
		else
		{
			printf("Unknown argument\n");
			return -1;
		}
	}

	// TO COMPLETE: Init game boards and ask user for boat placement
	memset(userGrid, DEFAULT, sizeof(userGrid));
    memset(advGrid, DEFAULT, sizeof(advGrid));

	// TO COMPLETE: Print board

    printf("%s\n", strings[BOAT_PLACE]);
    for(int i = 0; i < 3; ++i){
        int cell;
        do{
            cell = promptCell();
            if(userGrid[cell] == BOAT){
                printf("%s\n", strings[INV_BOAT_STACK]);
            }
        }while(userGrid[cell] == BOAT);

        userGrid[cell] = BOAT;

    }
    printf("\n");
    
    printf("%s\n", strings[USER_GRID]);
    printBoard(userGrid);

    if(server){

        fd = server_create(port); // Create a server socket.
        if(fd == -1) {
            perror("Server create failed\n");
            return -1;
        }

        printf("%s\n", strings[SERVER_CLIENT_CONNECT]);

    }else{
        printf("%s\n", strings[CLIENT_CONNECTING]);
        fd = client_connect(ip_addr, port);

        if(fd == -1) {
            perror("Client connect failed\n");
            return -1;
        }

        printf("%s\n", strings[CLIENT_CONNECTED]);
    }

	// TO COMPLETE: Init socket communication according to role

    sock_fd = fd;

	// TO COMPLETE (part 2): Setup signal handler for forfeit

    signal(SIGTSTP, quitGame);


    // Run game according to role
    int userBoats = 3, advBoats = 3;
    char data = 0;
    int isInputValid = 0;
    char userGameInput  = 0;
    int firstIter  = server ? 1 : 0;

    // Main game loop. It runs indefinitely until the game ends (win, lose, or forfeit).
    while(1)
    {
        isInputValid = 0;

        // If it's the client's turn to receive an attack.
        if(!firstIter){ 
            // Indicate waiting for the opponent's attack.
            printf("%s\n", strings[OPPONENT_WAIT]); 

            // Receive the attack from the opponent.
            if(socketRecv(fd, &data, 1) == -1){
                perror("Error msg received"); 
                break; // Exit the loop on error.
            }

            // Check for forfeit from the opponent.
            if(data == OPPONENT_FORFEIT){
                forfeit = 1;
                break; // Exit the loop.
            }

            // Validate the received data.
            if(data < 0 || data > 15){
                perror("Invalid index"); 
                break; // Exit the loop.
            }

            // Process the received attack.
            if(userGrid[data] == BOAT){
                printf("%s %X: %s\n",strings[OPPONENT_ATTACK], data, strings[HIT]); 
                userGrid[data] = SUNKEN_BOAT; 
                userBoats--; 
                data = HIT; 
            } else if(userGrid[data] == DEFAULT){
                printf("%s %X: %s\n",strings[OPPONENT_ATTACK], data, strings[MISS]); 
                userGrid[data] = MISSED_SHOT; 
                data = MISS; 
            }

            // Send the result of the attack back to the opponent.
            if(socketSend(fd, &data, 1) == -1){
                perror("Error msg send"); 
                break; // Exit the loop.
            }

            // Print the user's grid after the attack.
            printf("%s\n", strings[USER_GRID]);
            printBoard(userGrid);

            // Switch turns to the server.
            firstIter = 1; 
        } else{ // Server's turn to send an attack.
            printf("%s\n", strings[ATTACK_CELL]); 

            // Validate user input.
            while (!isInputValid)
            {
                userGameInput = promptCell(); 
                if(advGrid[userGameInput] != DEFAULT){
                    printf("%s\n", strings[INV_CELL_ATTACKED]); 
                    continue; // Ask for input again.
                }
                isInputValid = 1; // Mark input as valid.
            }

            // Store the user input in 'data' and send it to the opponent.
            data = userGameInput;
            if(socketSend(fd, &data, 1) == -1){
                perror("Error msg send"); 
                break; // Exit the loop on error.
            }

            // Receive the result of the attack from the opponent.
            if(socketRecv(fd, &data, 1) == -1){
                perror("Error msg received"); 
                break; // Exit the loop on error.
            }

            // Process the result of the attack.
            if(data == HIT){
                printf("%s %X: %s\n", strings[USER_ATTACK], userGameInput, strings[HIT]); 
                advGrid[userGameInput] = SUNKEN_BOAT; 
                advBoats--; 
            } else if(data == MISS){
                printf("%s %X: %s\n", strings[USER_ATTACK], userGameInput, strings[MISS]); 
                advGrid[userGameInput] = MISSED_SHOT; 
            } else if(data == OPPONENT_FORFEIT){
                forfeit = 1; 
                break; // Exit the loop.
            }

            // Print the adversary's grid after the attack.
            printf("%s\n", strings[OPPONENT_GRID]);
            printBoard(advGrid);

            // Switch turns to the client.
            firstIter = 0; 
        }

        // Check for end of game conditions.
        if(userBoats == 0){
            printf("%s\n", strings[USER_LOST]); 
            break; // End the game.
        }
        if(advBoats == 0){
            printf("%s\n", strings[USER_WON]); 
            break; // End the game.
        }

            // Check for end of game conditions.
            if(userBoats == 0){
                printf("%s\n", strings[USER_LOST]);
                break;
            }
            if(advBoats == 0){
                printf("%s\n", strings[USER_WON]);
                break;
            }
        }

    // TO COMPLETE (part 2): Check forfeit (out of game loop without all user or opponent's boats destroyed)
    if(forfeit){
        printf("%s: %s\n", strings[OPPONENT_FORFEIT], strings[USER_WON]);
    }

    // TO COMPLETE: Close socket file descriptor
    close(fd);

    return 0;
}



