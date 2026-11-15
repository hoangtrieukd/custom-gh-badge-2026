
#include <Arduino.h>
#include <SPI.h>
#include <string.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// =====================================================
// TFT CONFIGURATION
// =====================================================

#define TFT_CS 8
#define TFT_RST 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// =====================================================
// BUTTON CONFIGURATION
// =====================================================

#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7
#define BUTTON_SELECT 15

#define DEBOUNCE_MS 40

struct Button
{
    const char *name;
    uint8_t pin;
    bool lastReading;
    bool stableState;
    unsigned long lastChangeTime;
};

Button buttons[] = {
    {"UP", BUTTON_UP, HIGH, HIGH, 0},
    {"DOWN", BUTTON_DOWN, HIGH, HIGH, 0},
    {"LEFT", BUTTON_LEFT, HIGH, HIGH, 0},
    {"RIGHT", BUTTON_RIGHT, HIGH, HIGH, 0},
    {"SELECT", BUTTON_SELECT, HIGH, HIGH, 0}};

const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// =====================================================
// GAME CONFIGURATION
// =====================================================

#define BOARD_COLS 10
#define BOARD_ROWS 20
#define CELL_SIZE 10

#define BOARD_X 20
#define BOARD_Y 20

#define FALL_INTERVAL 600

uint8_t board[BOARD_ROWS][BOARD_COLS] = {0};

int score = 0;
bool gameOver = false;

unsigned long lastFallTime = 0;

// =====================================================
// COLORS
// =====================================================

const uint16_t pieceColors[] = {
    0,
    0x07FF, // I - Cyan
    0xFFE0, // O - Yellow
    0xF81F, // T - Magenta
    0xFD20, // L - Orange
    0x001F, // J - Blue
    0x07E0, // S - Green
    0xF800  // Z - Red
};

// =====================================================
// TETROMINO SHAPES
// =====================================================

const uint8_t shapes[7][4][4] = {

    // I
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},

    // O
    {
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},

    // T
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},

    // L
    {
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},

    // J
    {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},

    // S
    {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},

    // Z
    {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}}};

// =====================================================
// CURRENT PIECE
// =====================================================

struct Piece
{
    uint8_t type;
    int x;
    int y;
    uint8_t rotation;
};

Piece currentPiece;

// =====================================================
// PIECE ROTATION
// =====================================================

// Returns the value of a cell after rotation.
// Rotation is clockwise.

bool pieceCell(
    uint8_t type,
    int row,
    int col,
    uint8_t rotation)
{
    int r = row;
    int c = col;

    for (int i = 0; i < rotation; i++)
    {
        int oldR = r;
        r = 3 - c;
        c = oldR;
    }

    return shapes[type][r][c];
}

// =====================================================
// DISPLAY FUNCTIONS
// =====================================================

void drawCell(int x, int y, uint16_t color)
{

    int screenX = BOARD_X + x * CELL_SIZE;
    int screenY = BOARD_Y + y * CELL_SIZE;

    tft.fillRect(
        screenX + 1,
        screenY + 1,
        CELL_SIZE - 2,
        CELL_SIZE - 2,
        color);
}

