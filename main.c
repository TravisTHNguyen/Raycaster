#define _USE_MATH_DEFINES


#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#include "Textures.h"

float degToRad(float a) { return a * M_PI / 180.0; }
float FixAng(float a) { if (a > 359) { a -= 360; } if (a < 0) { a += 360; } return a; }
float distance(ax, ay, bx, by, ang) { return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay); }
float px, py, pdx, pdy, pa;
float frame1, frame2, fps;
int gameState = 0, timer = 0; //game state. init, start screen, game loop, win/lose
float fade = 0;             //the 3 screens can fade up from black

typedef struct
{
    int w, a, d, s;                     //button state on off
}ButtonKeys; ButtonKeys Keys;

//-----------------------------MAP----------------------------------------------
#define mapX  8      //map width
#define mapY  8      //map height
#define mapS 64      //map cube size
                     //Edit these 3 arrays with values 0-4 to change map
int mapW[] =          //walls
{
 9,1,1,1,2,2,2,2,
 4,0,0,1,0,0,0,2,
 9,0,0,4,0,2,0,2,
 9,5,4,5,0,0,0,2,
 9,0,0,0,0,0,0,1,
 9,0,0,0,0,1,0,1,
 9,0,0,0,0,0,0,1,
 9,1,1,1,1,1,1,1,
};

int mapF[] =          //floors
{
 8,8,8,8,8,8,8,8,
 8,8,0,8,8,8,8,6,
 8,7,8,8,8,8,8,8,
 8,7,8,6,8,6,8,8,
 8,4,2,4,9,6,8,8,
 8,7,2,6,9,9,8,8,
 8,7,2,8,6,8,8,8,
 8,7,8,8,9,9,8,8,
};

int mapC[] =          //ceiling
{
 6,6,6,6,6,6,6,6,
 0,0,0,6,6,0,0,6,
 0,0,0,6,6,6,0,6,
 0,0,0,6,0,6,0,6,
 0,4,2,4,6,6,0,6,
 0,0,2,6,6,6,6,0,
 0,6,2,0,6,0,0,0,
 6,6,6,6,6,0,0,0,
};


typedef struct       //All veriables per sprite
{
    int type;           //static, key, enemy
    int state;          //on off
    int map;            //texture to show
    float x, y, z;        //position
}sprite;
sprite spriteArr[4];
int depth[120];      //hold wall line depth to compare for sprite depth

