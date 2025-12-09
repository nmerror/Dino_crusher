# 🦖 Dino Crusher - Console Game

A console-based Dino running game written in C/C++ for Windows. This game features user authentication, difficulty levels, persistent high scores, global leaderboard, and CSV score export.

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

## 📋 Table of Contents

- [Features](#features)
- [Screenshots](#screenshots)
- [Installation](#installation)
- [How to Play](#how-to-play)
- [Game Controls](#game-controls)
- [Project Structure](#project-structure)
- [File Descriptions](#file-descriptions)
- [Technical Details](#technical-details)
- [Contributing](#contributing)
- [License](#license)

## ✨ Features

- **User Management System**
  - User registration and login
  - Support for up to 1000 users
  - Secure password authentication (3-attempt limit)

- **Game Features**
  - Three difficulty levels: Easy, Medium, Hard
  - Real-time score tracking
  - Side panel showing player info, score, and high score
  - ASCII art graphics for dinosaur and obstacles
  - Collision detection and game over handling

- **Score Management**
  - Per-difficulty high scores (Easy, Medium, Hard)
  - Global top-10 leaderboard
  - Session score list
  - CSV export for all game sessions

- **Data Persistence**
  - Binary file storage for users (`users.dat`)
  - Binary file storage for high scores (`highscores.dat`)
  - Binary file storage for leaderboard (`leaderboard.dat`)
  - CSV file export (`scores.csv`) for easy viewing

## 🎮 How to Play

1. **Start the Game**
   - Run the compiled executable
   - You'll see the welcome screen

2. **Create an Account**
   - Select option `2. Sign Up`
   - Enter a username and password
   - Confirm your password

3. **Login**
   - Select option `1. Login`
   - Enter your username and password

4. **Select Difficulty**
   - From the main menu, choose `2. Difficulty Level`
   - Select Easy, Medium, or Hard

5. **Start Playing**
   - Choose `1. Start Game` from the main menu
   - Press **SPACE** to make the dinosaur jump
   - Avoid obstacles to increase your score

## 🎯 Game Controls

| Key | Action |
|-----|--------|
| `SPACE` | Make the dinosaur jump |
| `X` | Return to main menu |
| `L` | Logout |

## 📁 Project Structure

```
Dino_Crusher/
│
├── main.cpp                 # Main source code file
├── README.md                # This file
├── Dino_Crusher_Project_Report.md  # Project documentation
│
├── users.dat               # User data (auto-generated)
├── highscores.dat          # High scores (auto-generated)
├── leaderboard.dat         # Leaderboard data (auto-generated)
└── scores.csv              # CSV export of all game sessions (auto-generated)
```

## 📄 File Descriptions

### Source Files

- **`main.cpp`**: Complete game implementation including:
  - User management (signup, login)
  - Game logic (dinosaur, obstacles, collision)
  - Menu system
  - File I/O operations
  - CSV export functionality

### Data Files (Auto-generated)

- **`users.dat`**: Binary file storing user accounts and passwords
- **`highscores.dat`**: Binary file storing difficulty-specific high scores
- **`leaderboard.dat`**: Binary file storing global top-10 leaderboard
- **`scores.csv`**: Text file with all game sessions (username, difficulty, score, timestamp)

## 🔧 Installation

### Prerequisites

- Windows Operating System
- C/C++ Compiler (GCC, MinGW, or Visual Studio)
- Console that supports Windows API

### Compilation

1. **Using GCC/MinGW:**
   ```bash
   g++ -o DinoCrusher main.cpp
   ```

2. **Using Visual Studio:**
   - Open Visual Studio
   - Create a new C++ project
   - Add `main.cpp` to the project
   - Build and run

3. **Using Code::Blocks:**
   - Create a new console application project
   - Add `main.cpp` to the project
   - Build and run

### Running the Game

```bash
./DinoCrusher.exe
```

or simply double-click the executable file.

## 💻 Technical Details

### Technologies Used

- **Language**: C/C++
- **Libraries**: 
  - `stdio.h` - Standard input/output
  - `conio.h` - Console input/output
  - `windows.h` - Windows API for console manipulation
  - `time.h` - Time functions for timestamps
  - `string.h` - String manipulation

### Key Functions

- `gotoxy(x, y)` - Position cursor at specific coordinates
- `delay(mseconds)` - Control game speed
- `ds(jump)` - Draw dinosaur with animation
- `obj()` - Handle obstacles and collision
- `savePlayerScore()` - Save scores to files
- `appendScoreToCSV()` - Export to CSV file

### Console Specifications

- **Window Size**: 82 columns × 29 lines
- **Game Area**: Left side (columns 0-57)
- **Info Panel**: Right side (columns 58-82)

## 📊 Features Breakdown

### User System
- Registration with validation
- Login with 3-attempt limit
- Password confirmation
- Username uniqueness check

### Game Mechanics
- Dinosaur jumping animation
- Obstacle movement (right to left)
- Collision detection
- Score increment on obstacle pass
- Speed increase with score

### Difficulty Levels
- **Easy**: Initial speed = 40ms delay
- **Medium**: Initial speed = 30ms delay
- **Hard**: Initial speed = 20ms delay

### Data Management
- Binary file I/O for efficient storage
- CSV export for human-readable data
- Automatic file creation
- Data validation and error handling

## 🎨 Screenshots

*Add screenshots of your game here:*
- Welcome/Login Screen
- Main Menu
- Gameplay
- Score List
- Settings

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📝 License

This project is open source and available under the [MIT License](LICENSE).

## 👤 Author

**Your Name**
- GitHub: [@yourusername](https://github.com/yourusername)

## 🙏 Acknowledgments

- Inspired by the classic Chrome Dino game
- Reference code from [sh0aib-ahmed/dino_game](https://github.com/sh0aib-ahmed/dino_game.git)

## 📧 Contact

For questions or suggestions, please open an issue on GitHub.

---

⭐ If you like this project, please give it a star!

