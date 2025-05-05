//
//program: space.cpp
//author: team 7
//framework program: asteroids.cpp
//framework author:  Gordon Griesel
//date:    2014 - 2025
//
//------------------------------------------------------------
//changes made:
//------------------------------------------------------------

#define GGLINUX
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/keysym.h>
#include "log.h"
#include "fonts.h"
#include "cbonoan.h"
#include <pthread.h>
#include <atomic>
#include "bbarrios.h"
#include "eaviles.h"
#include "davalos.h"

//texture variables
//GLuint ufoTexture;
//int ufoWidth, ufoHeight;

using namespace std; 

//defined types
typedef float Flt;
typedef float Vec[3];
typedef Flt	Matrix[4][4];

//macros
#define rnd() (((Flt)rand())/(Flt)RAND_MAX)
#define random(a) (rand()%a)
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
                             (c)[1]=(a)[1]-(b)[1]; \
(c)[2]=(a)[2]-(b)[2]

//constants
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 100;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;

//-----------------------------------------------------------------------------
//Setup timers
const double physicsRate = 1.0 / 60.0;
const double oobillion = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern struct timespec timePause;
extern double physicsCountdown;
extern double timeSpan;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
//-----------------------------------------------------------------------------

Vec mousePos = {0, 0, 0};
GLuint shipTextures[3];
int selectedShip = 0;

class Global {
    public:
        int xres, yres;
        char keys[65536];
        int mouse_cursor_on;
        int credits;
        int instructions;
        GLuint titleTexture;
        GLuint backgroundTexture;
        GLuint pauseTexture;
        GLuint gameOverTexture;
        int title;
        int background;
        int pause;
        int gameOver;
        atomic<float> polledMouseX{0};
        atomic<float> polledMouseY{0};
        Global() {
            xres = 1280;
            yres = 720;
            memset(keys, 0, 65536);
            mouse_cursor_on = 1;
            credits = 0;
            instructions = 0;
            title = 1;
            background = 1;
            gameOver = 0;
        }
} gl;

class Ship {
    public:
        Vec pos;
        Vec dir;
        Vec vel;
        Vec acc;
        float angle;
        float color[3];
        float health;   // 100 = full health, 0 = empty
    public:
        Ship() {
            pos[0] = (Flt)(gl.xres/2);
            pos[1] = (Flt)(gl.yres/2);
            pos[2] = 0.0f;
            VecZero(dir);
            VecZero(vel);
            VecZero(acc);
            angle = 0.0;
            color[0] = color[1] = color[2] = 1.0;
            health = 1.0;
        }
        void takeDamage(float damage) {
            health -= damage;
            if (health < 0)
                health = 0;
        }
        void heal(float amount) {
            health += amount;
            if (health > 1.0)
                health = 1.0;
        }
        //float getHealth() { return health; }
};

class Bullet {
    public:
        Vec pos;
        Vec vel;
        float color[3];
        struct timespec time;
    public:
        Bullet() { }
};

class Game {
    public:
        int ufo;
        int score;
        GLuint stardustTexture;
        GLuint ufoTexture;
        Vec pos;
        Ship ship;
        Bullet *barr;
        int nbullets;
        int nenemies;
        struct timespec bulletTimer;
        struct timespec mouseThrustTimer;
        bool mouseThrustOn;
        enum GameMode {
            MAIN_MENU,
            ENDLESS_MODE,
            BOSS_MODE,
            SHIP_SELECTION,
            PAUSE_MENU
        };
        GameMode gameMode;
        bool inMenu;
        int menuSelection;
        bool isPaused;
        bool isEnd;
        bool prevKeys[65536];
        int lastGameMode;
    public:
        Game() {
            ufo = 1;
            barr = new Bullet[MAX_BULLETS];
            nbullets = 0;
            nenemies = 0;
            mouseThrustOn = false;
            inMenu = true;
            gameMode = MAIN_MENU;
            menuSelection = 0;
            isPaused = false;
            isEnd = false;
            score = 0;
            lastGameMode = MAIN_MENU;
            memset(prevKeys, 0, sizeof(prevKeys));
            clock_gettime(CLOCK_REALTIME, &bulletTimer);
        }

        ~Game() {
            delete [] barr;
        }
} g;