void drawSprite()
{
    int x, y, s;
    if (px<spriteArr[0].x + 30 && px>spriteArr[0].x - 30 && py<spriteArr[0].y + 30 && py>spriteArr[0].y - 30) { spriteArr[0].state = 0; } //pick up key 	
    if (px<spriteArr[3].x + 30 && px>spriteArr[3].x - 30 && py<spriteArr[3].y + 30 && py>spriteArr[3].y - 30) { gameState = 4; } //enemy kills

    //enemy attack
    int spx = (int)spriteArr[3].x >> 6, spy = (int)spriteArr[3].y >> 6;          //normal grid position
    int spx_add = ((int)spriteArr[3].x + 15) >> 6, spy_add = ((int)spriteArr[3].y + 15) >> 6; //normal grid position plus     offset
    int spx_sub = ((int)spriteArr[3].x - 15) >> 6, spy_sub = ((int)spriteArr[3].y - 15) >> 6; //normal grid position subtract offset
    if (spriteArr[3].x > px && mapW[spy * 8 + spx_sub] == 0) { spriteArr[3].x -= 0.04 * fps; }
    if (spriteArr[3].x < px && mapW[spy * 8 + spx_add] == 0) { spriteArr[3].x += 0.04 * fps; }
    if (spriteArr[3].y > py && mapW[spy_sub * 8 + spx] == 0) { spriteArr[3].y -= 0.04 * fps; }
    if (spriteArr[3].y < py && mapW[spy_add * 8 + spx] == 0) { spriteArr[3].y += 0.04 * fps; }

    for (s = 0; s < 4; s++)
    {
        float sx = spriteArr[s].x - px; //temp float variables
        float sy = spriteArr[s].y - py;
        float sz = spriteArr[s].z;

        float CS = cos(degToRad(pa)), SN = sin(degToRad(pa)); //rotate around origin
        float a = sy * CS + sx * SN;
        float b = sx * CS - sy * SN;
        sx = a; sy = b;

        sx = (sx * 108.0 / sy) + (120 / 2); //convert to screen x,y
        sy = (sz * 108.0 / sy) + (80 / 2);

        int scale = 32 * 80 / b;   //scale sprite based on distance
        if (scale < 0) { scale = 0; } if (scale > 120) { scale = 120; }

        //texture
        float t_x = 0, t_y = 31, t_x_step = 31.5 / (float)scale, t_y_step = 32.0 / (float)scale;

        for (x = sx - scale / 2; x < sx + scale / 2; x++)
        {
            t_y = 31;
            for (y = 0; y < scale; y++)
            {
                if (spriteArr[s].state == 1 && x > 0 && x < 120 && b < depth[x])
                {
                    int pixel = ((int)t_y * 32 + (int)t_x) * 3 + (spriteArr[s].map * 32 * 32 * 3);
                    int red = sprites[pixel + 0];
                    int green = sprites[pixel + 1];
                    int blue = sprites[pixel + 2];
                    if (red != 255, green != 0, blue != 255) //dont draw if purple
                    {
                        glPointSize(8); 
                        glColor3ub(red, green, blue); 
                        glBegin(GL_POINTS); 
                        glVertex2i(x * 8, sy * 8 - y * 8); 
                        glEnd(); //draw point 
                    }
                    t_y -= t_y_step; if (t_y < 0) { t_y = 0; }
                }
            }
            t_x += t_x_step;
        }
    }
}

//void drawMap2D()
//{
//    int x, y, xo, yo;
//    for (y = 0; y < mapY; y++)
//    {
//        for (x = 0; x < mapX; x++)
//        {
//            if (mapW[y * mapX + x] > 0) { glColor3f(1, 1, 1); }
//            else { glColor3f(0, 0, 0); }
//            xo = x * mapS; yo = y * mapS;
//            glBegin(GL_QUADS);
//            glVertex2i(0 + xo + 1, 0 + yo + 1);
//            glVertex2i(0 + xo + 1, mapS + yo - 1);
//            glVertex2i(mapS + xo - 1, mapS + yo - 1);
//            glVertex2i(mapS + xo - 1, 0 + yo + 1);
//            glEnd();
//        }
//    }
//}//-----------------------------------------------------------------------------
//
//
////------------------------PLAYER------------------------------------------------
//void drawPlayer2D()
//{
//    glColor3f(1, 1, 0);   glPointSize(8);    glLineWidth(4);
//    glBegin(GL_POINTS); glVertex2i(px, py); glEnd();
//    glBegin(GL_LINES);  glVertex2i(px, py); glVertex2i(px + pdx * 20, py + pdy * 20); glEnd();
//}//-----------------------------------------------------------------------------

