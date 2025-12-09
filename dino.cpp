
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
#include <string.h>
struct Player
{
    char name[50];
    int score;
    int difficulty;
};
struct User
{
    char username[50];
    char password[50];
    int highScore;
      int highScoreEasy;
    int highScoreMedium;
    int highScoreHard;

};
struct LeaderboardEntry {
    char username[50];
    int score;
    int difficulty;
};



int t, speed=40;
int highScore=0;
int difficulty=1;
Player players[100];
int playerCount = 0;
char currentPlayerName[50] = "";
// User management
User users[1000];
int userCount = 0;
const char* USER_FILE = "users.dat";
const char* HIGHSCORES_FILE = "highscores.dat";
const char* SCORE_CSV_FILE = "scores.csv";

// Global current user tracking
User* currentUser = NULL;
// Global leaderboard
LeaderboardEntry globalLeaderboard[10];
int leaderboardCount = 0;
const char* LEADERBOARD_FILE = "leaderboard.dat";

// Forward declarations
void loadGlobalLeaderboard();
void saveGlobalLeaderboard();
int updateLeaderboardWithCurrentPlayer(const char* username, int score, int diff);
void appendScoreToCSV(const char* username, int score, int diff);



void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

// File I/O functions for users
void saveUsers()
{
    FILE* file = fopen(USER_FILE, "wb");
    if(file)
    {
        fwrite(&userCount, sizeof(int), 1, file);
        fwrite(users, sizeof(User), userCount, file);
        fclose(file);
    }
}

void loadUsers()
{
    FILE* file = fopen(USER_FILE, "rb");
    if(file)
    {
        fread(&userCount, sizeof(int), 1, file);
        if(userCount > 1000) userCount = 1000;
         // Initialize all users first to avoid garbage values
        for(int i = 0; i < 1000; i++)
        {
            users[i].highScoreEasy = 0;
            users[i].highScoreMedium = 0;
            users[i].highScoreHard = 0;
              users[i].highScore = 0;
        }
        fread(users, sizeof(User), userCount, file);
        // After reading, validate and fix any corrupted high score values
        for(int i = 0; i < userCount; i++)
        {
                 if(users[i].highScoreEasy < 0 || users[i].highScoreEasy > 1000000) users[i].highScoreEasy = 0;
            if(users[i].highScoreMedium < 0 || users[i].highScoreMedium > 1000000) users[i].highScoreMedium = 0;
            if(users[i].highScoreHard < 0 || users[i].highScoreHard > 1000000) users[i].highScoreHard = 0;
            if(users[i].highScore < 0 || users[i].highScore > 1000000) users[i].highScore = 0;

        }
        fclose(file);
    }
     // Also load difficulty-specific highscores
    loadGlobalLeaderboard();
}

void loadGlobalLeaderboard()
{

      // Build leaderboard from all saved player scores
    FILE* file = fopen(LEADERBOARD_FILE, "rb");
    leaderboardCount = 0;

    if(file)
    {
        fread(&leaderboardCount, sizeof(int), 1, file);
        if(leaderboardCount > 10) leaderboardCount = 10;
        fread(globalLeaderboard, sizeof(LeaderboardEntry), leaderboardCount, file);
        fclose(file);
    }

    // Also add from players array
    for(int i = 0; i < playerCount && leaderboardCount < 10; i++)
    {
        // Check if already in leaderboard
        int found = 0;
        for(int j = 0; j < leaderboardCount; j++)
        {
            if(strcmp(globalLeaderboard[j].username, players[i].name) == 0 &&
               globalLeaderboard[j].score == players[i].score)
            {
                found = 1;
                break;
            }
        }
        if(!found)
        {
            strcpy(globalLeaderboard[leaderboardCount].username, players[i].name);
            globalLeaderboard[leaderboardCount].score = players[i].score;
            globalLeaderboard[leaderboardCount].difficulty = players[i].difficulty;
            leaderboardCount++;
        }
    }

    // Sort leaderboard
    for(int i = 0; i < leaderboardCount - 1; i++)
    {
        for(int j = i + 1; j < leaderboardCount; j++)
        {
            if(globalLeaderboard[j].score > globalLeaderboard[i].score)
            {
                LeaderboardEntry temp = globalLeaderboard[i];
                globalLeaderboard[i] = globalLeaderboard[j];
                globalLeaderboard[j] = temp;
            }
        }
    }

    // Keep only top 10
    if(leaderboardCount > 10) leaderboardCount = 10;
}

void saveGlobalLeaderboard()
{
    FILE* file = fopen(LEADERBOARD_FILE, "wb");
    if(file)
    {
        fwrite(&leaderboardCount, sizeof(int), 1, file);
        fwrite(globalLeaderboard, sizeof(LeaderboardEntry), leaderboardCount, file);
        fclose(file);
    }
}

int updateLeaderboardWithCurrentPlayer(const char* username, int score, int diff)
{
    int inserted = 0;

    // Check if this score should be in top 10
    if(leaderboardCount < 10 || score > globalLeaderboard[leaderboardCount - 1].score)
    {
        // Find position to insert
        int pos = -1;
        for(int i = 0; i < leaderboardCount; i++)
        {
            if(score > globalLeaderboard[i].score)
            {
                pos = i;
                break;
            }
        }

        if(pos == -1 && leaderboardCount < 10)
        {
            pos = leaderboardCount;
        }

        if(pos != -1)
        {
            // Shift down
            if(leaderboardCount < 10) leaderboardCount++;
            for(int i = leaderboardCount - 1; i > pos; i--)
            {
                globalLeaderboard[i] = globalLeaderboard[i - 1];
            }

            // Insert
            strcpy(globalLeaderboard[pos].username, username);
            globalLeaderboard[pos].score = score;
            globalLeaderboard[pos].difficulty = diff;

            // Keep only top 10
            if(leaderboardCount > 10) leaderboardCount = 10;

            inserted = 1;
            saveGlobalLeaderboard();
        }
    }

    return inserted;
}

void saveUserHighscore(const char* username, int score, int diff)

{



    FILE* file = fopen(HIGHSCORES_FILE, "r+b");
    if(file)
    {
        int count = 0;
        User tempUsers[1000];

        // Read existing highscores
        if(fread(&count, sizeof(int), 1, file) == 1)
        {
            if(count > 1000) count = 1000;
            fread(tempUsers, sizeof(User), count, file);
        }

        // Update or add highscore for user
        int found = 0;
        for(int i = 0; i < count; i++)
        {
            if(strcmp(tempUsers[i].username, username) == 0)
            {



                if(diff == 1 && score > tempUsers[i].highScoreEasy)
                    tempUsers[i].highScoreEasy = score;
                else if(diff == 2 && score > tempUsers[i].highScoreMedium)
                    tempUsers[i].highScoreMedium = score;
                else if(diff == 3 && score > tempUsers[i].highScoreHard)
                    tempUsers[i].highScoreHard = score;

                       // Update overall high score (best across all difficulties)
                int bestScore = tempUsers[i].highScoreEasy;
                if(tempUsers[i].highScoreMedium > bestScore) bestScore = tempUsers[i].highScoreMedium;
                if(tempUsers[i].highScoreHard > bestScore) bestScore = tempUsers[i].highScoreHard;
                tempUsers[i].highScore = bestScore;


                found = 1;
                break;
            }
        }

        if(!found && count < 1000)
        {
            strcpy(tempUsers[count].username, username);

                        tempUsers[count].highScoreEasy = (diff == 1) ? score : 0;

tempUsers[count].highScoreMedium = (diff == 2) ? score : 0;
            tempUsers[count].highScoreHard = (diff == 3) ? score : 0;
            // Overall high score is the best across all difficulties
            int bestScore = tempUsers[count].highScoreEasy;
            if(tempUsers[count].highScoreMedium > bestScore) bestScore = tempUsers[count].highScoreMedium;
            if(tempUsers[count].highScoreHard > bestScore) bestScore = tempUsers[count].highScoreHard;
            tempUsers[count].highScore = bestScore;
            count++;
        }

        // Write back
        rewind(file);
        fwrite(&count, sizeof(int), 1, file);
        fwrite(tempUsers, sizeof(User), count, file);
        fclose(file);
    }
    else
    {
        // Create new file
        file = fopen(HIGHSCORES_FILE, "wb");
        if(file)
        {
            int count = 1;
            User tempUser;
            strcpy(tempUser.username, username);
                   tempUser.highScoreEasy = (diff == 1) ? score : 0;
            tempUser.highScoreMedium = (diff == 2) ? score : 0;
            tempUser.highScoreHard = (diff == 3) ? score : 0;
                  // Overall high score is the best across all difficulties
            int bestScore = tempUser.highScoreEasy;
            if(tempUser.highScoreMedium > bestScore) bestScore = tempUser.highScoreMedium;
            if(tempUser.highScoreHard > bestScore) bestScore = tempUser.highScoreHard;
            tempUser.highScore = bestScore;

            fwrite(&count, sizeof(int), 1, file);
            fwrite(&tempUser, sizeof(User), 1, file);
            fclose(file);
        }
    }
}
int loadUserHighscore(const char* username, int diff)
{
    FILE* file = fopen(HIGHSCORES_FILE, "rb");
    if(file)
    {
        int count = 0;
        if(fread(&count, sizeof(int), 1, file) == 1)
        {
            if(count > 1000) count = 1000;
            User tempUsers[1000];
            fread(tempUsers, sizeof(User), count, file);

            for(int i = 0; i < count; i++)
            {
                if(strcmp(tempUsers[i].username, username) == 0)
                {
                    fclose(file);
                      // Return high score for the specific difficulty
                    if(diff == 1) return tempUsers[i].highScoreEasy;
                    else if(diff == 2) return tempUsers[i].highScoreMedium;
                    else if(diff == 3) return tempUsers[i].highScoreHard;
                    else return 0;
                }
            }
        }
        fclose(file);
    }
    return 0;
}