class Image {
    public:
        int width, height;
        unsigned char *data;
        ~Image() { delete [] data; }
        Image(const char *fname) {
            if (fname[0] == '\0')
                return;
            int ppmFlag = 0;
            char name[40];
            strcpy(name, fname);
            int slen = strlen(name);
            char ppmname[80];
            if (strncmp(name+(slen-4), ".ppm", 4) == 0)
                ppmFlag = 1;
            if (ppmFlag) {
                strcpy(ppmname, name);
            } else {
                name[slen-4] = '\0';
                sprintf(ppmname,"%s.ppm", name);
                char ts[100];
                sprintf(ts, "convert %s %s", fname, ppmname);
                system(ts);
            }   
            FILE *fpi = fopen(ppmname, "r");
            if (fpi) {
                char line[200];
                fgets(line, 200, fpi);
                fgets(line, 200, fpi);
                while (line[0] == '#' || strlen(line) < 2)
                    fgets(line, 200, fpi);
                sscanf(line, "%i %i", &width, &height);
                fgets(line, 200, fpi);
                int n = width * height * 3;
                data = new unsigned char[n];
                for (int i=0; i<n; i++)
                    data[i] = fgetc(fpi);
                fclose(fpi);
            } else {
                printf("ERROR opening image: %s\n",ppmname);
                exit(0);
            }   
            if (!ppmFlag)
                unlink(ppmname);
        }   
};

Image img[2] = { 
    "./images/title.png",
    "./images/gameover.png"
};

//X Windows variables
class X11_wrapper {
    private:
        Display *dpy;
        Window win;
        GLXContext glc;
    public:
        X11_wrapper() { }
        X11_wrapper(int w, int h) {
            GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
            //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
            XSetWindowAttributes swa;
            setup_screen_res(gl.xres, gl.yres);
            dpy = XOpenDisplay(NULL);
            if (dpy == NULL) {
                std::cout << "\n\tcannot connect to X server" << std::endl;
                exit(EXIT_FAILURE);
            }
            Window root = DefaultRootWindow(dpy);
            XWindowAttributes getWinAttr;
            XGetWindowAttributes(dpy, root, &getWinAttr);
            int fullscreen = 0;
            gl.xres = w;
            gl.yres = h;
            if (!w && !h) {
                //Go to fullscreen.
                gl.xres = getWinAttr.width;
                gl.yres = getWinAttr.height;
                //When window is fullscreen, there is no client window
                //so keystrokes are linked to the root window.
                XGrabKeyboard(dpy, root, False,
                        GrabModeAsync, GrabModeAsync, CurrentTime);
                fullscreen=1;
            }
            XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
            if (vi == NULL) {
                std::cout << "\n\tno appropriate visual found\n" << std::endl;
                exit(EXIT_FAILURE);
            } 
            Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
            swa.colormap = cmap;
            swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
              PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                StructureNotifyMask | SubstructureNotifyMask;
            unsigned int winops = CWBorderPixel|CWColormap|CWEventMask;
            if (fullscreen) {
                winops |= CWOverrideRedirect;
                swa.override_redirect = True;
            }
            win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
                    vi->depth, InputOutput, vi->visual, winops, &swa);
            //win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
            //vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
            set_title();
            glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
            glXMakeCurrent(dpy, win, glc);
            show_mouse_cursor(0);
        }
        ~X11_wrapper() {
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
        }
        void set_title() {
            //Set the window title bar.
            XMapWindow(dpy, win);
            XStoreName(dpy, win, "Space Busters");
        }
        void check_resize(XEvent *e) {
            //The ConfigureNotify is sent by the
            //server if the window is resized.
            if (e->type != ConfigureNotify)
                return;
            XConfigureEvent xce = e->xconfigure;
            if (xce.width != gl.xres || xce.height != gl.yres) {
                //Window size did change.
                reshape_window(xce.width, xce.height);
            }
        }
        void reshape_window(int width, int height) {
            //window has been resized.
            setup_screen_res(width, height);
            glViewport(0, 0, (GLint)width, (GLint)height);
            glMatrixMode(GL_PROJECTION); glLoadIdentity();
            glMatrixMode(GL_MODELVIEW); glLoadIdentity();
            glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
            set_title();
        }
        void setup_screen_res(const int w, const int h) {
            gl.xres = w;
            gl.yres = h;
        }
        void swapBuffers() {
            glXSwapBuffers(dpy, win);
        }
        bool getXPending() {
            return XPending(dpy);
        }
        XEvent getXNextEvent() {
            XEvent e;
            XNextEvent(dpy, &e);
            return e;
        }
        void set_mouse_position(int x, int y) {
            XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
        }
        void show_mouse_cursor(const int onoff) {
            printf("show_mouse_cursor(%i)\n", onoff); fflush(stdout);
            if (onoff) {
                //this removes our own blank cursor.
                XUndefineCursor(dpy, win);
                return;
            }
            //vars to make blank cursor
            Pixmap blank;
            XColor dummy;
            char data[1] = {0};
            Cursor cursor;
            //make a blank cursor
            blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
            if (blank == None)
                std::cout << "error: out of memory." << std::endl;
            cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
            XFreePixmap(dpy, blank);
            //this makes the cursor. then set it using this function
            XDefineCursor(dpy, win, cursor);
            //after you do not need the cursor anymore use this function.
            //it will undo the last change done by XDefineCursor
            //(thus do only use ONCE XDefineCursor and then XUndefineCursor):
        }
        Display* getDisplayPointer() const { return dpy; }
        Window getWindowHandle() const { return win; }
} x11(gl.xres, gl.yres);
// ---> for fullscreen x11(0, 0);