//---------------------------Draw Rays and Walls--------------------------------
void drawRays2D()
{
    int r, mx, my, mp, dof, side; float vx, vy, rx, ry, ra, xAngle, yAngle, disV, disH;

    ra = FixAng(pa + 30);                                                              //ray set back 30 degrees

    for (r = 0; r < 120; r++)
    {
        int vmt = 0, hmt = 0;                                                              //vertical and horizontal map texture number 
        //---Vertical--- 
        dof = 0; side = 0; disV = 100000;
        float Tan = tan(degToRad(ra));
        if (cos(degToRad(ra)) > 0.001) { rx = (((int)px >> 6) << 6) + 64;      ry = (px - rx) * Tan + py; xAngle = 64; yAngle = -xAngle * Tan; }//looking left
        else if (cos(degToRad(ra)) < -0.001) { rx = (((int)px >> 6) << 6) - 0.0001; ry = (px - rx) * Tan + py; xAngle = -64; yAngle = -xAngle * Tan; }//looking right
        else { rx = px; ry = py; dof = 8; }                                                  //looking up or down. no hit  

        while (dof < 8)
        {
            mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && mapW[mp]>0) { vmt = mapW[mp] - 1; dof = 8; disV = cos(degToRad(ra)) * (rx - px) - sin(degToRad(ra)) * (ry - py); }//hit         
            else { rx += xAngle; ry += yAngle; dof += 1; }                                               //check next horizontal
        }
        vx = rx; vy = ry;

        //---Horizontal---
        dof = 0; disH = 100000;
        Tan = 1.0 / Tan;
        if (sin(degToRad(ra)) > 0.001) { ry = (((int)py >> 6) << 6) - 0.0001; rx = (py - ry) * Tan + px; yAngle = -64; xAngle = -yAngle * Tan; }//looking up 
        else if (sin(degToRad(ra)) < -0.001) { ry = (((int)py >> 6) << 6) + 64;      rx = (py - ry) * Tan + px; yAngle = 64; xAngle = -yAngle * Tan; }//looking down
        else { rx = px; ry = py; dof = 8; }                                                   //looking straight left or right

        while (dof < 8)
        {
            mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && mapW[mp]>0) { hmt = mapW[mp] - 1; dof = 8; disH = cos(degToRad(ra)) * (rx - px) - sin(degToRad(ra)) * (ry - py); }//hit         
            else { rx += xAngle; ry += yAngle; dof += 1; }                                               //check next horizontal
        }

        float shade = 1;
        glColor3f(0, 0.8, 0);
        if (disV < disH) { hmt = vmt; shade = 0.5; rx = vx; ry = vy; disH = disV; glColor3f(0, 0.6, 0); }//horizontal hit first

        int ca = FixAng(pa - ra); disH = disH * cos(degToRad(ca));                            //fix fisheye 
        int lineH = (mapS * 640) / (disH);
        float ty_step = 32.0 / (float)lineH;
        float ty_off = 0;
        if (lineH > 640) { ty_off = (lineH - 640) / 2.0; lineH = 640; }                            //line height and limit
        int lineOff = 320 - (lineH >> 1);                                               //line offset

        depth[r] = disH; //save this line's depth
        //---draw walls---
        int y;
        float ty = ty_off * ty_step;//+hmt*32;
        float tx;
        if (shade == 1) { 
            tx = (int)(rx / 2.0) % 32; if (ra > 180) { 
                tx = 31 - tx; 
            } 
        }
        else { 
            tx = (int)(ry / 2.0) % 32; if (ra > 90 && ra < 270) { 
            
                tx = 31 - tx; 
            } 
        }
        for (y = 0; y < lineH; y++)
        {
            int pixel = ((int)ty * 32 + (int)tx) * 3 + (hmt * 32 * 32 * 3);
            int red = All_Textures[pixel + 0] * shade;
            int green = All_Textures[pixel + 1] * shade;
            int blue = All_Textures[pixel + 2] * shade;
            glPointSize(8); 
            glColor3ub(red, green, blue); 
            glBegin(GL_POINTS); 
            glVertex2i(r * 8, y + lineOff); 
            glEnd();
            ty += ty_step;
        }

        //---draw floors---
        for (y = lineOff + lineH; y < 640; y++)
        {
            float dy = y - (640 / 2.0), deg = degToRad(ra), raFix = cos(degToRad(FixAng(pa - ra)));
            tx = px / 2 + cos(deg) * 158 * 2 * 32 / dy / raFix;
            ty = py / 2 - sin(deg) * 158 * 2 * 32 / dy / raFix;
            int mp = mapF[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
            int pixel = (((int)(ty) & 31) * 32 + ((int)(tx) & 31)) * 3 + mp * 3;
            int red = All_Textures[pixel + 0] * 0.7;
            int green = All_Textures[pixel + 1] * 0.7;
            int blue = All_Textures[pixel + 2] * 0.7;
            glPointSize(8); 
            glColor3ub(red, green, blue); 
            glBegin(GL_POINTS); 
            glVertex2i(r * 8, y); 
            glEnd();

            //---draw ceiling---
            mp = mapC[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
            pixel = (((int)(ty) & 31) * 32 + ((int)(tx) & 31)) * 3 + mp * 3;
            red = All_Textures[pixel + 0];
            green = All_Textures[pixel + 1];
            blue = All_Textures[pixel + 2];
            if (mp > 0) { glPointSize(8); 
            glColor3ub(red, green, blue); 
            glBegin(GL_POINTS); 
            glVertex2i(r * 8, 640 - y); 
            glEnd(); }
        }

        ra = FixAng(ra - 0.5);                                                               //go to next ray, 60 total
    }
}//-----------------------------------------------------------------------------