void loadUserDifficultyHighscores(const char* username, int* easy, int* medium, int* hard)
{
     *easy = 0;  // Default to 10 instead of 0
    *medium = 0;
    *hard = 0;


    FILE* file = fopen(HIGHSCORES_FILE, "rb");
    if(file)
    {
        int count = 0;
        if(fread(&count, sizeof(int), 1, file) == 1)
        {
            if(count > 1000) count = 1000;
            User tempUsers[1000];
              // Initialize all to 0 first to avoid garbage values
            for(int i = 0; i < 1000; i++)
            {
                tempUsers[i].highScoreEasy = 0;
                tempUsers[i].highScoreMedium = 0;
                tempUsers[i].highScoreHard = 0;
            }
            fread(tempUsers, sizeof(User), count, file);

            for(int i = 0; i < count; i++)
            {
                if(strcmp(tempUsers[i].username, username) == 0)
                {
                    *easy = tempUsers[i].highScoreEasy;
                    *medium = tempUsers[i].highScoreMedium;
                    *hard = tempUsers[i].highScoreHard;

      // Ensure values are valid (non-negative)
                    if(*easy < 0) *easy = 0;
                    if(*medium < 0) *medium = 0;
                    if(*hard < 0) *hard = 0;
                    fclose(file);
                    return;
                }
            }
        }
        fclose(file);
    }
}

User* findUser(const char* username)
{
    for(int i = 0; i < userCount; i++)
    {
        if(strcmp(users[i].username, username) == 0)
        {
            return &users[i];
        }
    }
    return NULL;
}

int signup()
{
    char username[50], password[50], confirmPassword[50];

    while(true)
    {
        system("cls");
        gotoxy(31,10);
        printf("================== SIGN UP ======================");
        gotoxy(31,12);
        printf("Username: ");
        gotoxy(31,13);
        printf("Password: ");
        gotoxy(31,14);
        printf("Confirm Password: ");
        gotoxy(42,12);
        scanf("%s", username);
        gotoxy(42,13);
        scanf("%s", password);
        gotoxy(51,14);
        scanf("%s", confirmPassword);

        // Validation
        if(strlen(username) == 0)
        {
            gotoxy(28,16);
            printf("Username cannot be empty!");
            delay(1500);
            continue;
        }

        if(strlen(password) == 0)
        {
            gotoxy(28,16);
            printf("Password cannot be empty!");
            delay(1500);
            continue;
        }

        if(strcmp(password, confirmPassword) != 0)
        {
            gotoxy(28,16);
            printf("Passwords do not match!");
            delay(1500);
            continue;
        }

        // Check if username already exists
        if(findUser(username) != NULL)
        {
            gotoxy(28,16);
            printf("Username already exists! Please choose another.");
            delay(2000);
            continue;
        }

        // Create new user
        if(userCount < 1000)
        {
            strcpy(users[userCount].username, username);
            strcpy(users[userCount].password, password);
            users[userCount].highScore = 0;
             users[userCount].highScoreEasy = 0;
            users[userCount].highScoreMedium = 0;
            users[userCount].highScoreHard = 0;
            userCount++;
            saveUsers();

            gotoxy(33,16);
            printf("Account created successfully!");
            delay(1500);
            return 1;
        }
        else
        {
            gotoxy(28,16);
            printf("Maximum users reached!");
            delay(1500);
            return 0;
        }
    }
}




int login()
{
    char username[50], password[50];

    int attempts = 0;

    while(attempts < 3)
    {
        system("cls");
        gotoxy(31,12);
        printf("================== LOGIN SYSTEM ======================");
        gotoxy(31,14);
        printf("Username: ");
        gotoxy(31,15);
        printf("Password: ");
        gotoxy(42,14);
        scanf("%s", username);
        gotoxy(42,15);
        scanf("%s", password);


        User* user = findUser(username);
        if(user != NULL && strcmp(user->password, password) == 0)
        {
            currentUser = user;
            strcpy(currentPlayerName, username);
                      highScore = loadUserHighscore(username, difficulty);


                 // Load difficulty-specific highscores
            int easy, medium, hard;
            loadUserDifficultyHighscores(username, &easy, &medium, &hard);
            user->highScoreEasy = easy;
            user->highScoreMedium = medium;
            user->highScoreHard = hard;

         // Set highScore based on current difficulty level
            if(difficulty == 1)
                highScore = user->highScoreEasy;
            else if(difficulty == 2)
                highScore = user->highScoreMedium;
            else if(difficulty == 3)
                highScore = user->highScoreHard;
            else
                        highScore = 0;



            gotoxy(33,17);
            printf("Login Successful!");
            gotoxy(28,18);
            printf("Welcome, %s! High Score: %d", username, highScore);
            delay(2000);
            return 1;
        }
        else
        {
            attempts++;
            gotoxy(28,17);
            printf("Invalid credentials! Attempts: %d/3", attempts);
            delay(1500);
            if(attempts >= 3)
            {
                gotoxy(26,18);
                printf("Too many failed attempts. Exiting...");
                delay(2000);
                return 0;
            }
        }
    }
    return 0;
}
int showLoginMenu()
{
    int choice;
    while(true)
    {
        system("cls");
        gotoxy(31,10);
        printf("============== WELCOME TO DINO GAME ==============");
        gotoxy(31,12);
        printf("1. Login");
        gotoxy(31,13);
        printf("2. Sign Up");
        gotoxy(31,14);
        printf("3. Exit");
        gotoxy(31,16);
        printf("==================================================");
        gotoxy(31,18);
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
        case 1:
            if(login())
                return 1;
            break;
        case 2:
            signup();
            break;
        case 3:
            return 0;
        default:
            gotoxy(28,20);
            printf("Invalid choice! Press any key...");
            getch();
            break;
        }
    }
}





void showMenu()
{
    system("cls");
    gotoxy(25,6);
    printf("Logged in as: %s", currentPlayerName);
    gotoxy(25,7);
    printf("High Score: %d", highScore);
    gotoxy(25,8);
    printf("==================== DINO GAME MENU ====================");
    gotoxy(25,10);
    printf("1. Start Game");
    gotoxy(25,11);
    printf("2. Difficulty Level");
    gotoxy(25,12);
    printf("3. Instructions");
    gotoxy(25,13);
    printf("4. Score List");
    gotoxy(25,14);
    printf("5. Settings");
    gotoxy(25,15);
    printf("6. Logout");
    gotoxy(25,16);
    printf("=========================================================");
    gotoxy(25,18);
    printf("Enter your choice: ");
}

void showInstructions()
{
    system("cls");
    gotoxy(25,8);
    printf("==================== INSTRUCTIONS ====================");
    gotoxy(20,10);
    printf("Press SPACE to make the dinosaur jump");
    gotoxy(20,11);
    printf("Avoid the obstacles coming from the right");
    gotoxy(20,12);
    printf("Your score increases as you pass obstacles");
    gotoxy(20,13);
    printf("Game speed increases as your score increases");
    gotoxy(20,14);
    printf("Press X during game to return to menu");
    gotoxy(20,15);
    printf("Press L during game to logout");
    gotoxy(25,17);
    printf("=====================================================");
    gotoxy(25,19);
    printf("Press any key to return to menu...");
    getch();
}

void showDifficultyLevel()
{
    system("cls");
    gotoxy(25,8);
    printf("==================== DIFFICULTY LEVEL ====================");
    gotoxy(25,10);
    printf("Current Difficulty: ");
    if(difficulty==1) printf("Easy");
    else if(difficulty==2) printf("Medium");
    else if(difficulty==3) printf("Hard");
    gotoxy(25,12);
    printf("1. Easy");
    gotoxy(25,13);
    printf("2. Medium");
    gotoxy(25,14);
    printf("3. Hard");
    gotoxy(25,16);
    printf("=========================================================");
    gotoxy(25,18);
    printf("Select difficulty (1-3): ");
    int choice;
    scanf("%d", &choice);
    if(choice>=1 && choice<=3)
    {
        difficulty = choice;
        if(difficulty==1) speed=40;
        else if(difficulty==2) speed=30;
        else if(difficulty==3) speed=20;

         // Update high score for the new difficulty level
        if(strlen(currentPlayerName) > 0)
        {
            highScore = loadUserHighscore(currentPlayerName, difficulty);
        }
        else
        {
            highScore = 0;
        }


        gotoxy(25,20);
        printf("Difficulty changed successfully!");
        delay(1500);
    }
}