//function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();
void drawHealthBar(float health);
void renderMenu();
void drawPauseMenu();
void cleanupTextures();
void* mousePollThread(void* arg);

extern void render_ship_selection();
extern void handle_ship_selection_input();
extern void updateParticles();
extern void renderParticles();
extern void addThrustParticle(float x, float y, float dx, float dy);
//========================================
//
float shipTargetPos[2] = {0.0f, 0.0f};
//
//========================================

//==========================================================================
// M A I N
//==========================================================================
int main()
{
    logOpen();
    init_opengl();
    initEnemies();
    initStar();
    srand(time(NULL));
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);
    x11.set_mouse_position(200, 200);
    x11.show_mouse_cursor(gl.mouse_cursor_on);
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, mousePollThread, NULL);
    int done=0;
    if (!initOpenAL()) {
        std::cerr << "OpenAL failed to initialize.\n";
        return 1;
    }
   playLaserSound();
   sleep(2); 

    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }
        clock_gettime(CLOCK_REALTIME, &timeCurrent);
        timeSpan = timeDiff(&timeStart, &timeCurrent);
        timeCopy(&timeStart, &timeCurrent);
        physicsCountdown += timeSpan;
        while (physicsCountdown >= physicsRate) {
            physics();
            physicsCountdown -= physicsRate;
        }
        
        //theme music
        static bool themeMusic = false;
        if (g.inMenu) {
            if (!themeMusic) {
                playThemeMusic();
                themeMusic = true;
            }
        } else {
            if (themeMusic) {
                stopThemeMusic();
                themeMusic = false;
            }
        }
    
        render();
        if (!introDone) {
            drawIntro();
        } else {
            renderMenu();
        }
        x11.swapBuffers();
    }
    pthread_cancel(thread_id);
    pthread_join(thread_id, NULL);
    cleanup_fonts();
    logClose();
    cleanupTextures();
    shutdownOpenAL();
    return 0;
}

unsigned char *buildAlphaData(Image *img)
{
    //Add 4th component to an RGB stream...
    //RGBA
    //When you do this, OpenGL is able to use the A component to determine
    //transparency information.
    //It is used in this application to erase parts of a texture-map from view.
    int i;
    int a,b,c, clear;
    unsigned char *newdata, *ptr;
    unsigned char *data = (unsigned char *)img->data;
    newdata = (unsigned char *)malloc(img->width * img->height * 4);
    ptr = newdata;

    for (i=0; i<img->width * img->height * 3; i+=3) {
        a = *(data+0);
        b = *(data+1);
        c = *(data+2);
        *(ptr+0) = a;
        *(ptr+1) = b;
        *(ptr+2) = c;

        if (a > 240 && b > 240 && c > 240) {
            clear = 0;  // Fully transparent if white pixels
        } else {
            clear = 255;  // Fully opaque if not
        }

        *(ptr + 3) = clear;

        ptr += 4;
        data += 3;
    }
    return newdata;
}


