#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 500

#define TIMER_PERIOD  20 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

bool timer = 1; // Timer variable for pausing

#define D2R 0.0174532

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

int x1 = -300; // Position variable for linear-moving target

int sc = 0; // counter variable for space bar, used for counting the number of bullets fired

bool rf = 0, lf = 0, uf = 0, df = 0; // flags for right, left, up and down arrows

bool en1f = 1, en2f = 1;
// flags for displaying/removing target objects (target 1: linear moving, target 2: rotating)

bool frf = 1, dbf = 1;
// frf: Fire rate flag. Used for making sure that we draw only 1 bullet each time
// dbf: Display bullet flag. Used for removing the bullet object after a collision

typedef struct {
    int x;
    int y;
}point_t;
//Structure for coordinates

typedef struct
{
    point_t point;
    float angle;
} player_t;

player_t player;
// Structure to store the player information

typedef struct
{
    point_t bcoor;
    float bangle;
}bullet_t;

bullet_t bullet;
// Structure to store bullet object's information

typedef struct {
    int x1min;
    int x1max;
    int altitude;
}lintar_t;

lintar_t lintar;
// Structure to store linear moving target's information

typedef struct {
    float tangle;
    int x2max;
    int x2min;
    int y2min;
    int y2max;
}rottar_t;

rottar_t rottar;
// Structure to store rotating target's information

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

void drawTank(int x, int y)
// Function for drawing the tank object
{

    glColor3f(0, 0.5, 0); // Dark green color
    circle(x, y, 20);
    // Tank Turret

    glColor3f(0, 0, 0); // Black color
    glRectf(x - 25, y - 40, x + 25, y - 10);
    circle(x - 25, y - 25, 15);
    circle(x + 25, y - 25, 15);
    // Tank Tracks

    glColor3f(0, 0.5, 0); // Dark green color
    circle(x - 25, y - 25, 10);
    circle(x + 25, y - 25, 10);
    glRectf(x - 25, y - 35, x + 25, y - 15);
    // Tank Body

    glColor3f(0.5, 0.5, 0.5); // Gray color
    circle(x - 30, y - 30, 4);
    circle(x - 20, y - 30, 5);
    circle(x - 10, y - 30, 5);
    circle(x, y - 30, 5);
    circle(x + 10, y - 30, 5);
    circle(x + 20, y - 30, 5);
    circle(x + 30, y - 30, 4);
    // Tank Wheels
}

void drawGun(int x, int y)
// Function for drawing the tank's gun
// Gun and tank are two different objects, only the gun rotates
{
    glColor3f(0.3, 0.3, 0.3); // Gray color
    glLineWidth(6);
    glBegin(GL_LINES);
    glVertex2f(player.point.x, player.point.y - 125);
    glVertex2f(player.point.x + 50 * cos(player.angle * D2R),
        player.point.y - 125 + 50 * sin(player.angle * D2R));
    // Gun Barrel
    glEnd();
    glLineWidth(1);

}

void drawBullet(int x, int y)
// Function for drawing the bullet object (Circle shape)
{
    glColor3f(0, 0, 0); // Black color
    circle(x, y, 3);
}

void drawEnemy1(int x, int y)
// Function for drawing the 1st enemy object (Linear moving UFO)
{
    glColor3f(0.5, 0.5, 0.5); // Gray color
    glBegin(GL_POLYGON);
    glVertex2f(x - 14, y + 21);
    glVertex2f(x + 14, y + 21);
    glVertex2f(x + 49, y);
    glVertex2f(x + 21, y - 7);
    glVertex2f(x + 14, y - 21);
    glVertex2f(x - 14, y - 21);
    glVertex2f(x - 21, y - 7);
    glVertex2f(x - 49, y);
    glEnd();

    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - 14, y + 21);
    glVertex2f(x + 14, y + 21);
    glVertex2f(x + 21, y + 7);
    glVertex2f(x + 49, y);
    glVertex2f(x + 21, y - 7);
    glVertex2f(x + 14, y - 21);
    glVertex2f(x - 14, y - 21);
    glVertex2f(x - 21, y - 7);
    glVertex2f(x - 49, y);
    glVertex2f(x - 21, y + 7);
    glEnd();
    // Drawing the 1st UFO

    lintar.x1min = x - 49; // Left edge of UFO
    lintar.x1max = x + 49; // Right edge of UFO
    // These are for collision detection
}