void showScoreList()
{
    system("cls");
    gotoxy(20,2);
    printf("==================== SCORE LIST ====================");
    gotoxy(20,3);
    printf("Player Name          Difficulty    Score");
    gotoxy(20,4);
    printf("----------------------------------------------------");

    if(playerCount == 0)
    {
        gotoxy(20,6);
        printf("No players yet. Play the game to add your score!");
    }
    else
    {
        Player sortedPlayers[100];
        for(int i = 0; i < playerCount; i++)
        {
            sortedPlayers[i] = players[i];
        }

        for(int i = 0; i < playerCount - 1; i++)
        {
            for(int j = i + 1; j < playerCount; j++)
            {
                if(sortedPlayers[j].score > sortedPlayers[i].score)
                {
                    Player temp = sortedPlayers[i];
                    sortedPlayers[i] = sortedPlayers[j];
                    sortedPlayers[j] = temp;
                }
            }
        }

        int startY = 5;
        int maxDisplay = 20;
        int displayCount = (playerCount < maxDisplay) ? playerCount : maxDisplay;

        for(int i = 0; i < displayCount; i++)
        {
            gotoxy(20, startY + i);
            char diffStr[10];
            if(sortedPlayers[i].difficulty == 1) strcpy(diffStr, "Easy");
            else if(sortedPlayers[i].difficulty == 2) strcpy(diffStr, "Medium");
            else if(sortedPlayers[i].difficulty == 3) strcpy(diffStr, "Hard");
            else strcpy(diffStr, "Unknown");

            printf("%-20s %-12s %d", sortedPlayers[i].name, diffStr, sortedPlayers[i].score);
        }

        if(playerCount > maxDisplay)
        {
            gotoxy(20, startY + maxDisplay);
            printf("... and %d more players", playerCount - maxDisplay);
        }
    }

    gotoxy(20,27);
    printf("====================================================");
    gotoxy(20,28);

    printf("Press any key to return to menu...");
    getch();
}

void showSettings()
{
    system("cls");
    gotoxy(25,8);
    printf("==================== SETTINGS ====================");
    gotoxy(25,10);
    printf("Current User: %s", currentPlayerName);
    gotoxy(25,11);
    printf("Current Settings:");
    gotoxy(25,12);
    printf("Difficulty: ");
    if(difficulty==1) printf("Easy");
    else if(difficulty==2) printf("Medium");
    else if(difficulty==3) printf("Hard");
    gotoxy(25,13);
    printf("Game Speed: %d", speed);
    gotoxy(25,14);
    printf("High Score: %d", highScore);
    gotoxy(25,16);
    printf("==================================================");
    gotoxy(25,18);
    printf("Press any key to return to menu...");
    getch();
}
void showGameSideBar(int currentScore, bool forceRefresh = false)
{

     static int lastScore = -1;
    static int lastDifficulty = -1;
    static int lastHighScore = -1;

    static bool firstDraw = true;


       // Panel positioned in top-right corner (console is 82 cols wide)
     // Panel starts at col 58, leaving game area 0-57 on left
    const int PANEL_X = 58;

      // Force refresh if requested (e.g., after difficulty change or score save)
    if(forceRefresh)
    {
        firstDraw = true;
        lastScore = -1;
        lastDifficulty = -1;
        lastHighScore = -1;

        // Reload high scores when forcing refresh to ensure we have latest values
        if(strlen(currentPlayerName) > 0)
        {
            int easy = 0, medium = 0, hard = 0;
            loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
            if(currentUser != NULL)
            {
                currentUser->highScoreEasy = easy;
                currentUser->highScoreMedium = medium;
                currentUser->highScoreHard = hard;
            }
        }
    }




   // Detect game restart (score reset to 0 after a game) or new game start
    if(currentScore == 0 && (lastScore > 0 || forceRefresh))
    {
        firstDraw = true; // Reset for new game
         // Reload high scores after game ends or when starting new game to show updated values

         if(strlen(currentPlayerName) > 0)
        {
            int easy = 0, medium = 0, hard = 0;
            loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
            if(currentUser != NULL)
            {
                currentUser->highScoreEasy = easy;
                currentUser->highScoreMedium = medium;
                currentUser->highScoreHard = hard;
            }
        }
    }
        int highScore = 0;  // High score for current difficulty level

    int easy = 0, medium = 0, hard = 0;
        if(strlen(currentPlayerName) > 0)
    {

        loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);

        // Update currentUser if it exists


      if(currentUser != NULL)
        {
            currentUser->highScoreEasy = easy;
            currentUser->highScoreMedium = medium;
            currentUser->highScoreHard = hard;
        }

 // Get high score for current difficulty level
        if(difficulty == 1)
            highScore = easy;
        else if(difficulty == 2)
            highScore = medium;
        else if(difficulty == 3)
            highScore = hard;
        else
            highScore = 0;



    }
      else
    {
        // No player name, so no high score
         highScore = 0;
    }

   // Ensure high score is not negative (should be 0 if not set)
      if(highScore < 0) highScore = 0;
     // Always redraw if force refresh, first draw, or if values changed
    // Force refresh ensures we always update when starting a new game
    if(forceRefresh || firstDraw || currentScore != lastScore || difficulty != lastDifficulty ||
         highScore != lastHighScore)
    {
        int yPos = 1;

        // Draw border and header (only if first draw)
        if(firstDraw)
        {
                gotoxy(PANEL_X, yPos++);
            printf("+======================+");
            gotoxy(PANEL_X, yPos++);
            printf("|    PLAYER INFO       |");
            gotoxy(PANEL_X, yPos++);
            printf("+======================+");
            yPos++;
        }
       // Always display player name
        yPos = 5;
        char displayName[20];
        if(strlen(currentPlayerName) > 0)
        {
            strncpy(displayName, currentPlayerName, 19);
            displayName[19] = '\0';

        }
        else
        {
               strcpy(displayName, "Guest");
        }
  gotoxy(PANEL_X, yPos++);
        printf("| Player: %-13s |", displayName);
          // Update Score at fixed position (yPos 6)
        gotoxy(PANEL_X, yPos);
        printf("| Score:  %-13d |", currentScore);
        lastScore = currentScore;
        yPos++;

        // Update Difficulty at fixed position (yPos 7)
        gotoxy(PANEL_X, yPos);
        printf("| Diff:   ");
        if(difficulty == 1) printf("Easy          |");
        else if(difficulty == 2) printf("Medium        |");
        else if(difficulty == 3) printf("Hard          |");
        else printf("Unknown       |");
        lastDifficulty = difficulty;
        yPos++;

       // Draw separator at fixed position (yPos 8)
        if(firstDraw)
        {
            yPos++;
              gotoxy(PANEL_X, yPos++);
            printf("+----------------------+");

        }
        else
        {
          yPos = 9; // Skip to high score position
        }

  // Always display High Score at fixed position (yPos 9)
        // This shows the best high score across all difficulties
        // It automatically saves and updates when player scores higher

        if(firstDraw && yPos < 9) yPos = 9;
        if(!firstDraw) yPos = 9;


         // Clear the line first to prevent duplicate display
        gotoxy(PANEL_X, yPos);
        printf("|                      |");  // Clear with spaces

   // Display High Score - best score ever achieved (saves and updates automatically)

        gotoxy(PANEL_X, yPos);
            printf("| High:   %-13d |", highScore);

        // Update tracking variable
        lastHighScore = highScore;
           // Draw bottom border (only if first draw)
        if(firstDraw)
        {
            yPos++;
            gotoxy(PANEL_X, yPos);
            printf("+======================+");
        }

        firstDraw = false;
    }

}

int getPlayerName()
{

    // Player name is already set from login, just verify
    if(strlen(currentPlayerName) == 0)
    {
        system("cls");
        gotoxy(30,12);
        printf("No user logged in!");
        delay(1500);
        return 0;
    }
    return 1;
}