void init_opengl(void)
{
//OpenGL initialization
glViewport(0, 0, gl.xres, gl.yres);
//Initialize matrices
glMatrixMode(GL_PROJECTION); glLoadIdentity();
glMatrixMode(GL_MODELVIEW); glLoadIdentity();
//This sets 2D mode (no perspective)
glOrtho(0, gl.xres, 0, gl.yres, -1, 1);

glDisable(GL_LIGHTING);
glDisable(GL_DEPTH_TEST);
glDisable(GL_FOG);
glDisable(GL_CULL_FACE);

//Clear the screen to black
glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

//Fonts
glEnable(GL_TEXTURE_2D);
initialize_fonts();

// Load ship textures
const char *shipFiles[3] = {
    "./images/ufo-B.png",
    "./images/ufo-G.png",
    "./images/ufo-R.png"
};
glGenTextures(3, shipTextures);
for (int i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, shipTextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    Image img(shipFiles[i]);
    unsigned char *data = buildAlphaData(&img);  // Adding transparency

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    free(data);
}

// Load other textures
glGenTextures(1, &gl.titleTexture);
glGenTextures(1, &gl.gameOverTexture);

// Load title texture
glBindTexture(GL_TEXTURE_2D, gl.titleTexture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D, 0, 3, img[0].width, img[0].height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, img[0].data);

// Load game over texture
glBindTexture(GL_TEXTURE_2D, gl.gameOverTexture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexImage2D(GL_TEXTURE_2D, 0, 3, img[1].width, img[1].height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, img[1].data);
}

void normalize2d(Vec v)
{
    Flt len = v[0]*v[0] + v[1]*v[1];
    if (len == 0.0f) {
        v[0] = 1.0;
        v[1] = 0.0;
        return;
    }
    len = 1.0f / sqrt(len);
    v[0] *= len;
    v[1] *= len;
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    static int ct=0;

    if (e->type == ButtonPress) {
        if (e->xbutton.button == Button1) {
            gl.keys[Button1] = 1;
        }
    }
    if (e->type == ButtonRelease) {
        if (e->xbutton.button == Button1) {
            gl.keys[Button1] = 0;
        }
    }
    if (e->xbutton.button==3) {
        //Right button is down
    }

    //keys[XK_Up] = 0;
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        // the mouse moved
        int xdiff = savex - e->xbutton.x;
        int ydiff = savey - e->xbutton.y;
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        if (++ct < 10)
            return;		
        // If mouse cursor is on, it does not control the ship.
        // It's a regular mouse.
        if (gl.mouse_cursor_on)
            return;
        //printf("mouse move "); fflush(stdout);
        if (xdiff > 0) {
            g.ship.angle += 0.05f * (float)xdiff;
            if (g.ship.angle >= 360.0f)
                g.ship.angle -= 360.0f;
        }
        else if (xdiff < 0) {
            g.ship.angle += 0.05f * (float)xdiff;
            if (g.ship.angle < 0.0f)
                g.ship.angle += 360.0f;
        }
        if (ydiff > 0) {
            //apply thrust
            //convert ship angle to radians
            Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
            //convert angle to a vector
            Flt xdir = cos(rad);
            Flt ydir = sin(rad);
            g.ship.vel[0] += xdir * (float)ydiff * 0.01f;
            g.ship.vel[1] += ydir * (float)ydiff * 0.01f;
            Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
                    g.ship.vel[1]*g.ship.vel[1]);
            if (speed > 10.0f) {
                speed = 10.0f;
                normalize2d(g.ship.vel);
                g.ship.vel[0] *= speed;
                g.ship.vel[1] *= speed;
            }
            g.mouseThrustOn = true;
            clock_gettime(CLOCK_REALTIME, &g.mouseThrustTimer);
        }
        x11.set_mouse_position(200,200);
        savex = 200;
        savey = 200;
    }

    if (e->type == MotionNotify) {
        mousePos[0] = (float)e->xmotion.x;
        mousePos[1] = (float)(gl.yres - e->xmotion.y);
    }
}

int check_keys(XEvent *e)
{
    static int shift=0;
    if (e->type != KeyRelease && e->type != KeyPress) {
        //not a keyboard event
        return 0;
    }
    int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
    //Log("key: %i\n", key);
    if (e->type == KeyRelease) {
        gl.keys[key] = 0;
        if (key == XK_Shift_L || key == XK_Shift_R)
            shift = 0;
        return 0;
    }
    if (e->type == KeyPress) {
        //std::cout << "press" << std::endl;
        gl.keys[key]=1;
        if (key == XK_Shift_L || key == XK_Shift_R) {
            shift = 1;
            return 0;
        }
    }



    (void)shift;
    switch (key) {
        case XK_Escape:
            //	return 1;
            if (!g.inMenu &&
               (g.gameMode == Game::ENDLESS_MODE ||
                g.gameMode == Game::BOSS_MODE)) 
                g.isPaused = !g.isPaused;
            break;
        case XK_m:
            gl.mouse_cursor_on = !gl.mouse_cursor_on;
            x11.show_mouse_cursor(gl.mouse_cursor_on);
            break;
        case XK_c:
            // credits
            gl.credits = !gl.credits;   // logical not
            break;
        case XK_i:
            // instructions
            gl.instructions = !gl.instructions;
            break;
        case XK_space:
            gl.keys[XK_space] = 1;
            break;
    }
    return 0;
}

void setGameMode(Game::GameMode mode) {
    g.gameMode = mode;
}

void shootBullet() {
    struct timespec bt;
    clock_gettime(CLOCK_REALTIME, &bt);
    double ts = timeDiff(&g.bulletTimer, &bt);

    if (ts > 0.2 && g.nbullets < MAX_BULLETS) {
        timeCopy(&g.bulletTimer, &bt);
        Bullet *b = &g.barr[g.nbullets];
        timeCopy(&b->time, &bt);
        b->pos[0] = g.ship.pos[0];
        b->pos[1] = g.ship.pos[1];

        Vec aimDir;
        aimDir[0] = mousePos[0] - g.ship.pos[0];
        aimDir[1] = mousePos[1] - g.ship.pos[1];

        float length = sqrt(aimDir[0] * aimDir[0] + aimDir[1] * aimDir[1]);
        if (length > 0.0f) {
            aimDir[0] /= length;
            aimDir[1] /= length;
        } else {
            aimDir[0] = 1.0f;
            aimDir[1] = 0.0f;
        }

        b->vel[0] = aimDir[0] * 6.0f + g.ship.vel[0] * 0.5f;
        b->vel[1] = aimDir[1] * 6.0f + g.ship.vel[1] * 0.5f;
        b->color[0] = 1.0f;
        b->color[1] = 1.0f;
        b->color[2] = 1.0f;
        g.nbullets++;
        std::cout << "playing laser sound..\n";
        playLaserSound();

    }
}