void drawScore()
{

    tft.fillRect(
        145, 85,
        165, 35,
        ST77XX_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(150, 90);
    tft.print(score);
}

void drawBoard()
{

    // Clear board area
    tft.fillRect(
        BOARD_X,
        BOARD_Y,
        BOARD_COLS * CELL_SIZE,
        BOARD_ROWS * CELL_SIZE,
        ST77XX_BLACK);

    // Draw locked pieces
    for (int row = 0; row < BOARD_ROWS; row++)
    {

        for (int col = 0; col < BOARD_COLS; col++)
        {

            if (board[row][col] != 0)
            {

                drawCell(
                    col,
                    row,
                    pieceColors[board[row][col]]);
            }
        }
    }

    // Draw current falling piece
    if (!gameOver)
    {

        for (int row = 0; row < 4; row++)
        {

            for (int col = 0; col < 4; col++)
            {

                if (pieceCell(
                        currentPiece.type,
                        row,
                        col,
                        currentPiece.rotation))
                {

                    int x = currentPiece.x + col;
                    int y = currentPiece.y + row;

                    if (
                        x >= 0 &&
                        x < BOARD_COLS &&
                        y >= 0 &&
                        y < BOARD_ROWS)
                    {

                        drawCell(
                            x,
                            y,
                            pieceColors[currentPiece.type + 1]);
                    }
                }
            }
        }
    }

    // Draw grid
    for (int col = 0; col <= BOARD_COLS; col++)
    {

        int x = BOARD_X + col * CELL_SIZE;

        tft.drawLine(
            x, BOARD_Y,
            x, BOARD_Y + BOARD_ROWS * CELL_SIZE,
            0x4208);
    }

    for (int row = 0; row <= BOARD_ROWS; row++)
    {

        int y = BOARD_Y + row * CELL_SIZE;

        tft.drawLine(
            BOARD_X, y,
            BOARD_X + BOARD_COLS * CELL_SIZE, y,
            0x4208);
    }

    // Board border
    tft.drawRect(
        BOARD_X,
        BOARD_Y,
        BOARD_COLS * CELL_SIZE,
        BOARD_ROWS * CELL_SIZE,
        ST77XX_WHITE);
}

void drawGameOver()
{

    tft.fillRect(
        145, 155,
        165, 60,
        ST77XX_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(150, 160);
    tft.print("GAME OVER");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(150, 190);
    tft.print("SELECT: Restart");
}

void drawInterface()
{

    tft.fillScreen(ST77XX_BLACK);

    // Title
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(150, 25);
    tft.print("TETRIS");

    // Score label
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(150, 75);
    tft.print("SCORE");

    // Controls
    tft.setCursor(150, 135);
    tft.print("CONTROLS");

    tft.setCursor(150, 150);
    tft.print("LEFT/RIGHT: Move");

    tft.setCursor(150, 165);
    tft.print("DOWN: Faster");

    tft.setCursor(150, 180);
    tft.print("UP: Rotate");

    tft.setCursor(150, 195);
    tft.print("SELECT: Restart");

    drawScore();
}

// =====================================================
// COLLISION DETECTION
// =====================================================

bool canMove(int newX, int newY)
{

    for (int row = 0; row < 4; row++)
    {

        for (int col = 0; col < 4; col++)
        {

            if (!pieceCell(
                    currentPiece.type,
                    row,
                    col,
                    currentPiece.rotation))
            {
                continue;
            }

            int x = newX + col;
            int y = newY + row;

            // Left and right boundaries
            if (x < 0 || x >= BOARD_COLS)
            {
                return false;
            }

            // Bottom boundary
            if (y >= BOARD_ROWS)
            {
                return false;
            }

            // Existing locked pieces
            if (y >= 0 && board[y][x] != 0)
            {
                return false;
            }
        }
    }

    return true;
}

// =====================================================
// SPAWN PIECE
// =====================================================

void spawnPiece()
{

    currentPiece.type = random(0, 7);
    currentPiece.x = 3;
    currentPiece.y = 0;
    currentPiece.rotation = 0;

    if (!canMove(currentPiece.x, currentPiece.y))
    {

        gameOver = true;

        Serial.println("GAME OVER");
    }
}

// =====================================================
// ROTATION
// =====================================================

void rotatePiece()
{

    uint8_t oldRotation = currentPiece.rotation;

    currentPiece.rotation =
        (currentPiece.rotation + 1) % 4;

    if (!canMove(currentPiece.x, currentPiece.y))
    {

        // Cancel invalid rotation
        currentPiece.rotation = oldRotation;
    }
}

// =====================================================
// LINE CLEARING AND SCORING
// =====================================================

void clearCompletedLines()
{

    int linesCleared = 0;

    for (int row = BOARD_ROWS - 1; row >= 0; row--)
    {

        bool full = true;

        for (int col = 0; col < BOARD_COLS; col++)
        {

            if (board[row][col] == 0)
            {

                full = false;
                break;
            }
        }

        if (full)
        {

            linesCleared++;

            // Shift all rows above downward
            for (int r = row; r > 0; r--)
            {

                for (int c = 0; c < BOARD_COLS; c++)
                {

                    board[r][c] = board[r - 1][c];
                }
            }

            // Clear top row
            memset(board[0], 0, sizeof(board[0]));

            // Recheck this row after shifting
            row++;
        }
    }

    // Score based on lines cleared simultaneously
    switch (linesCleared)
    {

    case 1:
        score += 100;
        break;

    case 2:
        score += 300;
        break;

    case 3:
        score += 500;
        break;

    case 4:
        score += 800;
        break;
    }

    if (linesCleared > 0)
    {

        Serial.print("LINES CLEARED: ");
        Serial.println(linesCleared);

        Serial.print("SCORE: ");
        Serial.println(score);

        drawScore();
    }
}

// =====================================================
// LOCK PIECE
// =====================================================

void lockPiece()
{

    for (int row = 0; row < 4; row++)
    {

        for (int col = 0; col < 4; col++)
        {

            if (!pieceCell(
                    currentPiece.type,
                    row,
                    col,
                    currentPiece.rotation))
            {
                continue;
            }

            int x = currentPiece.x + col;
            int y = currentPiece.y + row;

            if (y < 0)
            {

                gameOver = true;
                return;
            }

            if (
                x >= 0 &&
                x < BOARD_COLS &&
                y < BOARD_ROWS)
            {

                board[y][x] = currentPiece.type + 1;
            }
        }
    }

    clearCompletedLines();

    spawnPiece();
}

// =====================================================
// MOVEMENT
// =====================================================

void moveDown()
{

    if (canMove(currentPiece.x, currentPiece.y + 1))
    {

        currentPiece.y++;
    }
    else
    {

        lockPiece();
    }
}

// =====================================================
// GAME RESTART
// =====================================================

void restartGame()
{

    memset(board, 0, sizeof(board));

    score = 0;
    gameOver = false;

    spawnPiece();

    lastFallTime = millis();

    drawInterface();
    drawBoard();

    if (gameOver)
    {
        drawGameOver();
    }

    Serial.println("GAME RESTARTED");
}

// =====================================================
// BUTTON EVENTS
// =====================================================

void onButtonPressed(const char *name)
{

    Serial.print("BUTTON PRESSED: ");
    Serial.println(name);

    // SELECT restarts the game
    if (strcmp(name, "SELECT") == 0)
    {

        restartGame();
        return;
    }

    if (gameOver)
    {
        return;
    }

    // LEFT
    if (strcmp(name, "LEFT") == 0)
    {

        if (canMove(currentPiece.x - 1, currentPiece.y))
        {

            currentPiece.x--;
        }
    }

    // RIGHT
    else if (strcmp(name, "RIGHT") == 0)
    {

        if (canMove(currentPiece.x + 1, currentPiece.y))
        {

            currentPiece.x++;
        }
    }

    // DOWN
    else if (strcmp(name, "DOWN") == 0)
    {

        moveDown();

        if (!gameOver)
        {
            lastFallTime = millis();
        }
    }

    // UP - ROTATE
    else if (strcmp(name, "UP") == 0)
    {

        rotatePiece();
    }

    drawBoard();

    if (gameOver)
    {
        drawGameOver();
    }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{

    Serial.begin(115200);
    delay(500);

    // Initialize TFT
    SPI.begin(
        TFT_SCLK,
        TFT_MISO,
        TFT_MOSI,
        TFT_CS);

    tft.init(240, 320);
    tft.setRotation(1);
    tft.invertDisplay(false);

    // Initialize buttons
    for (int i = 0; i < BUTTON_COUNT; i++)
    {

        pinMode(buttons[i].pin, INPUT_PULLUP);

        buttons[i].lastReading =
            digitalRead(buttons[i].pin);

        buttons[i].stableState =
            buttons[i].lastReading;

        buttons[i].lastChangeTime = millis();
    }

    // Random piece generation
    randomSeed(esp_random());

    // Start game
    restartGame();

    Serial.println("========================");
    Serial.println("      TETRIS READY");
    Serial.println("========================");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{

    unsigned long now = millis();

    // Button polling and debounce
    for (int i = 0; i < BUTTON_COUNT; i++)
    {

        bool reading = digitalRead(buttons[i].pin);

        if (reading != buttons[i].lastReading)
        {

            buttons[i].lastChangeTime = now;
            buttons[i].lastReading = reading;
        }

        if ((now - buttons[i].lastChangeTime) >= DEBOUNCE_MS)
        {

            if (reading != buttons[i].stableState)
            {

                buttons[i].stableState = reading;

                if (reading == LOW)
                {

                    onButtonPressed(buttons[i].name);
                }
            }
        }
    }

    // Automatic falling
    if (
        !gameOver &&
        now - lastFallTime >= FALL_INTERVAL)
    {

        lastFallTime = now;

        moveDown();

        drawBoard();

        if (gameOver)
        {
            drawGameOver();
        }
    }
}