void savePlayerScore(int score)
{
    if(strlen(currentPlayerName) == 0 || score <= 0)
        return;

    // Always append this run to a human-readable CSV file in the project folder
    appendScoreToCSV(currentPlayerName, score, difficulty);

    // Save to players array for score list
    if(playerCount < 100)
    {
        strcpy(players[playerCount].name, currentPlayerName);
        players[playerCount].score = score;
        players[playerCount].difficulty = difficulty;
        playerCount++;
    }

       if(currentUser != NULL)
    {
         // Update difficulty-specific highscore only if score is higher
        bool updated = false;
        if(difficulty == 1 && score > currentUser->highScoreEasy)


        {
                        currentUser->highScoreEasy = score;

            highScore = score;
             updated = true;
        }


        else if(difficulty == 2 && score > currentUser->highScoreMedium)
           {
            currentUser->highScoreMedium = score;
            highScore=score;
            updated=true;
           }
        else if(difficulty == 3 && score > currentUser->highScoreHard)
          {
            currentUser->highScoreHard = score;
               highScore = score;
            updated = true;
        }

        // Update overall high score (best across all difficulties)
        int bestScore = currentUser->highScoreEasy;
        if(currentUser->highScoreMedium > bestScore) bestScore = currentUser->highScoreMedium;
        if(currentUser->highScoreHard > bestScore) bestScore = currentUser->highScoreHard;
        currentUser->highScore = bestScore;

  // Only save if we updated a high score
        if(updated)
        {
            saveUserHighscore(currentUser->username, score, difficulty);
        }


    // Reload high scores from file to ensure consistency
        int easy = 0, medium = 0, hard = 0;
        loadUserDifficultyHighscores(currentUser->username, &easy, &medium, &hard);
        currentUser->highScoreEasy = easy;
        currentUser->highScoreMedium = medium;
        currentUser->highScoreHard = hard;

        // Update highScore variable for current difficulty
        if(difficulty == 1)
            highScore = easy;
        else if(difficulty == 2)
            highScore = medium;
        else if(difficulty == 3)
            highScore = hard;


        saveUsers(); // Update user file with new highscore

          // Update global leaderboard
        updateLeaderboardWithCurrentPlayer(currentUser->username, score, difficulty);
        loadGlobalLeaderboard();

    }
}

void getup()
{
    system("cls");
    gotoxy(2,2);
    printf("Press X to Menu, Space to Jump, L to Logout");

    gotoxy(1,25);
       for(int x=0; x<57; x++)  // Stop before panel area (panel starts at col 58)
        printf("ß");
       // Initialize and draw the player info panel with force refresh to ensure high scores are loaded
    // Reload high scores for current user to ensure they're up to date
       // This is critical when starting a new game after finishing one, to show the updated high score
    if(strlen(currentPlayerName) > 0)
    {
        int easy = 0, medium = 0, hard = 0;
        loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
         // Update currentUser if it exists
        if(currentUser != NULL)
        {
       currentUser->highScoreEasy = easy;
            currentUser->highScoreMedium = medium;
            currentUser->highScoreHard = hard;
        }
    }
    // Force refresh to ensure high score is displayed correctly when starting new game
    showGameSideBar(0, true);


}

void ds(int jump=0){

    static int a=1;

    if(jump==0)
        t=0;
    else if(jump==2)
        t--;
    else t++;
    gotoxy(2,15-t);
    printf("                 ");
    gotoxy(2,16-t);
    printf("         ÜÛßÛÛÛÛÜ");
    gotoxy(2,17-t);
    printf("         ÛÛÛÛÛÛÛÛ");
    gotoxy(2,18-t);
    printf("         ÛÛÛÛÛßßß");
    gotoxy(2,19-t);
    printf(" Û      ÜÛÛÛÛßßß ");
    gotoxy(2,20-t);
    printf(" ÛÛÜ  ÜÛÛÛÛÛÛÜÜÜ ");
    gotoxy(2,21-t);
    printf(" ßÛÛÛÛÛÛÛÛÛÛÛ  ß ");
    gotoxy(2,22-t);
    printf("   ßÛÛÛÛÛÛÛß     ");
    gotoxy(2,23-t);
    if(jump==1 || jump==2)
    {
        printf("    ÛÛß ßÛ       ");
        gotoxy(2,24-t);
        printf("    ÛÜ   ÛÜ      ");
    }
    else if(a==1)
    {
        printf("    ßÛÛß  ßßß    ");
        gotoxy(2,24-t);
        printf("      ÛÜ         ");
        a=2;
    }
    else if(a==2)
    {
        printf("     ßÛÜ ßÛ      ");
        gotoxy(2,24-t);
        printf("          ÛÜ     ");
        a=1;
    }
    gotoxy(2,25-t);
    if(jump!=0)
    {
        printf("                ");
    }
    else
    {
        // Draw ground line, stopping before panel area (panel starts at col 58)
        for(int x=0; x<55; x++)  // 55 chars from col 2 = stops at col 57
            printf("ßß");
    }
    delay(speed);
}
int obj(int reset=0, int *currentScore=NULL)

{
    static int x=0,scr=0;
    if(reset==1)
    {
        x=0;
        scr=0;
        if(currentScore != NULL)
            *currentScore = 0;
        return 0;
    }
    if(x==56 && t<4)
    {
        savePlayerScore(scr);

        if(currentScore != NULL)
            *currentScore = scr;

                  // Force refresh panel to show updated high score after game ends
        showGameSideBar(scr, true);

        scr=0;
        x=0;

        if(difficulty==1) speed=40;
        else if(difficulty==2) speed=30;
        else if(difficulty==3) speed=20;
        gotoxy(36,8);
        printf("Game Over");
        getch();
        gotoxy(36,8);
        printf("         ");
        return 1;
    }
    gotoxy(74-x,20);
    printf("Û    Û ");
    gotoxy(74-x,21);
    printf("Û    Û ");
    gotoxy(74-x,22);
    printf("ÛÜÜÜÜÛ ");
    gotoxy(74-x,23);
    printf("  Û    ");
    gotoxy(74-x,24);
    printf("  Û  " );
    x++;
    if(x==73)
    {
        x=0;
        scr++;
        // Score is now displayed in the right panel, no need to update here
        if(speed>20)
            speed--;
    }
    if(currentScore != NULL)
        *currentScore = scr;
    return 0;
}
int startGame()
{

    if(!getPlayerName())
    {
        return 0;
    }

    char ch;
    int i;
    int gameOver = 0;
    int currentScore = 0;
    t=0;
    obj(1, &currentScore);
    if(difficulty==1) speed=40;
    else if(difficulty==2) speed=30;
    else if(difficulty==3) speed=20;
    getup();

    while(true)
    {
        while(!kbhit())
        {
            ds();
            gameOver = obj(0, &currentScore);
           showGameSideBar(currentScore);
            if(gameOver)
                return 0;
        }
        ch=getch();
        if(ch==' ')
        {
            for(i=0; i<10; i++)
            {
                ds(1);
                gameOver = obj(0, &currentScore);
                 showGameSideBar(currentScore);
                if(gameOver)
                {
                    savePlayerScore(currentScore);
                    return 0;
                }
            }

            for(i=0; i<10; i++)
            {
                ds(2);
                gameOver = obj(0, &currentScore);
               showGameSideBar(currentScore);
                if(gameOver)
                {
                    savePlayerScore(currentScore);
                    return 0;
                }
            }
        }
        else if (ch=='x' || ch=='X')
        {
            savePlayerScore(currentScore);
            return 0;
        }
        else if (ch=='l' || ch=='L')
        {
            savePlayerScore(currentScore);
            system("cls");
            gotoxy(28,14);
            printf("Logged out successfully!");
            delay(1500);
            return 1;
        }
    }

}