// void pollMousePosition(Display *dpy, Window win) {
//     Window root_return, child_return;
//     int root_x, root_y;
//     int win_x, win_y;
//     unsigned int mask_return;

//     Bool result = XQueryPointer(dpy, win,
//                   &root_return, &child_return,
//                   &root_x, &root_y, &win_x, &win_y,
//                   &mask_return);

//     if (!result) {
//         // Cursor not on the same screen, don't update mousePos
//         return;
//     }

//     mousePos[0] = (float)win_x;
//     mousePos[1] = (float)(gl.yres - win_y);
//     //cout << "polling mouse: " << win_x << ", " << win_y << std::endl;
// }

void* mousePollThread(void* arg) {
    (void)arg; // Gets rid of warning for not using arg
    Display *dpy = x11.getDisplayPointer();
    Window win = x11.getWindowHandle();

    while (true) {
        Window root_return, child_return;
        int root_x, root_y;
        int win_x, win_y;
        unsigned int mask_return;

        if (XQueryPointer(dpy, win,
                          &root_return, &child_return,
                          &root_x, &root_y,
                          &win_x, &win_y,
                          &mask_return)) {
            gl.polledMouseX.store((float)win_x);
            gl.polledMouseY.store((float)(gl.yres - win_y));
        }

        usleep(100000); // rate of poll
    }
    return NULL;
}