void drawEnemy2(int x, int y)
// FUnction for drawing 2nd enemy object (Rotating UFO)
{
    int tarx, tary; // Variables for trigonometric operations

    tarx = x * cos(rottar.tangle * D2R);
    tary = y * sin(rottar.tangle * D2R);
    // Trigonometric operations for rotation movement

    glColor3f(0, 0, 1);
    circle(tarx, tary, 14);
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_POLYGON);
    glVertex2f(tarx + 14, tary);
    glVertex2f(tarx + 28, tary - 14);
    glVertex2f(tarx - 28, tary - 14);
    glVertex2f(tarx - 14, tary);
    glEnd();
    // Drawing the 2nd UFO

    rottar.x2max = tarx + 28; // Right edge of UFO
    rottar.x2min = tarx - 28; // Left edge of UFO
    rottar.y2max = tary + 14; // Top of UFO
    rottar.y2min = tary - 14; // Bottom of UFO
    // These are for collision detection
}


//
// To display onto window using OpenGL commands
//
void display() {
    //
    // clear window to black
    //
    glClearColor(0.5, 0.5, 1, 0); // Light blue color
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.2, 0.2, 0.2); // Dark gray color
    glRectf(-winWidth / 2, -winHeight / 2, winWidth / 2, -winHeight / 2.5);
    // Instructions panel

    glColor3f(1, 1, 1);
    vprint(-winWidth / 2 + 25, -winHeight / 2 + 20, GLUT_BITMAP_8_BY_13, "<Spacebar> Fire");
    vprint(-winWidth / 2 + 175, -winHeight / 2 + 30, GLUT_BITMAP_8_BY_13, "<F1> Pause/Restart");
    vprint(-winWidth / 2 + 175, -winHeight / 2 + 10, GLUT_BITMAP_8_BY_13, "<F2> Pause");
    vprint(-winWidth / 2 + 350, -winHeight / 2 + 30, GLUT_BITMAP_8_BY_13, "<Left/Right Arrow> Move Tank");
    vprint(-winWidth / 2 + 350, -winHeight / 2 + 10, GLUT_BITMAP_8_BY_13, "<Up/Down Arrow> Aim Gun");

    glColor3f(0, 0, 0); // Black color
    if ((5 - sc) > 0)
        vprint(winWidth / 2 - 90, winHeight / 2 - 40, GLUT_BITMAP_8_BY_13, "Bullets: %d", 5 - sc);
    // Counter showing how many bullets are left
    else
        vprint(winWidth / 2 - 90, winHeight / 2 - 40, GLUT_BITMAP_8_BY_13, "Bullets: 0");
    // if player is out of bullets, only shows "0", not negative numbers

    glColor3f(0.80, 0.5, 0.3); // Brown color
    glRectf(-winWidth / 2, -winHeight / 2.5, winWidth / 2, -winHeight / 3);
    // Drawing ground (desert)

    if (sc > 0 && dbf == 1 && sc < 6)
        // Space bar must be pressed, player must have bullets left and fire must be available (dbf)
    {
        drawBullet(bullet.bcoor.x, bullet.bcoor.y);
        // Displaying the bullet object
    }

    drawGun(player.point.x, -winHeight / 3 + 40); // Drawing gun object
    drawTank(player.point.x, -winHeight / 3 + 40); // Drawing tank object

    if (en1f == 1)
        drawEnemy1(x1, 150); // Draw enemy #1 (if it is not destroyed)
    if (en2f == 1)
        drawEnemy2(50, 50);  // Draw enemy #2 (if it is not destroyed)

    if (en1f == 0 && en2f == 0) // If both enemies are destroyed
    {
        glColor3f(1, 0, 0); // Red color
        vprint(-40, winHeight / 2 - 100, GLUT_BITMAP_8_BY_13, "You Win!");
        vprint(-40, winHeight / 2 - 120, GLUT_BITMAP_8_BY_13, "F1 to Restart");
    }

    if (en1f == 1 && sc > 4 || en2f == 1 && sc > 4)
        // If player is out of bullets and not all enemies are destroyed
    {
        glColor3f(1, 0, 0);
        vprint(200, winHeight / 2 - 180, GLUT_BITMAP_8_BY_13, "You Lost!");
        vprint(200, winHeight / 2 - 200, GLUT_BITMAP_8_BY_13, "You ran out of bullets!");
        vprint(200, winHeight / 2 - 220, GLUT_BITMAP_8_BY_13, "F1 to Restart");
    }

    glutSwapBuffers();
}