void appendScoreToCSV(const char* username, int score, int diff)
{
    if(username == NULL || username[0] == '\0' || score <= 0)
        return;

    FILE* file = fopen(SCORE_CSV_FILE, "a");
    if(!file)
        return;

    const char* diffStr = "Unknown";
    if(diff == 1) diffStr = "Easy";
    else if(diff == 2) diffStr = "Medium";
    else if(diff == 3) diffStr = "Hard";

    // Optional timestamp column so you know when the score was recorded
    time_t now = time(NULL);
    struct tm* tinfo = localtime(&now);
    char timebuf[32] = "";
    if(tinfo != NULL)
    {
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tinfo);
    }

    // CSV format: username,difficulty,score,timestamp
    fprintf(file, "%s,%s,%d,%s\n", username, diffStr, score,
            (timebuf[0] != '\0') ? timebuf : "");

    fclose(file);
}
int main()
{
    system("mode con: lines=29 cols=82");
    int choice;
    int logoutFlag = 0;

    // Load users from file on startup
    loadUsers();


    while(true)
    {
        // Show login/signup menu
        if(!showLoginMenu())
        {
            return 0;
        }

        logoutFlag = 0;
        while(!logoutFlag)
        {
            showMenu();
            fflush(stdin);
            scanf("%d", &choice);

            switch(choice)
            {
            case 1:
                if(startGame() == 1)
                    logoutFlag = 1;
                break;
            case 2:
                showDifficultyLevel();
                break;
            case 3:
                showInstructions();
                break;
            case 4:
                showScoreList();
                break;
            case 5:
                showSettings();
                break;
            case 6:
                // Logout
                currentUser = NULL;
                strcpy(currentPlayerName, "");
                highScore = 0;
                system("cls");
                gotoxy(30,14);
                // printf("Thank you for playing!");
                printf("Logged out successfully!");
                delay(1500);
                logoutFlag = 1;
                break;
            default:
                gotoxy(25,20);
                printf("Invalid choice! Press any key...");
                getch();
                break;
            }
        }
    }

    return 0;
}
/*
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
#include <string.h>
struct Player
{
    char name[50];
    int score;
    int difficulty;
};
struct User
{
    char username[50];
    char password[50];
    int highScore;
    int highScoreEasy;
    int highScoreMedium;
    int highScoreHard;
};
struct LeaderboardEntry
{
    char username[50];
    int score;
    int difficulty;
};
int t, speed=40;
int highScore=0;
int difficulty=1;
Player players[100];
int playerCount = 0;
char currentPlayerName[50] = "";
// User management
User users[1000];
int userCount = 0;
const char* USER_FILE = "users.dat";
const char* HIGHSCORES_FILE = "highscores.dat";
const char* SCORE_CSV_FILE = "scores.csv";
// Global current user tracking
User* currentUser = NULL;
// Global leaderboard
LeaderboardEntry globalLeaderboard[10];
int leaderboardCount = 0;
const char* LEADERBOARD_FILE = "leaderboard.dat";
// Forward declarations
void loadGlobalLeaderboard();
void saveGlobalLeaderboard();
int updateLeaderboardWithCurrentPlayer(const char* username, int score, int diff);
void appendScoreToCSV(const char* username, int score, int diff);
void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
// File I/O functions for users
void saveUsers()
{
    FILE* file = fopen(USER_FILE, "wb");
    if(file)
    {
        fwrite(&userCount, sizeof(int), 1, file);
        fwrite(users, sizeof(User), userCount, file);
        fclose(file);
    }
}
void loadUsers()
{
    FILE* file = fopen(USER_FILE, "rb");
    if(file)
    {
        fread(&userCount, sizeof(int), 1, file);
        if(userCount > 1000) userCount = 1000;
        // Initialize all users first to avoid garbage values
        for(int i = 0; i < 1000; i++)
        {
            users[i].highScoreEasy = 0;
            users[i].highScoreMedium = 0;
            users[i].highScoreHard = 0;
            users[i].highScore = 0;
        }
        fread(users, sizeof(User), userCount, file);
        // After reading, validate and fix any corrupted high score values
        for(int i = 0; i < userCount; i++)
        {
            if(users[i].highScoreEasy < 0 || users[i].highScoreEasy > 1000000) users[i].highScoreEasy = 0;
            if(users[i].highScoreMedium < 0 || users[i].highScoreMedium > 1000000) users[i].highScoreMedium = 0;
            if(users[i].highScoreHard < 0 || users[i].highScoreHard > 1000000) users[i].highScoreHard = 0;
            if(users[i].highScore < 0 || users[i].highScore > 1000000) users[i].highScore = 0;
        }
        fclose(file);
    }
    // Also load difficulty-specific highscores
    loadGlobalLeaderboard();
}
void loadGlobalLeaderboard()
{
    // Build leaderboard from all saved player scores
    FILE* file = fopen(LEADERBOARD_FILE, "rb");
    leaderboardCount = 0;
    if(file)
    {
        fread(&leaderboardCount, sizeof(int), 1, file);
        if(leaderboardCount > 10) leaderboardCount = 10;
        fread(globalLeaderboard, sizeof(LeaderboardEntry), leaderboardCount, file);
        fclose(file);
    }
    // Also add from players array
    for(int i = 0; i < playerCount && leaderboardCount < 10; i++)
    {
        // Check if already in leaderboard
        int found = 0;
        for(int j = 0; j < leaderboardCount; j++)
        {
            if(strcmp(globalLeaderboard[j].username, players[i].name) == 0 &&
                    globalLeaderboard[j].score == players[i].score)
            {
                found = 1;
                break;
            }
        }
        if(!found)
        {
            strcpy(globalLeaderboard[leaderboardCount].username, players[i].name);
            globalLeaderboard[leaderboardCount].score = players[i].score;
            globalLeaderboard[leaderboardCount].difficulty = players[i].difficulty;
            leaderboardCount++;
        }
    }
    // Sort leaderboard
    for(int i = 0; i < leaderboardCount - 1; i++)
    {
        for(int j = i + 1; j < leaderboardCount; j++)
        {
            if(globalLeaderboard[j].score > globalLeaderboard[i].score)
            {
                LeaderboardEntry temp = globalLeaderboard[i];
                globalLeaderboard[i] = globalLeaderboard[j];
                globalLeaderboard[j] = temp;
            }
        }
    }
    // Keep only top 10
    if(leaderboardCount > 10) leaderboardCount = 10;
}
void saveGlobalLeaderboard()
{
    FILE* file = fopen(LEADERBOARD_FILE, "wb");
    if(file)
    {
        fwrite(&leaderboardCount, sizeof(int), 1, file);
        fwrite(globalLeaderboard, sizeof(LeaderboardEntry), leaderboardCount, file);
        fclose(file);
    }
}
int updateLeaderboardWithCurrentPlayer(const char* username, int score, int diff)
{
    int inserted = 0;
    // Check if this score should be in top 10
    if(leaderboardCount < 10 || score > globalLeaderboard[leaderboardCount - 1].score)
    {
        // Find position to insert
        int pos = -1;
        for(int i = 0; i < leaderboardCount; i++)
        {
            if(score > globalLeaderboard[i].score)
            {
                pos = i;
                break;
            }
        }
        if(pos == -1 && leaderboardCount < 10)
        {
            pos = leaderboardCount;
        }
        if(pos != -1)
        {
            // Shift down
            if(leaderboardCount < 10) leaderboardCount++;
            for(int i = leaderboardCount - 1; i > pos; i--)
            {
                globalLeaderboard[i] = globalLeaderboard[i - 1];
            }
            // Insert
            strcpy(globalLeaderboard[pos].username, username);
            globalLeaderboard[pos].score = score;
            globalLeaderboard[pos].difficulty = diff;
            // Keep only top 10
            if(leaderboardCount > 10) leaderboardCount = 10;
            inserted = 1;
            saveGlobalLeaderboard();
        }
    }
    return inserted;
}
void saveUserHighscore(const char* username, int score, int diff)
{
    FILE* file = fopen(HIGHSCORES_FILE, "r+b");
    if(file)
    {
        int count = 0;
        User tempUsers[1000];
        // Read existing highscores
        if(fread(&count, sizeof(int), 1, file) == 1)
        {
            if(count > 1000) count = 1000;
            fread(tempUsers, sizeof(User), count, file);
        }

        // Update or add highscore for user
        int found = 0;
        for(int i = 0; i < count; i++)
        {
            if(strcmp(tempUsers[i].username, username) == 0)
            {
                if(diff == 1 && score > tempUsers[i].highScoreEasy)
                    tempUsers[i].highScoreEasy = score;
                else if(diff == 2 && score > tempUsers[i].highScoreMedium)
                    tempUsers[i].highScoreMedium = score;
                else if(diff == 3 && score > tempUsers[i].highScoreHard)
                    tempUsers[i].highScoreHard = score;
                // Update overall high score (best across all difficulties)
                int bestScore = tempUsers[i].highScoreEasy;
                if(tempUsers[i].highScoreMedium > bestScore) bestScore = tempUsers[i].highScoreMedium;
                if(tempUsers[i].highScoreHard > bestScore) bestScore = tempUsers[i].highScoreHard;
                tempUsers[i].highScore = bestScore;
                found = 1;
                break;
            }
        }

        if(!found && count < 1000)
        {
            strcpy(tempUsers[count].username, username);
            tempUsers[count].highScoreEasy = (diff == 1) ? score : 0;
            tempUsers[count].highScoreMedium = (diff == 2) ? score : 0;
            tempUsers[count].highScoreHard = (diff == 3) ? score : 0;
            // Overall high score is the best across all difficulties
            int bestScore = tempUsers[count].highScoreEasy;
            if(tempUsers[count].highScoreMedium > bestScore) bestScore = tempUsers[count].highScoreMedium;
            if(tempUsers[count].highScoreHard > bestScore) bestScore = tempUsers[count].highScoreHard;
            tempUsers[count].highScore = bestScore;
            count++;
        }
        // Write back
        rewind(file);
        fwrite(&count, sizeof(int), 1, file);
        fwrite(tempUsers, sizeof(User), count, file);
        fclose(file);
    }
    else
    {
        // Create new file
        file = fopen(HIGHSCORES_FILE, "wb");
        if(file)
        {
            int count = 1;
            User tempUser;
            strcpy(tempUser.username, username);
            tempUser.highScoreEasy = (diff == 1) ? score : 0;
            tempUser.highScoreMedium = (diff == 2) ? score : 0;
            tempUser.highScoreHard = (diff == 3) ? score : 0;
            // Overall high score is the best across all difficulties
            int bestScore = tempUser.highScoreEasy;
            if(tempUser.highScoreMedium > bestScore) bestScore = tempUser.highScoreMedium;
            if(tempUser.highScoreHard > bestScore) bestScore = tempUser.highScoreHard;
            tempUser.highScore = bestScore;
            fwrite(&count, sizeof(int), 1, file);
            fwrite(&tempUser, sizeof(User), 1, file);
            fclose(file);
        }
    }
}
int loadUserHighscore(const char* username, int diff)
{
    FILE* file = fopen(HIGHSCORES_FILE, "rb");
    if(file)
    {
        int count = 0;
        if(fread(&count, sizeof(int), 1, file) == 1)
        {
            if(count > 1000) count = 1000;
            User tempUsers[1000];
            fread(tempUsers, sizeof(User), count, file);

            for(int i = 0; i < count; i++)
            {
                if(strcmp(tempUsers[i].username, username) == 0)
                {
                    fclose(file);
                    // Return high score for the specific difficulty
                    if(diff == 1) return tempUsers[i].highScoreEasy;
                    else if(diff == 2) return tempUsers[i].highScoreMedium;
                    else if(diff == 3) return tempUsers[i].highScoreHard;
                    else return 0;
                }
            }
        }
        fclose(file);
    }
    return 0;
}
void loadUserDifficultyHighscores(const char* username, int* easy, int* medium, int* hard)
{
    *easy = 0;  // Default to 10 instead of 0
    *medium = 0;
    *hard = 0;
    FILE* file = fopen(HIGHSCORES_FILE, "rb");
    if(file)
    {
        int count = 0;
        if(fread(&count, sizeof(int), 1, file) == 1)
        {
            if(count > 1000) count = 1000;
            User tempUsers[1000];
            // Initialize all to 0 first to avoid garbage values
            for(int i = 0; i < 1000; i++)
            {
                tempUsers[i].highScoreEasy = 0;
                tempUsers[i].highScoreMedium = 0;
                tempUsers[i].highScoreHard = 0;
            }
            fread(tempUsers, sizeof(User), count, file);
            for(int i = 0; i < count; i++)
            {
                if(strcmp(tempUsers[i].username, username) == 0)
                {
                    *easy = tempUsers[i].highScoreEasy;
                    *medium = tempUsers[i].highScoreMedium;
                    *hard = tempUsers[i].highScoreHard;
                    // Ensure values are valid (non-negative)
                    if(*easy < 0) *easy = 0;
                    if(*medium < 0) *medium = 0;
                    if(*hard < 0) *hard = 0;
                    fclose(file);
                    return;
                }
            }
        }
        fclose(file);
    }
}
User* findUser(const char* username)
{
    for(int i = 0; i < userCount; i++)
    {
        if(strcmp(users[i].username, username) == 0)
        {
            return &users[i];
        }
    }
    return NULL;
}
int signup()
{
    char username[50], password[50], confirmPassword[50];
    while(true)
    {
        system("cls");
        gotoxy(31,10);
        printf("================== SIGN UP ======================");
        gotoxy(31,12);
        printf("Username: ");
        gotoxy(31,13);
        printf("Password: ");
        gotoxy(31,14);
        printf("Confirm Password: ");
        gotoxy(42,12);
        scanf("%s", username);
        gotoxy(42,13);
        scanf("%s", password);
        gotoxy(51,14);
        scanf("%s", confirmPassword);
        // Validation
        if(strlen(username) == 0)
        {
            gotoxy(28,16);
            printf("Username cannot be empty!");
            delay(1500);
            continue;
        }
        if(strlen(password) == 0)
        {
            gotoxy(28,16);
            printf("Password cannot be empty!");
            delay(1500);
            continue;
        }
        if(strcmp(password, confirmPassword) != 0)
        {
            gotoxy(28,16);
            printf("Passwords do not match!");
            delay(1500);
            continue;
        }
        // Check if username already exists
        if(findUser(username) != NULL)
        {
            gotoxy(28,16);
            printf("Username already exists! Please choose another.");
            delay(2000);
            continue;
        }
        // Create new user
        if(userCount < 1000)
        {
            strcpy(users[userCount].username, username);
            strcpy(users[userCount].password, password);
            users[userCount].highScore = 0;
            users[userCount].highScoreEasy = 0;
            users[userCount].highScoreMedium = 0;
            users[userCount].highScoreHard = 0;
            userCount++;
            saveUsers();
            gotoxy(33,16);
            printf("Account created successfully!");
            delay(1500);
            return 1;
        }
        else
        {
            gotoxy(28,16);
            printf("Maximum users reached!");
            delay(1500);
            return 0;
        }
    }
}
int login()
{
    char username[50], password[50];
    int attempts = 0;
    while(attempts < 3)
    {
        system("cls");
        gotoxy(31,12);
        printf("================== LOGIN SYSTEM ======================");
        gotoxy(31,14);
        printf("Username: ");
        gotoxy(31,15);
        printf("Password: ");
        gotoxy(42,14);
        scanf("%s", username);
        gotoxy(42,15);
        scanf("%s", password);
        User* user = findUser(username);
        if(user != NULL && strcmp(user->password, password) == 0)
        {
            currentUser = user;
            strcpy(currentPlayerName, username);
            highScore = loadUserHighscore(username, difficulty);
            // Load difficulty-specific highscores
            int easy, medium, hard;
            loadUserDifficultyHighscores(username, &easy, &medium, &hard);
            user->highScoreEasy = easy;
            user->highScoreMedium = medium;
            user->highScoreHard = hard;
            // Set highScore based on current difficulty level
            if(difficulty == 1)
                highScore = user->highScoreEasy;
            else if(difficulty == 2)
                highScore = user->highScoreMedium;
            else if(difficulty == 3)
                highScore = user->highScoreHard;
            else
                highScore = 0;
            gotoxy(33,17);
            printf("Login Successful!");
            gotoxy(28,18);
            printf("Welcome, %s! High Score: %d", username, highScore);
            delay(2000);
            return 1;
        }
        else
        {
            attempts++;
            gotoxy(28,17);
            printf("Invalid credentials! Attempts: %d/3", attempts);
            delay(1500);
            if(attempts >= 3)
            {
                gotoxy(26,18);
                printf("Too many failed attempts. Exiting...");
                delay(2000);
                return 0;
            }
        }
    }
    return 0;
}
int showLoginMenu()
{
    int choice;
    while(true)
    {
        system("cls");
        gotoxy(31,10);
        printf("============== WELCOME TO DINO GAME ==============");
        gotoxy(31,12);
        printf("1. Login");
        gotoxy(31,13);
        printf("2. Sign Up");
        gotoxy(31,14);
        printf("3. Exit");
        gotoxy(31,16);
        printf("==================================================");
        gotoxy(31,18);
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch(choice)
        {
        case 1:
            if(login())
                return 1;
            break;
        case 2:
            signup();
            break;
        case 3:
            return 0;
        default:
            gotoxy(28,20);
            printf("Invalid choice! Press any key...");
            getch();
            break;
        }
    }
}
void showMenu()
{
    system("cls");
    gotoxy(25,6);
    printf("Logged in as: %s", currentPlayerName);
    gotoxy(25,7);
    printf("High Score: %d", highScore);
    gotoxy(25,8);
    printf("==================== DINO GAME MENU ====================");
    gotoxy(25,10);
    printf("1. Start Game");
    gotoxy(25,11);
    printf("2. Difficulty Level");
    gotoxy(25,12);
    printf("3. Instructions");
    gotoxy(25,13);
    printf("4. Score List");
    gotoxy(25,14);
    printf("5. Settings");
    gotoxy(25,15);
    printf("6. Logout");
    gotoxy(25,16);
    printf("=========================================================");
    gotoxy(25,18);
    printf("Enter your choice: ");
}
void showInstructions()
{
    system("cls");
    gotoxy(25,8);
    printf("==================== INSTRUCTIONS ====================");
    gotoxy(20,10);
    printf("Press SPACE to make the dinosaur jump");
    gotoxy(20,11);
    printf("Avoid the obstacles coming from the right");
    gotoxy(20,12);
    printf("Your score increases as you pass obstacles");
    gotoxy(20,13);
    printf("Game speed increases as your score increases");
    gotoxy(20,14);
    printf("Press X during game to return to menu");
    gotoxy(20,15);
    printf("Press L during game to logout");
    gotoxy(25,17);
    printf("=====================================================");
    gotoxy(25,19);
    printf("Press any key to return to menu...");
    getch();
}
void showDifficultyLevel()
{
    system("cls");
    gotoxy(25,8);
    printf("==================== DIFFICULTY LEVEL ====================");
    gotoxy(25,10);
    printf("Current Difficulty: ");
    if(difficulty==1) printf("Easy");
    else if(difficulty==2) printf("Medium");
    else if(difficulty==3) printf("Hard");
    gotoxy(25,12);
    printf("1. Easy");
    gotoxy(25,13);
    printf("2. Medium");
    gotoxy(25,14);
    printf("3. Hard");
    gotoxy(25,16);
    printf("=========================================================");
    gotoxy(25,18);
    printf("Select difficulty (1-3): ");
    int choice;
    scanf("%d", &choice);
    if(choice>=1 && choice<=3)
    {
        difficulty = choice;
        if(difficulty==1) speed=40;
        else if(difficulty==2) speed=30;
        else if(difficulty==3) speed=20;
        // Update high score for the new difficulty level
        if(strlen(currentPlayerName) > 0)
        {
            highScore = loadUserHighscore(currentPlayerName, difficulty);
        }
        else
        {
            highScore = 0;
        }
        gotoxy(25,20);
        printf("Difficulty changed successfully!");
        delay(1500);
    }
}
void showScoreList()
{
    system("cls");
    gotoxy(20,2);
    printf("==================== SCORE LIST ====================");
    gotoxy(20,3);
    printf("Player Name          Difficulty    Score");
    gotoxy(20,4);
    printf("----------------------------------------------------");
    if(playerCount == 0)
    {
        gotoxy(20,6);
        printf("No players yet. Play the game to add your score!");
    }
    else
    {
        Player sortedPlayers[100];
        for(int i = 0; i < playerCount; i++)
        {
            sortedPlayers[i] = players[i];
        }
        for(int i = 0; i < playerCount - 1; i++)
        {
            for(int j = i + 1; j < playerCount; j++)
            {
                if(sortedPlayers[j].score > sortedPlayers[i].score)
                {
                    Player temp = sortedPlayers[i];
                    sortedPlayers[i] = sortedPlayers[j];
                    sortedPlayers[j] = temp;
                }
            }
        }
        int startY = 5;
        int maxDisplay = 20;
        int displayCount = (playerCount < maxDisplay) ? playerCount : maxDisplay;
        for(int i = 0; i < displayCount; i++)
        {
            gotoxy(20, startY + i);
            char diffStr[10];
            if(sortedPlayers[i].difficulty == 1) strcpy(diffStr, "Easy");
            else if(sortedPlayers[i].difficulty == 2) strcpy(diffStr, "Medium");
            else if(sortedPlayers[i].difficulty == 3) strcpy(diffStr, "Hard");
            else strcpy(diffStr, "Unknown");

            printf("%-20s %-12s %d", sortedPlayers[i].name, diffStr, sortedPlayers[i].score);
        }
        if(playerCount > maxDisplay)
        {
            gotoxy(20, startY + maxDisplay);
            printf("... and %d more players", playerCount - maxDisplay);
        }
    }
    gotoxy(20,27);
    printf("====================================================");
    gotoxy(20,28);

    printf("Press any key to return to menu...");
    getch();
}
void showSettings()
{
    system("cls");
    gotoxy(25,8);
    printf("==================== SETTINGS ====================");
    gotoxy(25,10);
    printf("Current User: %s", currentPlayerName);
    gotoxy(25,11);
    printf("Current Settings:");
    gotoxy(25,12);
    printf("Difficulty: ");
    if(difficulty==1) printf("Easy");
    else if(difficulty==2) printf("Medium");
    else if(difficulty==3) printf("Hard");
    gotoxy(25,13);
    printf("Game Speed: %d", speed);
    gotoxy(25,14);
    printf("High Score: %d", highScore);
    gotoxy(25,16);
    printf("==================================================");
    gotoxy(25,18);
    printf("Press any key to return to menu...");
    getch();
}
void showGameSideBar(int currentScore, bool forceRefresh = false)
{
    static int lastScore = -1;
    static int lastDifficulty = -1;
    static int lastHighScore = -1;
    static bool firstDraw = true;
    // Panel positioned in top-right corner (console is 82 cols wide)
    // Panel starts at col 58, leaving game area 0-57 on left
    const int PANEL_X = 58;
    // Force refresh if requested (e.g., after difficulty change or score save)
    if(forceRefresh)
    {
        firstDraw = true;
        lastScore = -1;
        lastDifficulty = -1;
        lastHighScore = -1;
        // Reload high scores when forcing refresh to ensure we have latest values
        if(strlen(currentPlayerName) > 0)
        {
            int easy = 0, medium = 0, hard = 0;
            loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
            if(currentUser != NULL)
            {
                currentUser->highScoreEasy = easy;
                currentUser->highScoreMedium = medium;
                currentUser->highScoreHard = hard;
            }
        }
    }
    // Detect game restart (score reset to 0 after a game) or new game start
    if(currentScore == 0 && (lastScore > 0 || forceRefresh))
    {
        firstDraw = true; // Reset for new game
        // Reload high scores after game ends or when starting new game to show updated values
        if(strlen(currentPlayerName) > 0)
        {
            int easy = 0, medium = 0, hard = 0;
            loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
            if(currentUser != NULL)
            {
                currentUser->highScoreEasy = easy;
                currentUser->highScoreMedium = medium;
                currentUser->highScoreHard = hard;
            }
        }
    }
    int highScore = 0;  // High score for current difficulty level
    int easy = 0, medium = 0, hard = 0;
    if(strlen(currentPlayerName) > 0)
    {
        loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
        // Update currentUser if it exists
        if(currentUser != NULL)
        {
            currentUser->highScoreEasy = easy;
            currentUser->highScoreMedium = medium;
            currentUser->highScoreHard = hard;
        }
// Get high score for current difficulty level
        if(difficulty == 1)
            highScore = easy;
        else if(difficulty == 2)
            highScore = medium;
        else if(difficulty == 3)
            highScore = hard;
        else
            highScore = 0;
    }
    else
    {
        // No player name, so no high score
        highScore = 0;
    }
    // Ensure high score is not negative (should be 0 if not set)
    if(highScore < 0) highScore = 0;
    // Always redraw if force refresh, first draw, or if values changed
    // Force refresh ensures we always update when starting a new game
    if(forceRefresh || firstDraw || currentScore != lastScore || difficulty != lastDifficulty ||
            highScore != lastHighScore)
    {
        int yPos = 1;
        // Draw border and header (only if first draw)
        if(firstDraw)
        {
            gotoxy(PANEL_X, yPos++);
            printf("+======================+");
            gotoxy(PANEL_X, yPos++);
            printf("|    PLAYER INFO       |");
            gotoxy(PANEL_X, yPos++);
            printf("+======================+");
            yPos++;
        }
        // Always display player name
        yPos = 5;
        char displayName[20];
        if(strlen(currentPlayerName) > 0)
        {
            strncpy(displayName, currentPlayerName, 19);
            displayName[19] = '\0';

        }
        else
        {
            strcpy(displayName, "Guest");
        }
        gotoxy(PANEL_X, yPos++);
        printf("| Player: %-13s |", displayName);
        // Update Score at fixed position (yPos 6)
        gotoxy(PANEL_X, yPos);
        printf("| Score:  %-13d |", currentScore);
        lastScore = currentScore;
        yPos++;
        // Update Difficulty at fixed position (yPos 7)
        gotoxy(PANEL_X, yPos);
        printf("| Diff:   ");
        if(difficulty == 1) printf("Easy          |");
        else if(difficulty == 2) printf("Medium        |");
        else if(difficulty == 3) printf("Hard          |");
        else printf("Unknown       |");
        lastDifficulty = difficulty;
        yPos++;
        // Draw separator at fixed position (yPos 8)
        if(firstDraw)
        {
            yPos++;
            gotoxy(PANEL_X, yPos++);
            printf("+----------------------+");

        }
        else
        {
            yPos = 9; // Skip to high score position
        }
        // Always display High Score at fixed position (yPos 9)
        // This shows the best high score across all difficulties
        // It automatically saves and updates when player scores higher
        if(firstDraw && yPos < 9) yPos = 9;
        if(!firstDraw) yPos = 9;
        // Clear the line first to prevent duplicate display
        gotoxy(PANEL_X, yPos);
        printf("|                      |");  // Clear with spaces
        // Display High Score - best score ever achieved (saves and updates automatically)
        gotoxy(PANEL_X, yPos);
        printf("| High:   %-13d |", highScore);
        // Update tracking variable
        lastHighScore = highScore;
        // Draw bottom border (only if first draw)
        if(firstDraw)
        {
            yPos++;
            gotoxy(PANEL_X, yPos);
            printf("+======================+");
        }
        firstDraw = false;
    }
}
int getPlayerName()
{
    // Player name is already set from login, just verify
    if(strlen(currentPlayerName) == 0)
    {
        system("cls");
        gotoxy(30,12);
        printf("No user logged in!");
        delay(1500);
        return 0;
    }
    return 1;
}
void savePlayerScore(int score)
{
    if(strlen(currentPlayerName) == 0 || score <= 0)
        return;
    // Always append this run to a human-readable CSV file in the project folder
    appendScoreToCSV(currentPlayerName, score, difficulty);
    // Save to players array for score list
    if(playerCount < 100)
    {
        strcpy(players[playerCount].name, currentPlayerName);
        players[playerCount].score = score;
        players[playerCount].difficulty = difficulty;
        playerCount++;
    }
    if(currentUser != NULL)
    {
        // Update difficulty-specific highscore only if score is higher
        bool updated = false;
        if(difficulty == 1 && score > currentUser->highScoreEasy)
        {
            currentUser->highScoreEasy = score;

            highScore = score;
            updated = true;
        }
        else if(difficulty == 2 && score > currentUser->highScoreMedium)
        {
            currentUser->highScoreMedium = score;
            highScore=score;
            updated=true;
        }
        else if(difficulty == 3 && score > currentUser->highScoreHard)
        {
            currentUser->highScoreHard = score;
            highScore = score;
            updated = true;
        }
        // Update overall high score (best across all difficulties)
        int bestScore = currentUser->highScoreEasy;
        if(currentUser->highScoreMedium > bestScore) bestScore = currentUser->highScoreMedium;
        if(currentUser->highScoreHard > bestScore) bestScore = currentUser->highScoreHard;
        currentUser->highScore = bestScore;
        // Only save if we updated a high score
        if(updated)
        {
            saveUserHighscore(currentUser->username, score, difficulty);
        }
        // Reload high scores from file to ensure consistency
        int easy = 0, medium = 0, hard = 0;
        loadUserDifficultyHighscores(currentUser->username, &easy, &medium, &hard);
        currentUser->highScoreEasy = easy;
        currentUser->highScoreMedium = medium;
        currentUser->highScoreHard = hard;
        // Update highScore variable for current difficulty
        if(difficulty == 1)
            highScore = easy;
        else if(difficulty == 2)
            highScore = medium;
        else if(difficulty == 3)
            highScore = hard;
        saveUsers(); // Update user file with new highscore
        // Update global leaderboard
        updateLeaderboardWithCurrentPlayer(currentUser->username, score, difficulty);
        loadGlobalLeaderboard();
    }
}
void getup()
{
    system("cls");
    gotoxy(2,2);
    printf("Press X to Menu, Space to Jump, L to Logout");
    gotoxy(1,25);
    for(int x=0; x<57; x++)  // Stop before panel area (panel starts at col 58)
        printf("ß");
    // Initialize and draw the player info panel with force refresh to ensure high scores are loaded
    // Reload high scores for current user to ensure they're up to date
    // This is critical when starting a new game after finishing one, to show the updated high score
    if(strlen(currentPlayerName) > 0)
    {
        int easy = 0, medium = 0, hard = 0;
        loadUserDifficultyHighscores(currentPlayerName, &easy, &medium, &hard);
        // Update currentUser if it exists
        if(currentUser != NULL)
        {
            currentUser->highScoreEasy = easy;
            currentUser->highScoreMedium = medium;
            currentUser->highScoreHard = hard;
        }
    }
    // Force refresh to ensure high score is displayed correctly when starting new game
    showGameSideBar(0, true);
}
void ds(int jump=0)
{
    static int a=1;
    if(jump==0)
        t=0;
    else if(jump==2)
        t--;
    else t++;
    gotoxy(2,15-t);
    printf("                 ");
    gotoxy(2,16-t);
    printf("         ÜÛßÛÛÛÛÜ");
    gotoxy(2,17-t);
    printf("         ÛÛÛÛÛÛÛÛ");
    gotoxy(2,18-t);
    printf("         ÛÛÛÛÛßßß");
    gotoxy(2,19-t);
    printf(" Û      ÜÛÛÛÛßßß ");
    gotoxy(2,20-t);
    printf(" ÛÛÜ  ÜÛÛÛÛÛÛÜÜÜ ");
    gotoxy(2,21-t);
    printf(" ßÛÛÛÛÛÛÛÛÛÛÛ  ß ");
    gotoxy(2,22-t);
    printf("   ßÛÛÛÛÛÛÛß     ");
    gotoxy(2,23-t);
    if(jump==1 || jump==2)
    {
        printf("    ÛÛß ßÛ       ");
        gotoxy(2,24-t);
        printf("    ÛÜ   ÛÜ      ");
    }
    else if(a==1)
    {
        printf("    ßÛÛß  ßßß    ");
        gotoxy(2,24-t);
        printf("      ÛÜ         ");
        a=2;
    }
    else if(a==2)
    {
        printf("     ßÛÜ ßÛ      ");
        gotoxy(2,24-t);
        printf("          ÛÜ     ");
        a=1;
    }
    gotoxy(2,25-t);
    if(jump!=0)
    {
        printf("                ");
    }
    else
    {
        // printf("ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß");
        // Draw ground line, stopping before panel area (panel starts at col 58)
        for(int x=0; x<55; x++)  // 55 chars from col 2 = stops at col 57
            printf("ßß");
    }
    delay(speed);
}
int obj(int reset=0, int *currentScore=NULL)

{
    static int x=0,scr=0;
    if(reset==1)
    {
        x=0;
        scr=0;
        if(currentScore != NULL)
            *currentScore = 0;
        return 0;
    }
    if(x==56 && t<4)
    {
        savePlayerScore(scr);
        if(currentScore != NULL)
            *currentScore = scr;
        // Force refresh panel to show updated high score after game ends
        showGameSideBar(scr, true);
        scr=0;
        x=0;
        if(difficulty==1) speed=40;
        else if(difficulty==2) speed=30;
        else if(difficulty==3) speed=20;
        gotoxy(36,8);
        printf("Game Over");
        getch();
        gotoxy(36,8);
        printf("         ");
        return 1;
    }
    gotoxy(74-x,20);
    printf("Û    Û ");
    gotoxy(74-x,21);
    printf("Û    Û ");
    gotoxy(74-x,22);
    printf("ÛÜÜÜÜÛ ");
    gotoxy(74-x,23);
    printf("  Û    ");
    gotoxy(74-x,24);
    printf("  Û  " );
    x++;
    if(x==73)
    {
        x=0;
        scr++;
        if(speed>20)
            speed--;
    }
    if(currentScore != NULL)
        *currentScore = scr;
    return 0;
}
int startGame()
{

    if(!getPlayerName())
    {
        return 0;
    }
    char ch;
    int i;
    int gameOver = 0;
    int currentScore = 0;
    t=0;
    obj(1, &currentScore);
    if(difficulty==1) speed=40;
    else if(difficulty==2) speed=30;
    else if(difficulty==3) speed=20;
    getup();
    while(true)
    {
        while(!kbhit())
        {
            ds();
            gameOver = obj(0, &currentScore);
            showGameSideBar(currentScore);
            if(gameOver)
                return 0;
        }
        ch=getch();
        if(ch==' ')
        {
            for(i=0; i<10; i++)
            {
                ds(1);
                gameOver = obj(0, &currentScore);
                showGameSideBar(currentScore);
                if(gameOver)
                {
                    savePlayerScore(currentScore);
                    return 0;
                }
            }
            for(i=0; i<10; i++)
            {
                ds(2);
                gameOver = obj(0, &currentScore);
                showGameSideBar(currentScore);
                if(gameOver)
                {
                    savePlayerScore(currentScore);
                    return 0;
                }
            }
        }
        else if (ch=='x' || ch=='X')
        {
            savePlayerScore(currentScore);
            return 0;
        }
        else if (ch=='l' || ch=='L')
        {
            savePlayerScore(currentScore);
            system("cls");
            gotoxy(28,14);
            printf("Logged out successfully!");
            delay(1500);
            return 1;
        }
    }
}
void appendScoreToCSV(const char* username, int score, int diff)
{
    if(username == NULL || username[0] == '\0' || score <= 0)
        return;
    FILE* file = fopen(SCORE_CSV_FILE, "a");
    if(!file)
        return;
    const char* diffStr = "Unknown";
    if(diff == 1) diffStr = "Easy";
    else if(diff == 2) diffStr = "Medium";
    else if(diff == 3) diffStr = "Hard";
    // Optional timestamp column so you know when the score was recorded
    time_t now = time(NULL);
    struct tm* tinfo = localtime(&now);
    char timebuf[32] = "";
    if(tinfo != NULL)
    {
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tinfo);
    }
    // CSV format: username,difficulty,score,timestamp
    fprintf(file, "%s-%s-%d-%s\n", username, diffStr, score,
            (timebuf[0] != '\0') ? timebuf : "");
    fclose(file);
}
int main()
{
    system("mode con: lines=29 cols=82");
    int choice;
    int logoutFlag = 0;

    // Load users from file on startup
    loadUsers();
    while(true)
    {
        // Show login/signup menu
        if(!showLoginMenu())
        {
            return 0;
        }
        logoutFlag = 0;
        while(!logoutFlag)
        {
            showMenu();
            fflush(stdin);
            scanf("%d", &choice);

            switch(choice)
            {
            case 1:
                if(startGame() == 1)
                    logoutFlag = 1;
                break;
            case 2:
                showDifficultyLevel();
                break;
            case 3:
                showInstructions();
                break;
            case 4:
                showScoreList();
                break;
            case 5:
                showSettings();
                break;
            case 6:
                // Logout
                currentUser = NULL;
                strcpy(currentPlayerName, "");
                highScore = 0;
                system("cls");
                gotoxy(30,14);
                // printf("Thank you for playing!");
                printf("Logged out successfully!");
                delay(1500);
                logoutFlag = 1;
                break;
            default:
                gotoxy(25,20);
                printf("Invalid choice! Press any key...");
                getch();
                break;
            }
        }
    }
    return 0;
}
*/