void physics()
{
    //Flt d0,d1,dist;

    // static struct timespec lastPollTime = {0, 0};
    // struct timespec now;
    // clock_gettime(CLOCK_REALTIME, &now);
    // double dt = timeDiff(&lastPollTime, &now);
    // if (dt > .1) { // rate to poll for mouse position
    //     pollMousePosition(x11.getDisplayPointer(), x11.getWindowHandle());
    //     timeCopy(&lastPollTime, &now);
    // }
    updateCrashAnimation();
    mousePos[0] = gl.polledMouseX.load();
    mousePos[1] = gl.polledMouseY.load();

    if (gl.keys[XK_space] || gl.keys[Button1]) {
        shootBullet();
    }

    //Update bullet positions
    struct timespec bt;
    clock_gettime(CLOCK_REALTIME, &bt);
    int i = 0;
    while (i < g.nbullets) {
        Bullet *b = &g.barr[i];
        //How long has bullet been alive?
        double ts = timeDiff(&b->time, &bt);
        if (ts > 2.5) {
            //time to delete the bullet.
            memcpy(&g.barr[i], &g.barr[g.nbullets-1],
                    sizeof(Bullet));
            g.nbullets--;
            //do not increment i.
            continue;
        }
        //move the bullet
        b->pos[0] += b->vel[0];
        b->pos[1] += b->vel[1];
       
        //Enemy
        if (g.gameMode == Game::ENDLESS_MODE || g.gameMode == Game::BOSS_MODE) {
           hitEnemy(b->pos[0], b->pos[1]);
	   
	   //play enemy explodes sound
	   playEnemyDieSound();
	} 

        // bounce the bullets/ kill the bullet
        if (b->pos[0] < 0.0f) {
            memcpy(&g.barr[i], &g.barr[g.nbullets - 1], sizeof(Bullet));
            g.nbullets--;
            continue;
            //b->pos[0] = 0.0f;
            //b->vel[0] = -b->vel[0];
        } else if (b->pos[0] > (float)gl.xres) {
            memcpy(&g.barr[i], &g.barr[g.nbullets - 1], sizeof(Bullet));
            g.nbullets--;
            continue;
            //b->pos[0] = gl.xres;
            //b->vel[0] = -b->vel[0];
        }

        // bullets bounce off the edges / kill the bullet
        if (b->pos[1] < 0.0f) {
            memcpy(&g.barr[i], &g.barr[g.nbullets - 1], sizeof(Bullet));
            g.nbullets--;
            continue;
            //b->pos[1] = 0.0f;
            //b->vel[1] = -b->vel[1];
        } else if (b->pos[1] > (float)gl.yres) {
            memcpy(&g.barr[i], &g.barr[g.nbullets - 1], sizeof(Bullet));
            g.nbullets--;
            continue;
            //b->pos[1] = gl.yres;
            //b->vel[1] = -b->vel[1];
        }
        ++i;
    }

    if (gl.keys[XK_Up] || gl.keys[XK_w]) {
        g.ship.vel[1] += 0.1f;  // Up
        addThrustParticle(g.ship.pos[0], g.ship.pos[1], -1.0f, 0.0f);
    }
    if (gl.keys[XK_Down] || gl.keys[XK_s]) {
        g.ship.vel[1] -= 0.1f;  // Down
        addThrustParticle(g.ship.pos[0], g.ship.pos[1], -1.0f, 0.0f);
    }
    if (gl.keys[XK_Left] || gl.keys[XK_a]) {
        g.ship.vel[0] -= 0.1f;  // Left
        addThrustParticle(g.ship.pos[0], g.ship.pos[1], -1.0f, 0.0f);
    }
    if (gl.keys[XK_Right] || gl.keys[XK_d]) {
        g.ship.vel[0] += 0.1f;  // Right
        addThrustParticle(g.ship.pos[0], g.ship.pos[1], -1.0f, 0.0f);
    }

    g.ship.pos[0] += g.ship.vel[0];
    g.ship.pos[1] += g.ship.vel[1];

    shipTargetPos[0] = g.ship.pos[0];
    shipTargetPos[1] = g.ship.pos[1];


    // adds some friction
    g.ship.vel[0] *= 0.99f; 
    g.ship.vel[1] *= 0.99f;

    //Enemy movement and collision
    //collision between enemy and player
    if (g.gameMode == Game::ENDLESS_MODE || g.gameMode == Game::BOSS_MODE) {
        moveEnemiesTowardPlayer();
        updateEnemySpawnTimer();
        updateStarSpawnTimer();

        for (int i=0; i < MAX_ENEMIES; i++) {
            if (zorpArmy[i].active) {
                float dx = zorpArmy[i].x - g.ship.pos[0];
                float dy = zorpArmy[i].y - g.ship.pos[1];
                float dist = sqrt(dx * dx + dy * dy);
                    if (dist < 35.0f) {
                        g.ship.takeDamage(0.1f);
                        zorpArmy[i].active = false;
                        printf("hit by Zorp! Health %2f\n", g.ship.health);
                    }
            }
            if (wiblobArmy[i].active) {
                float dx = wiblobArmy[i].x - g.ship.pos[0];
                float dy = wiblobArmy[i].y - g.ship.pos[1];
                float dist = sqrt(dx * dx + dy * dy);
                    if (dist < 35.0f) {
                        g.ship.takeDamage(0.1f);
                        wiblobArmy[i].active = false;
                        printf("hit by Wiblob! Health %2f\n", g.ship.health);
                    }
            }
        }
        
        for( int i=0; i < HealthPack; i++) {
            if(HealObject[i].active) {
                float dx = HealObject[i].x - g.ship.pos[0];
                float dy = HealObject[i].y - g.ship.pos[1];
                float dist = sqrt(dx * dx + dy * dy);
                    if (dist < 35.0f) {
                        g.ship.heal(0.1f);
                        HealObject[i].active = false;
                    }
            }
        }
              
            if (g.ship.health == 0.0f && !crash_animation_active && !g.isEnd && !gameOverReady) {
           
                // Start the crash animation only once
                startCrashAnimation(g.ship.pos[0], g.ship.pos[1]);
           
            } else if (g.ship.health == 0.0f && !g.isEnd && gameOverReady) {
                // After animation is done, transition to Game Over
                g.lastGameMode = static_cast<int>(g.gameMode);
                g.isEnd = true;
                g.inMenu = false;
                gameOverReady = false;
            }

        /*if (g.ship.health == 0.0f && !g.isEnd && isCrashDone()) {
            g.lastGameMode = static_cast<int>(g.gameMode);
            g.isEnd = true;
            g.inMenu = false;
            g.lastGameMode = g.gameMode; 

            startCrashAnimation(g.ship.pos[0], g.ship.pos[1]);
            g.lastGameMode = static_cast<int>(g.gameMode);
            g.isEnd = true;
            g.inMenu = false;

            //renderMenu();
        } else if (g.ship.health == 0.0f && !g.isEnd && !isCrashDone()) {
            startCrashAnimation (g.ship.pos[0], g.ship.pos[1]);
        } */           

}

    //======================================================================
    // keep ship within window
    if (g.ship.pos[0] < 0) g.ship.pos[0] = 0;
    if (g.ship.pos[0] > gl.xres) g.ship.pos[0] = gl.xres;
    if (g.ship.pos[1] < 0) g.ship.pos[1] = 0;
    if (g.ship.pos[1] > gl.yres) g.ship.pos[1] = gl.yres;

    //---------------------------------------------------
    //check keys pressed now
    if (gl.keys[XK_Left]) {
        g.ship.angle += 4.0;
        if (g.ship.angle >= 360.0f)
            g.ship.angle -= 360.0f;
    }
    if (gl.keys[XK_Right]) {
        g.ship.angle -= 4.0;
        if (g.ship.angle < 0.0f)
            g.ship.angle += 360.0f;
    }
    if (gl.keys[XK_Up]) {
        //apply thrust
        //convert ship angle to radians
        Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
        //convert angle to a vector
        Flt xdir = cos(rad);
        Flt ydir = sin(rad);
        g.ship.vel[0] += xdir*0.02f;
        g.ship.vel[1] += ydir*0.02f;
        Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
                g.ship.vel[1]*g.ship.vel[1]);
        if (speed > 10.0f) {
            speed = 10.0f;
            normalize2d(g.ship.vel);
            g.ship.vel[0] *= speed;
            g.ship.vel[1] *= speed;
        }
    }
    // if (gl.keys[XK_space]) {
    // 	//a little time between each bullet
    // 	struct timespec bt;
    // 	clock_gettime(CLOCK_REALTIME, &bt);
    // 	double ts = timeDiff(&g.bulletTimer, &bt);
    // 	if (ts > 0.1) {
    // 		timeCopy(&g.bulletTimer, &bt);
    // 		if (g.nbullets < MAX_BULLETS) {
    // 			//shoot a bullet...
    // 			//Bullet *b = new Bullet;
    // 			Bullet *b = &g.barr[g.nbullets];
    // 			timeCopy(&b->time, &bt);
    // 			b->pos[0] = g.ship.pos[0];
    // 			b->pos[1] = g.ship.pos[1];
    // 			b->vel[0] = g.ship.vel[0];
    // 			b->vel[1] = g.ship.vel[1];
    // 			//convert ship angle to radians
    // 			Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
    // 			//convert angle to a vector
    // 			Flt xdir = cos(rad);
    // 			Flt ydir = sin(rad);
    // 			b->pos[0] += xdir*20.0f;
    // 			b->pos[1] += ydir*20.0f;
    // 			b->vel[0] += xdir*6.0f + rnd()*0.1;
    // 			b->vel[1] += ydir*6.0f + rnd()*0.1;
    // 			b->color[0] = 1.0f;
    // 			b->color[1] = 1.0f;
    // 			b->color[2] = 1.0f;
    // 			g.nbullets++;
    // 		}
    // 	}
    // }
    if (g.mouseThrustOn) {
        //should thrust be turned off
        struct timespec mtt;
        clock_gettime(CLOCK_REALTIME, &mtt);
        double tdif = timeDiff(&mtt, &g.mouseThrustTimer);
        //std::cout << "tdif: " << tdif << std::endl;
        if (tdif < -0.3)
            g.mouseThrustOn = false;
    }

    updateParticles();
}