void drawSky()     //draw sky and rotate based on player rotation
{
    int x, y;
    for (y = 0; y < 40; y++)
    {
        for (x = 0; x < 120; x++)
        {
            int xo = (int)pa * 2 - x; if (xo < 0) { xo += 120; } xo = xo % 120; //return 0-120 based on player angle
            int pixel = (y * 120 + xo) * 3;
            int red = sky[pixel + 0];
            int green = sky[pixel + 1];
            int blue = sky[pixel + 2];
            glPointSize(8); 
            glColor3ub(red, green, blue); 
            glBegin(GL_POINTS); 
            glVertex2i(x * 8, y * 8); 
            glEnd();
        }
    }
}

void screen(int v) //draw any full screen image. 120x80 pixels
{
    int x, y;
    int* T = NULL;
    if (v == 1) { T = title; }
    if (v == 2) { T = won; }
    if (v == 3) { T = lost; }
    for (y = 0; y < 80; y++)
    {
        for (x = 0; x < 120; x++)
        {
            int pixel = (y * 120 + x) * 3;
            int red = T[pixel + 0] * fade;
            int green = T[pixel + 1] * fade;
            int blue = T[pixel + 2] * fade;
            glPointSize(8); 
            glColor3ub(red, green, blue); 
            glBegin(GL_POINTS); 
            glVertex2i(x * 8, y * 8); 
            glEnd();
        }
    }
    if (fade < 1) { fade += 0.001 * fps; }
    if (fade > 1) { fade = 1; }
}


void init()//init all variables when game starts
{
    glClearColor(0.3, 0.3, 0.3, 0);
    px = 150; py = 400; pa = 90;
    pdx = cos(degToRad(pa)); pdy = -sin(degToRad(pa));                                 //init player
    mapW[19] = 4; 
    mapW[26] = 4; //close doors

    //can add more sprites by extending textures
    spriteArr[0].type = 1; spriteArr[0].state = 1; spriteArr[0].map = 0; spriteArr[0].x = 1.5 * 64; spriteArr[0].y = 5 * 64;   spriteArr[0].z = 20; //key
    spriteArr[1].type = 2; spriteArr[1].state = 1; spriteArr[1].map = 1; spriteArr[1].x = 1.5 * 64; spriteArr[1].y = 4.5 * 64; spriteArr[1].z = 0; //light 1
    spriteArr[2].type = 2; spriteArr[2].state = 1; spriteArr[2].map = 1; spriteArr[2].x = 3.5 * 64; spriteArr[2].y = 4.5 * 64; spriteArr[2].z = 0; //light 2
    spriteArr[3].type = 3; spriteArr[3].state = 1; spriteArr[3].map = 2; spriteArr[3].x = 2.5 * 64; spriteArr[3].y = 2 * 64;   spriteArr[3].z = 20; //enemy
}