//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    if (key == 32) // Space bar
    {
        if (frf == 1) { // If firing is available
            sc++; // increment space bar counter

            bullet.bcoor.x = player.point.x + 50 * cos(player.angle * D2R);
            // Initializing horizonal posiion of the bullet

            bullet.bcoor.y = player.point.y - 125 + 50 * sin(player.angle * D2R);
            // Initializing horizonal posiion of the bullet

            bullet.bangle = player.angle;
            // Initializing angle of the bullet

            frf = 0;
            // firing a second bullet is unavailable until the first bullet goes out of the window
            if (dbf == 0)
                // If a collision has happened and the last bullet was destroyed in the collision
                dbf = 1; // Displaying bullets is available again!
        }
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_F1:

        if (en1f == 1 && en2f == 1 && sc < 5)
            timer = !timer; // F1 pauses the game in the beginning
        else
        {
            en1f = 1; // reset the condition of 1st enemy
            en2f = 1; // reset the condition of 1st enemy
            player.point.x = 0; // reset tank position
            sc = 0; // reset number of bullets left
            player.angle = 0; // reset gun angle
            frf = 1; // firing is available again!
        } // F1 restarts the game once the player makes progress e.g. destroys a target

        break;
    case GLUT_KEY_F2:
        timer = !timer; // F2 for only pausing
        break;
    case GLUT_KEY_UP:
        uf = 1; // Press and hold up arrow
        break;
    case GLUT_KEY_DOWN:
        df = 1; // Press and hold down arrow
        break;
    case GLUT_KEY_LEFT:
        lf = 1; // Press and hold left arrow
        break;
    case GLUT_KEY_RIGHT:
        rf = 1; // Press and hold right arrow
        break;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {

    case GLUT_KEY_UP:
        uf = 0; // letting go of the up arrow
        break;
    case GLUT_KEY_DOWN:
        df = 0; // letting go of the down arrow
        break;
    case GLUT_KEY_LEFT:
        lf = 0; // letting go of the left arrow
        break;
    case GLUT_KEY_RIGHT:
        rf = 0; // letting go of the right arrow
        break;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
    // Write your codes here.



    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function   
    glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function
    glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
    // Write your codes here.

    lintar.altitude = 150; // altitude of 1st enemy is 150

    if (timer) { // If the game is not paused...

        x1 += 7; // 1st enemy linear speed is 7
        if (x1 >= winWidth)
            x1 = -400; // It comes out the other side of the screen

        rottar.tangle += 11;
        // The angle of the 2nd enemy changes at a speed of 11 degrees per timer period

        if (rf == 1 && player.point.x < winWidth / 2 - 60)
            player.point.x += 3;
        // Moving the tank and gun objects to the right when right arrow key is pressed and held
        // Tank speed is 3

        else if (lf == 1 && player.point.x > -winWidth / 2 + 60)
            player.point.x -= 3;
        // Moving the tank and gun objects to the left when left arrow key is pressed and held

        if (uf == 1 && player.angle < 179)
            player.angle += 3;
        // Rotating the gun object counter-clockwise when up arrow key is pressed and held

        if (df == 1 && player.angle > 0)
            player.angle -= 3;
        // Rotating the gun object clockwise when down arrow key is pressed and held

        if (sc >= 1 && dbf == 1 && sc <= 5)
            // Condition for drawing bullet objects: 
            //Space bar must be pressed, player must have bullets left and fire must be available (dbf)
        {
            bullet.bcoor.x += cos(bullet.bangle * D2R) * 10; // Horizontal movement of the bullet
            bullet.bcoor.y += sin(bullet.bangle * D2R) * 10; // Vertical movement of the bullet

            if (bullet.bcoor.y > lintar.altitude && bullet.bcoor.y < winHeight / 2
                && bullet.bcoor.x >= lintar.x1min && bullet.bcoor.x <= lintar.x1max)
                // Condition for collision with 1st enemy
            {
                en1f = 0; // Stop displaying 1st enemy after the collision
                dbf = 0; // Stop displaying the bullet after the collision
            }

            if (bullet.bcoor.x < rottar.x2max && bullet.bcoor.x > rottar.x2min
                && bullet.bcoor.y < rottar.y2max && bullet.bcoor.y > rottar.y2min && en2f == 1)
                // Condition for collision with 2nd enemy
            {
                en2f = 0; // Stop displaying 2nd enemy after the collision
                dbf = 0; // Stop displaying the bullet after the collision
            }

            if (bullet.bcoor.x > 400 || bullet.bcoor.x < -400 || bullet.bcoor.y > 250 || dbf == 0)
                frf = 1;
            // Player cannot fire a second bullet before the first bullet goes out of the window
            // This moderates the fire rate (frf)
        }
    }

    // to refresh the window it calls display() function
    glutPostRedisplay(); // display()

}
#endif

void Init() {

    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //glutInitWindowPosition(100, 100);
    glutCreateWindow("TANK vs. ALIENS");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    //
    // keyboard registration
    //
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);

    //
    // mouse registration
    //
    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    Init();

    glutMainLoop();
}