// -------------------------------------------------------------------
// add restart game logic after health reaches 0
// -------------------------------------------------------------------
void restartGame() {
    g.ship.health = 1.0f;
    g.ship.pos[0] = gl.xres / 2;
    g.ship.pos[1] = gl.yres / 2;
    g.ship.vel[0] = 0.0f;
    g.ship.vel[1] = 0.0f;
    g.inMenu = false;
    g.isPaused = false;
    g.isEnd = false;

    // reset enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        zorpArmy[i].active = false;
        wiblobArmy[i].active = false;
    }

    // reset health packs
    for (int i = 0; i < HealthPack; i++) {
        HealObject[i].active = false;
    }

    // reset score 
    g.score = 0;

    // set game mode to last one played
    g.gameMode = static_cast<Game::GameMode>(g.lastGameMode);
}

//----------------------------------------------------------------
//added function to render menu before game and pause menu during
//----------------------------------------------------------------
void renderMenu() {
    if (g.inMenu) {
        drawMenu(gl.title, gl.xres, gl.yres, gl.titleTexture, g.menuSelection);
        int tmpGameMode = static_cast<int>(g.gameMode);
        handleMainMenuInput(gl.keys, g.menuSelection, g.prevKeys, tmpGameMode,
                g.inMenu);
        g.gameMode = static_cast<Game::GameMode>(tmpGameMode);
        return;
    }
    if (g.isPaused) {
        drawPauseMenu(gl.xres, gl.yres, g.menuSelection);
        int tmpGameMode = static_cast<int>(g.gameMode);
        handlePauseMenuInput(gl.keys, g.menuSelection, g.prevKeys, tmpGameMode, 
                g.inMenu, g.isPaused);
        g.gameMode = static_cast<Game::GameMode>(tmpGameMode);
        return;
    }
    if (g.isEnd) {
        if (!gl.gameOver)
            gl.gameOver = 1;

        drawGameOver(gl.gameOver, gl.xres, gl.yres, gl.gameOverTexture,
                g.menuSelection);
        int tmpGameMode = static_cast<int>(g.gameMode);
        // check if player wants to restart game
        bool restartRequested = false;
        handleGameOverInput(gl.keys, g.menuSelection, g.prevKeys, tmpGameMode,
                g.inMenu, g.isPaused, g.isEnd, gl.gameOver, restartRequested);
        g.gameMode = static_cast<Game::GameMode>(tmpGameMode);
        // if the player selected restart
        if (restartRequested) {
            restartGame();
            return;
        }
        if (g.inMenu) {
            gl.gameOver = 0;
            return;
        }
        return;
    }

    if (g.gameMode == Game::MAIN_MENU) {
        drawMenu(gl.title, gl.xres, gl.yres, gl.titleTexture, g.menuSelection);
        int tmpGameMode = static_cast<int>(g.gameMode);
        handleMainMenuInput(gl.keys, g.menuSelection, g.prevKeys, tmpGameMode, 
                g.inMenu);
        g.gameMode = static_cast<Game::GameMode>(tmpGameMode);
        return;
    } else if (g.gameMode == Game::ENDLESS_MODE) {
        // add calls to functions that spawn certain enemies, etc..
        g.gameMode = Game::ENDLESS_MODE;
        g.inMenu = false;
        g.isEnd = false;
        gl.gameOver = 0;
        render();
        //restartGame();
    } else if (g.gameMode == Game::BOSS_MODE) {
        // add calls to functions that spawns boss enemy, etc..
        g.gameMode = Game::BOSS_MODE;
        g.inMenu = false;
        g.isEnd = false;
        gl.gameOver = 0;
        render();   
        //restartGame();     
    } else if (g.gameMode == Game::SHIP_SELECTION) { 
        static bool firstTime = true;
        g.gameMode = Game::SHIP_SELECTION;
        g.inMenu = false;
        g.isEnd = false;
        g.isPaused = false;
        gl.gameOver = 0;
        if (firstTime) {
            gl.keys[XK_Return] = 0;
            firstTime = false;
        }
        render_ship_selection();
        handle_ship_selection_input();
    }
}