static void display()
{
    //frames per second
    frame2 = glutGet(GLUT_ELAPSED_TIME); fps = (frame2 - frame1); frame1 = glutGet(GLUT_ELAPSED_TIME);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameState == 0) { init(); fade = 0; timer = 0; gameState = 1; } //init game
    if (gameState == 1) { screen(1); timer += 1 * fps; if (timer > 2000) { fade = 0; timer = 0; gameState = 2; } } //start screen
    if (gameState == 2) //The main game loop
    {
        //buttons
        if (Keys.a == 1) { pa += 0.2 * fps; pa = FixAng(pa); pdx = cos(degToRad(pa)); pdy = -sin(degToRad(pa)); }
        if (Keys.d == 1) { pa -= 0.2 * fps; pa = FixAng(pa); pdx = cos(degToRad(pa)); pdy = -sin(degToRad(pa)); }

        int xo = 0; if (pdx < 0) { xo = -20; }
        else { xo = 20; }                                    //x offset to check map
        int yo = 0; if (pdy < 0) { yo = -20; }
        else { yo = 20; }                                    //y offset to check map
        int ipx = px / 64.0, ipx_add_xo = (px + xo) / 64.0, ipx_sub_xo = (px - xo) / 64.0;             //x position and offset
        int ipy = py / 64.0, ipy_add_yo = (py + yo) / 64.0, ipy_sub_yo = (py - yo) / 64.0;             //y position and offset
        if (Keys.w == 1)                                                                  //move forward
        {
            if (mapW[ipy * mapX + ipx_add_xo] == 0) { px += pdx * 0.2 * fps; }
            if (mapW[ipy_add_yo * mapX + ipx] == 0) { py += pdy * 0.2 * fps; }
        }
        if (Keys.s == 1)                                                                  //move backward
        {
            if (mapW[ipy * mapX + ipx_sub_xo] == 0) { px -= pdx * 0.2 * fps; }
            if (mapW[ipy_sub_yo * mapX + ipx] == 0) { py -= pdy * 0.2 * fps; }
        }
        drawSky();
        drawRays2D();
        drawSprite();
        if ((int)px >> 6 == 1 && (int)py >> 6 == 1) { fade = 0; timer = 0; gameState = 3; } //Entered block 1, Win game!!
    }

    if (gameState == 3) { screen(2); timer += 1 * fps; if (timer > 2000) { fade = 0; timer = 0; gameState = 0; } } //won screen
    if (gameState == 4) { screen(3); timer += 1 * fps; if (timer > 2000) { fade = 0; timer = 0; gameState = 0; } } //lost screen

    glutPostRedisplay();
    glutSwapBuffers();
}

void ButtonDown(unsigned char key, int x, int y)                                  //keyboard button pressed down
{
    if (key == 'a') { Keys.a = 1; }
    if (key == 'd') { Keys.d = 1; }
    if (key == 'w') { Keys.w = 1; }
    if (key == 's') { Keys.s = 1; }
    if (key == 'e' && spriteArr[0].state == 0)             //open doors
    {
        int xo = 0; if (pdx < 0) { xo = -25; }
        else { xo = 25; }
        int yo = 0; if (pdy < 0) { yo = -25; }
        else { yo = 25; }
        int ipx = px / 64.0, ipx_add_xo = (px + xo) / 64.0;
        int ipy = py / 64.0, ipy_add_yo = (py + yo) / 64.0;
        if (mapW[ipy_add_yo * mapX + ipx_add_xo] == 4) { mapW[ipy_add_yo * mapX + ipx_add_xo] = 0; }
    }

    glutPostRedisplay();
}

void ButtonUp(unsigned char key, int x, int y)                                    //keyboard button pressed up
{
    if (key == 'a') { Keys.a = 0; }
    if (key == 'd') { Keys.d = 0; }
    if (key == 'w') { Keys.w = 0; }
    if (key == 's') { Keys.s = 0; }
    glutPostRedisplay();
}

void resize(int w, int h)                                                        //screen window rescaled, snap back
{
    glutReshapeWindow(960, 640);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(960, 640);
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2 - 960 / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - 640 / 2);
    glutCreateWindow("Raycaster");
    gluOrtho2D(0, 960, 640, 0);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
    glutMainLoop();
}
