#include<Gamby.h>
#include <avr/pgmspace.h>

GambyGraphicsMode gamby;

extern const long font[];
const unsigned int COLUMNS = 9; //Columns of bricks
const unsigned int ROWS = 4;     //Rows of bricks
const unsigned int BRICK_WIDTH = 6;
const unsigned int BRICK_HEIGHT = 3;
const unsigned int DRAW_SPEED = 100;

char text[16];

long timeToDraw;
byte tick;

int dx = -1;        //Initial movement of ball
int dy = -1;        //Initial movement of ball
int xb;           //Balls starting position
int yb;           //Balls starting position
boolean released;     //If the ball has been released by the player
boolean paused = false;   //If the game has been paused
int xPaddle;       //X position of paddle
boolean isHit[ROWS][COLUMNS];   //Array of if bricks are hit or not
boolean bounced = false; //Used to fix double bounce glitch
byte lives = 3;       //Amount of lives
byte level = 1;       //Current level
unsigned int score = 0; //Score for the game
unsigned int brickCount;  //Amount of bricks hit

//Ball Bounds used in collision detection
byte leftBall;
byte rightBall;
byte topBall;
byte bottomBall;

//Brick Bounds used in collision detection
byte leftBrick;
byte rightBrick;
byte topBrick;
byte bottomBrick;

void setup() {
  gamby.font = font;
  gamby.drawPattern = PATTERN_BLACK;

  xPaddle = 55;
  xb = 62;
  yb = 54;

  srand(millis());

  timeToDraw = millis() + DRAW_SPEED;
  Serial.begin(9600);

  for (byte i = 0; i < ROWS; i++) {
    for (byte j = 0; j < COLUMNS; j++) {
      isHit[i][j] = false;
    }
  }

  drawPaddle();
  drawBall();
}

void drawLives() {
  for(byte i = 0; i < lives; i++){
    gamby.circle(4 * i + 6, 58, 1);
  }
  gamby.update();
}

void drawPaddle() {
  gamby.rect(xPaddle, 58, xPaddle + 14, 61);
  gamby.update();
}

void movePaddle() {
  //move left
  if ((gamby.inputs & DPAD_LEFT) && xPaddle + 4 > 0) {
    xPaddle -= 4;
  }
  //move left
  if ((gamby.inputs & DPAD_RIGHT) && xPaddle + 14 < 93) {
    xPaddle += 4;
  }
}

void drawBlocks() {
  int brickX = 4;
  int brickY = 6;

  for (byte i = 0; i < ROWS; i++) {
    for (byte j = 0; j < COLUMNS; j++, brickX += 10) {

      if (isHit[i][j])
        continue;

      gamby.rect(
        brickX,  //x1
        brickY,  //y1
        brickX + BRICK_WIDTH,  //x2
        brickY + BRICK_HEIGHT); //y2
    }

    brickX = 4;
    brickY += 6;
  }
  gamby.update();
}

void drawBall() {
  gamby.circle(xb, yb, 2);
  gamby.update();
}

void moveBall() {
  tick++;
  if (released) {

    if (abs(dx) == 2) {
      xb += dx / 2;

      if (tick % 2 == 0) {
        xb += dx / 2;
      }

    } else {
      xb += dx;
    }
    yb += dy;

    leftBall = xb;
    rightBall = xb + 2;
    topBall = yb;
    bottomBall = yb + 2;

  } else {
    //ball follows paddle
    xb = xPaddle + 7;

    //if click any button, start ball motion
    if (gamby.inputs & (BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4)) {
      released = true;

      //go left or right ??
      if (random(0, 2) == 0) {
        dx = 1;
      } else {
        dx = -1;
      }

      //guarantee ball motion upwards
      dy = -1;
    }
  }
}

void hitTop() {
  if (yb <= 0) {
    yb = 2;
    dy = -dy;
  }
}

void hitBottom() {
  if (yb > 64) {
    //hit bottom
    --lives;
    xPaddle = 55;
    xb = 62;
    yb = 54;
    released = false;
  }
}

void hitLeft() {
  if (xb <= 0) {
    xb = 2;
    dx = -dx;
  }
}

void hitRight() {
  if (xb > 94) {
    xb = 94 - 2;
    dx = -dx;
  }
}

void hitPaddle() {
  if (xb + 1 >= xPaddle && xb <= xPaddle + 12 && yb + 2 >= 58 && yb <= 61)
  {
    dy = -dy;
    dx = ((xb - (xPaddle + 6)) / 3); //Applies spin on the ball
    // prevent straight bounce
    if (dx == 0) {
      dx = (random(0, 2) == 1) ? 1 : -1;
    }
  }
}

void hitBlocks() {

  int brickX = 4;
  int brickY = 6;

  for (byte row = 0; row < ROWS; row++)
  {
    for (byte column = 0; column < COLUMNS; column++, brickX += 10)
    {
      if (!isHit[row][column])
      {
        leftBrick = brickX;
        rightBrick = brickX + BRICK_WIDTH;
        topBrick = brickY;
        bottomBrick = brickY + BRICK_HEIGHT;

        //collison has occured
        if (topBall <= bottomBrick && bottomBall >= topBrick &&
            leftBall <= rightBrick && rightBall >= leftBrick)
        {
          isHit[row][column] = true;
          brickCount++;
          tone(9, 220, 400);

          if (bottomBall > bottomBrick || topBall < topBrick) {
            //Only bounce once each ball move
            if (!bounced)
            {
              dy = - dy;
              yb += dy;
              bounced = true;
            }
          }

          if (leftBall < leftBrick || rightBall > rightBrick)
          {
            //Only bounce once brick each ball move
            if (!bounced)
            {
              dx = - dx;
              xb += dx;
              bounced = true;
            }
          }
        }
      }
    }

    brickX = 4;
    brickY += 6;
  }
  bounced = false;
}

void checkCollisions() {
  hitTop();
  hitBottom();
  hitLeft();
  hitRight();
  hitPaddle();
  hitBlocks();
}

void loop() {
  if (!paused) {
    if (millis() > timeToDraw) {
      gamby.readInputs();
      gamby.clearScreen();

      drawLives();
      drawBlocks();
      movePaddle();
      drawPaddle();
      moveBall();
      drawBall();
      checkCollisions();

      timeToDraw = millis() + DRAW_SPEED;
    }
  }
}