void drawUFO(float x, float y, GLuint texture) {
    float w = 32.0f * 0.7, h = 32.0f * 0.7; //shrank to 70%

    glPushMatrix();
    glTranslatef(x, y, 0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-w, -h);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-w,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( w,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( w, -h);
    glEnd();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

void cleanupTextures() {
    glDeleteTextures(1, &g.ufoTexture);
    glDeleteTextures(3, shipTextures);
}

void render()
{
    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (g.gameMode == Game::ENDLESS_MODE) {
        drawNebulaBackground(); // replaces gl.backgroundTexture
        updateEnemySpawnTimer();
        updateStarSpawnTimer();
        moveEnemiesTowardPlayer();
        renderEnemies();
        renderHeal();
    } else if (g.gameMode == Game::BOSS_MODE) {
        drawNebulaBackground();
        moveBossesTowardPlayer();
        renderBosses();
        renderHeal();
    } else if (gl.background) {
        glBindTexture(GL_TEXTURE_2D, gl.backgroundTexture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(0, gl.yres);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(gl.xres, gl.yres);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(gl.xres, 0);
        glEnd();
    }

    renderParticles();

    drawUFO(g.ship.pos[0], g.ship.pos[1], shipTextures[selectedShip]);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
    glVertex2f(0.0f, 0.0f);
    glEnd();
    glPopMatrix();
    
    glEnable(GL_TEXTURE_2D);


    //------------------------------
    //draws crash animation   
    if (crash_animation_active) {
        drawCrashAnimation();
       // return;
    }

    //-------------------------------------------------------------------------
    //Draw the bullets
    for (int i=0; i<g.nbullets; i++) {
        Bullet *b = &g.barr[i];
        //Log("draw bullet...\n");
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_POINTS);
        glVertex2f(b->pos[0],      b->pos[1]);
        glVertex2f(b->pos[0]-1.0f, b->pos[1]);
        glVertex2f(b->pos[0]+1.0f, b->pos[1]);
        glVertex2f(b->pos[0],      b->pos[1]-1.0f);
        glVertex2f(b->pos[0],      b->pos[1]+1.0f);
        glColor3f(0.8, 0.8, 0.8);
        glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
        glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
        glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
        glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
        glEnd();
    }
    drawHealthBar(gl.xres, g.ship.health);
    drawCrashAnimation();
    screenLeftText(gl.yres);
    screenRightText(gl.xres, gl.yres, g.score); 
    if (gl.credits) {
        r.bot = gl.yres - 55;
        r.left = 10;
        r.center = 0;
        show_credits(&r);
    }

    if (gl.instructions) {
        r.bot = gl.yres - 135;
        r.left = 10;
        r.center = 0;
        show_instructions(&r);
    }

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}



