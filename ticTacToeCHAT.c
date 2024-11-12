#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For sleep in seconds (Linux/Unix)

#define MAX_SIZE 10  // Maximum grid size

int grid[MAX_SIZE][MAX_SIZE];
int gridSize;
int playerScore = 0, cpuScore = 0;

// Function Prototypes
void initializeGrid(int gridSize);
void displayGrid();
void cpu(int playerTurn);
void pvp(int playerTurn);
int checkWin(int player);
int fullGrid();
void gameMain(int mode);
void formatGameState(char *state, int playerTurn);
size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *data);
void getChatGPTMove(char *gameState, int *playerTurn, int *col, int *row);

int main() {
  int playAgain = 1;

  while (playAgain) {
    int mode;
    printf("Enter desired grid size (3 to 10): ");
    scanf("%d", &gridSize);

    while (gridSize < 3 || gridSize > 10) {
      printf("Invalid Grid Size! Please enter a grid size between 3-10: ");
      scanf("%d", &gridSize);
    }

    printf("Select game mode: 1 - for local PVP. 2 - Game against CPU: ");
    scanf("%d", &mode);

    while (mode < 1 || mode > 2) {
      printf("Invalid game mode! Please enter either game mode 1 or 2: ");
      scanf("%d", &mode);
    }

    gameMain(mode);

    printf("Scores - Player 1: %d, Player 2/CPU: %d\n", playerScore, cpuScore);

    // Ask the player if they want to play again
    printf("Would you like to play again? 1 - Yes, 2 - No: ");
    scanf("%d", &playAgain);

    // Reset scores and grid if they choose to play again
    if (playAgain == 1) {
      playerScore = 0;
      cpuScore = 0;
    } else {
      printf("Thanks for playing!\n");
      break;
    }
  }

  return 0;
}

void initializeGrid(int gridSize) {
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      grid[i][j] = 0;
    }
  }
}

void displayGrid() {
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      if (grid[i][j] == 1) {
        printf(" X ");
      } else if (grid[i][j] == 2) {
        printf(" O ");
      } else {
        printf(" . ");
      }
      if (j < gridSize - 1) {
        printf("|");
      }
    }
    printf("\n");
  }
}

int checkWin(int player) {
  // Check rows
  for (int i = 0; i < gridSize; i++) {
    int rowWin = 1;
    for (int j = 0; j < gridSize; j++) {
      if (grid[i][j] != player) {
        rowWin = 0;
        break;
      }
    }
    if (rowWin) return 1;
  }

  // Check columns
  for (int j = 0; j < gridSize; j++) {
    int colWin = 1;
    for (int i = 0; i < gridSize; i++) {
      if (grid[i][j] != player) {
        colWin = 0;
        break;
      }
    }
    if (colWin) return 1;
  }

  // Check main diagonal
  int diag1Win = 1;
  for (int i = 0; i < gridSize; i++) {
    if (grid[i][i] != player) {
      diag1Win = 0;
      break;
    }
  }
  if (diag1Win) return 1;

  // Check secondary diagonal
  int diag2Win = 1;
  for (int i = 0; i < gridSize; i++) {
    if (grid[i][gridSize - i - 1] != player) {
      diag2Win = 0;
      break;
    }
  }
  if (diag2Win) return 1;

  return 0;  // No win detected
}

void pvp(int playerTurn) {
  int row, col;
  do {
    printf("Enter row and column: ");
    scanf("%d %d", &row, &col);
  } while (row < 0 || col < 0 || row >= gridSize || col >= gridSize ||
           grid[row][col] != 0);

  grid[row][col] = playerTurn;
}

void cpu(int playerTurn) {
  printf("CPU's turn!\n");

  char gameState[200];
  formatGameState(gameState, playerTurn);  // Format the current game state

  printf("Sending the following game state to ChatGPT:\n%s\n", gameState);

  int row = -1;
  int col = -1;

  // Loop until ChatGPT suggests a valid move
  while (1) {
    getChatGPTMove(gameState, &playerTurn, &col,
                   &row);  // Get the move from ChatGPT
    printf("ChatGPT suggested move: Row %d, Column %d\n", row, col);

    // Check if the move is within bounds and on an empty spot
    if (row >= 0 && row < gridSize && col >= 0 && col < gridSize &&
        grid[row][col] == 0) {
      break;  // Valid move found, exit the loop
    } else {
      printf("ChatGPT suggested an invalid move. Trying again...\n");
    }
  }

  // Apply the valid move
  grid[row][col] = playerTurn;
}

int fullGrid() {
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      if (grid[i][j] == 0) {
        return 0;
      }
    }
  }
  return 1;
}

void gameMain(int mode) {
  initializeGrid(gridSize);
  int playerTurn = 1;

  while (1) {
    displayGrid();

    if (mode == 1 || playerTurn == 1) {
      pvp(playerTurn);  // Player's turn in PvP or player 1's turn
    } else {
      cpu(playerTurn);  // CPU's turn
    }

    if (checkWin(playerTurn)) {
      displayGrid();
      if (playerTurn == 1) {
        printf("Player 1 wins!\n");
        playerScore++;
      } else {
        printf("Player 2 (or CPU) wins!\n");
        cpuScore++;
      }
      break;
    }

    if (fullGrid()) {
      printf("It's a draw!\n");
      break;
    }

    playerTurn =
        3 - playerTurn;  // Switch turns between player 1 and 2 (or CPU)
  }
}

void formatGameState(char *state, int playerTurn) {
  int index = 0;
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      if (grid[i][j] == 1) {
        state[index++] = 'X';
      } else if (grid[i][j] == 2) {
        state[index++] = 'O';
      } else {
        state[index++] = '.';
      }
    }
    state[index++] = '\n';
  }
  state[index] = '\0';
}

size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t totalSize = size * nmemb;
  char *responseBuffer = (char *)data;

  if (strlen(responseBuffer) + totalSize < 2048) {
    strncat(responseBuffer, (char *)ptr, totalSize);
  } else {
    printf("Response buffer too small for the data.\n");
  }

  return totalSize;
}

void getChatGPTMove(char *gameState, int *playerTurn, int *col, int *row) {
  CURL *curl;
  CURLcode res;
  char *apiKey = getenv("OPENAI_API_KEY");
  char response[2048] = "";  // Increased buffer size for response

  if (!apiKey) {
    printf("API key is missing!\n");
    exit(1);
  }

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(
        headers,
        "Authorization: Bearer" "API_KEY");

    // Format the JSON payload
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "model", "gpt-3.5-turbo");

    // Create the messages array
    cJSON *messages = cJSON_CreateArray();

    // Add the system message
    cJSON *messageSystem = cJSON_CreateObject();
    cJSON_AddStringToObject(messageSystem, "role", "system");
    cJSON_AddStringToObject(
        messageSystem, "content",
        "You are a Tic-Tac-Toe expert. Your objective is to make the best "
        "moves possible to either win or prevent the first player from "
        "winning. The game ends when either player gets 3 of their symbols in "
        "a row (horizontally, vertically, or diagonally). Your top priority "
        "should be blocking the opponent from completing a row, column, or "
        "diagonal of their symbol. If there are no immediate threats, aim to "
        "set up a potential winning position for yourself. Your response "
        "format must be 'n m', where 'n' is the row number and 'm' is the "
        "column number. Both 'n' and 'm' must always be between 0 and 2. The "
        "move you "
        "select must be on an empty space — never place your symbol in a "
        "position where an 'X' or 'O' already exists. Always ensure your move "
        "is valid according to the current board state. Key Rules: Prevent the "
        "opponent from winning if they are one move away from completing a "
        "line (horizontally, vertically, or diagonally). If the opponent is "
        "not immediately threatening to win, prioritize moves that maximize "
        "your own chances of winning. Avoid making random moves — always "
        "select the most strategic option available. Only provide coordinates "
        "in the specified format 'n m'. Do not include explanations or "
        "additional text. Do not repeat moves or place symbols on occupied "
        "spaces. Example Scenarios: If the game state is X.O/n .../n ... ,Your "
        "valid moves are: 0 1, 1 0, 1 1, 1 2, 2 0, 2 1, 2 2. If the opponent "
        "can win in their next move, block them. Otherwise, aim to create a "
        "winning opportunity for yourself. If the game state is: X../n O../n "
        "... ,Invalid moves would be 0 0 and 1 0 since they are already "
        "occupied. The remaining valid moves should focus on preventing the "
        "opponent from setting up a win. This prompt is not an exhaustive list "
        "of game scenarios. Analyze each game state intelligently and "
        "strategically to make the optimal move.");
    cJSON_AddItemToArray(messages, messageSystem);

    // Add the user message
    cJSON *messageUser = cJSON_CreateObject();
    cJSON_AddStringToObject(messageUser, "role", "user");

    // Allocate memory for the gameStateString
    char gameStateString[1024];  // Adjust the size as needed
    snprintf(gameStateString, sizeof(gameStateString),
             "Here is the current game state: %s - My move as Player %d, what "
             "should I do?",
             gameState, *playerTurn);
    cJSON_AddStringToObject(messageUser, "content", gameStateString);
    cJSON_AddItemToArray(messages, messageUser);

    // Add the messages array to the data object
    cJSON_AddItemToObject(data, "messages", messages);

    // Convert the cJSON object to a string for the API request
    char *jsonString = cJSON_PrintUnformatted(data);
    if (jsonString == NULL) {
      printf("Failed to create JSON string.\n");
      cJSON_Delete(data);
      return;
    }

    // Debug: Print the JSON payload
    // printf("Formatted JSON payload:\n%s\n", jsonString);

    // Set up the CURL request
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "Curl request failed: %s\n", curl_easy_strerror(res));
      return;  // Exit if request fails
    }

    // Debug output for response
    // printf("Received response: %s\n", response);

    cJSON *jsonResponse = cJSON_Parse(response);
    if (jsonResponse != NULL) {
      cJSON *choices =
          cJSON_GetObjectItemCaseSensitive(jsonResponse, "choices");
      if (choices != NULL && cJSON_IsArray(choices)) {
        cJSON *firstChoice = cJSON_GetArrayItem(choices, 0);
        if (firstChoice != NULL) {
          cJSON *message =
              cJSON_GetObjectItemCaseSensitive(firstChoice, "message");
          if (message != NULL) {
            cJSON *content =
                cJSON_GetObjectItemCaseSensitive(message, "content");
            if (content != NULL) {
              // Safely parse the response
              int scanned =
                  sscanf(cJSON_GetStringValue(content), "%d %d", row, col);
              if (scanned != 2) {
                printf(
                    "Error: Failed to parse move coordinates. Received: %s\n",
                    cJSON_GetStringValue(content));
              }
            } else {
              printf("Error: 'content' not found in the response.\n");
            }
          } else {
            printf("Error: 'message' not found in the response.\n");
          }
        } else {
          printf("Error: No choices in the response.\n");
        }
      } else {
        printf("Error: 'choices' is missing or not an array.\n");
      }
      cJSON_Delete(jsonResponse);
    } else {
      printf("Error: Failed to parse JSON.\n");
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  curl_global_cleanup();
  usleep(1000000);
}
