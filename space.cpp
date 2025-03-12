//
//program: space.cpp
//author: team 7
//framework program: asteroids.cpp
//framework author:  Gordon Griesel
//date:    2014 - 2025
//
//------------------------------------------------------------
//changes made:
//
//------------------------------------------------------------
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
        int title;
        int background;
        
        Global() {
            xres = 900;
            yres = 760;
            memset(keys, 0, 65536);
            mouse_cursor_on = 1;
            credits = 0;
            instructions = 0;
            title = 1;
            background = 1;
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

        void takeDamage(int damage) {
            health -= damage;
            if (health < 0)
                health = 0;
        }

        void heal(int amount) {
            health += amount;
            if (health > 1.0)
                health = 1.0;
        }
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

class Asteroid {
    public:
        Vec pos;
        Vec vel;
        int nverts;
        Flt radius;
        Vec vert[8];
        float angle;
        float rotate;
        float color[3];
        struct Asteroid *prev;
        struct Asteroid *next;
    public:
        Asteroid() {
            prev = NULL;
            next = NULL;
        }
};

class Game {
    public:
        int ufo;
        int score;
        GLuint stardustTexture;
        GLuint ufoTexture;
        Ship ship;
        Asteroid *ahead;
        Bullet *barr;
        int showStardust;
        int nasteroids;
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
        bool prevKeys[65536];

    public:
        Game() {
            ufo = 1;
            showStardust = 0;
            ahead = NULL;
            barr = new Bullet[MAX_BULLETS];
            nasteroids = 0;
            nbullets = 0;
            nenemies = 0;
            mouseThrustOn = false;
            inMenu = true;
            gameMode = MAIN_MENU;
            menuSelection = 0;
            isPaused = false;
            score = 0;
            memset(prevKeys, 0, sizeof(prevKeys));

            for (int j=0; j<8; j++) {
                Asteroid *a = new Asteroid;
                a->nverts = 8;
                a->radius = rnd()*80.0 + 40.0;
                Flt angle = 0.0f;
                Flt inc = (PI * 2.0) / (Flt)a->nverts;
                for (int i=0; i<a->nverts; i++) {
                    a->vert[i][0] = sin(angle) * a->radius;
                    a->vert[i][1] = cos(angle) * a->radius;
                    angle += inc;
                }
                a->pos[0] = (Flt)(rand() % gl.xres);
                a->pos[1] = (Flt)(rand() % gl.yres);
                a->pos[2] = 0.0f;
                a->angle = 0.0;
                a->rotate = rnd() * 4.0 - 2.0;
                a->color[0] = 0.8;
                a->color[1] = 0.8;
                a->color[2] = 0.7;
                a->vel[0] = (Flt)(rnd()*2.0-1.0);
                a->vel[1] = (Flt)(rnd()*2.0-1.0);
                a->next = ahead;
                if (ahead != NULL)
                    ahead->prev = a;
                ahead = a;
                ++nasteroids;
            }
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

Image img[4] = { 
    "./images/title1.png",
    "./images/background.png",
    "./images/ufo.png",
    "./images/stardust-health.png"
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
                PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
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
//void drawMenu();
void handleMainMenuInput();
void handlePauseMenuInput();
void drawPauseMenu();
void cleanupTextures();

extern void render_ship_selection();
extern void handle_ship_selection_input();


//==========================================================================
// M A I N
//==========================================================================
int main()
{
    logOpen();
    init_opengl();
    srand(time(NULL));
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);
    x11.set_mouse_position(200, 200);
    x11.show_mouse_cursor(gl.mouse_cursor_on);
    int done=0;
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
        //render();
        renderMenu();
        x11.swapBuffers();
    }
    cleanup_fonts();
    logClose();
    cleanupTextures();
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

        //-----------------------------------------------
        //get largest color component...
        //*(ptr+3) = (unsigned char)((
        //      (int)*(ptr+0) +
        //      (int)*(ptr+1) +
        //      (int)*(ptr+2)) / 3);
        //d = a;
        //if (b >= a && b >= c) d = b;
        //if (c >= a && c >= b) d = c;
        //*(ptr+3) = d;
        //-----------------------------------------------
        //this code optimizes the commented code above.
        //code contributed by student: Chris Smith
        //
        //*(ptr+3) = (a|b|c);
        //-----------------------------------------------
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
glClearColor(0.5f, 0.0f, 1.0f, 1.0f);

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
glGenTextures(1, &gl.backgroundTexture);
glGenTextures(1, &g.ufoTexture);
glGenTextures(1, &g.stardustTexture);

// Load title texture
glBindTexture(GL_TEXTURE_2D, gl.titleTexture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D, 0, 3, img[0].width, img[0].height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, img[0].data);

// Load background texture
glBindTexture(GL_TEXTURE_2D, gl.backgroundTexture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D, 0, 3, img[1].width, img[1].height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, img[1].data);

// Load UFO texture
glBindTexture(GL_TEXTURE_2D, g.ufoTexture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
unsigned char *ufoData = buildAlphaData(&img[2]);  // Adding transparency
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img[2].width, img[2].height, 0,
             GL_RGBA, GL_UNSIGNED_BYTE, ufoData);
free(ufoData);

// Load Stardust texture
glBindTexture(GL_TEXTURE_2D, g.stardustTexture);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D, 0, 3, img[3].width, img[3].height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, img[3].data);
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
    //
    static int ct=0;
    //std::cout << "m" << std::endl << std::flush;
    // if (e->type == ButtonRelease) {
    // 	return;
    // }
    // if (e->type == ButtonPress) {
    // 	if (e->xbutton.button==1) {
    // // 		//Left button is down
    // // 		//a little time between each bullet
    // // 		struct timespec bt;
    // // 		clock_gettime(CLOCK_REALTIME, &bt);
    // // 		double ts = timeDiff(&g.bulletTimer, &bt);
    // // 		if (ts > 0.1) {
    // // 			timeCopy(&g.bulletTimer, &bt);
    // // 			//shoot a bullet...
    // // 			if (g.nbullets < MAX_BULLETS) {
    // // 				Bullet *b = &g.barr[g.nbullets];
    // // 				timeCopy(&b->time, &bt);
    // // 				b->pos[0] = g.ship.pos[0];
    // // 				b->pos[1] = g.ship.pos[1];
    // // 				b->vel[0] = g.ship.vel[0];
    // // 				b->vel[1] = g.ship.vel[1];
    // // 				//convert ship angle to radians
    // // 				Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
    // // 				//convert angle to a vector
    // // 				Flt xdir = cos(rad);
    // // 				Flt ydir = sin(rad);
    // // 				b->pos[0] += xdir*20.0f;
    // // 				b->pos[1] += ydir*20.0f;
    // // 				b->vel[0] += xdir*6.0f + rnd()*0.1;
    // // 				b->vel[1] += ydir*6.0f + rnd()*0.1;
    // // 				b->color[0] = 1.0f;
    // // 				b->color[1] = 1.0f;
    // // 				b->color[2] = 1.0f;
    // // 				++g.nbullets;
    //  			}
    //  		}

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
        //std::cout << "savex: " << savex << std::endl << std::flush;
        //std::cout << "e->xbutton.x: " << e->xbutton.x << std::endl <<
        //std::flush;
        //
        // If mouse cursor is on, it does not control the ship.
        // It's a regular mouse.
        if (gl.mouse_cursor_on)
            return;
        //printf("mouse move "); fflush(stdout);
        if (xdiff > 0) {
            //std::cout << "xdiff: " << xdiff << std::endl << std::flush;
            g.ship.angle += 0.05f * (float)xdiff;
            if (g.ship.angle >= 360.0f)
                g.ship.angle -= 360.0f;
        }
        else if (xdiff < 0) {
            //std::cout << "xdiff: " << xdiff << std::endl << std::flush;
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
            if (!g.inMenu)
                g.isPaused = !g.isPaused;
            //renderMenu();
            break;
        case XK_m:
            gl.mouse_cursor_on = !gl.mouse_cursor_on;
            x11.show_mouse_cursor(gl.mouse_cursor_on);
            break;
        case XK_c:
            // credits
            // =====================================================
            // optimize the following 4 lines
            //if (gl.credits == 0) 
            //    gl.credits = 1;
            //else
            //    gl.credits = 0;
            // =====================================================
            gl.credits = !gl.credits;   // logical not
            // =====================================================
            //gl.credits = gl.credits ^ 1;  // exclusive or
            // ===================================================== 
            //gl.credits = ~gl.credits;   // bitwise not
            // =====================================================
            break;
        case XK_i:
            // instructions
            // -------------------------------------------------------
            // added instructions for space game; has not changed for
            // original asteroids game
            // -------------------------------------------------------
            gl.instructions = !gl.instructions;
            break;
        case XK_space:
            gl.keys[XK_space] = 1;
            break;
    }
    return 0;
}

void deleteAsteroid(Game *g, Asteroid *node)
{
    //Remove a node from doubly-linked list
    //Must look at 4 special cases below.
    if (node->prev == NULL) {
        if (node->next == NULL) {
            //only 1 item in list.
            g->ahead = NULL;
        } else {
            //at beginning of list.
            node->next->prev = NULL;
            g->ahead = node->next;
        }
    } else {
        if (node->next == NULL) {
            //at end of list.
            node->prev->next = NULL;
        } else {
            //in middle of list.
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
}

void setGameMode(Game::GameMode mode) {
    g.gameMode = mode;
}

void buildAsteroidFragment(Asteroid *ta, Asteroid *a)
{
    //build ta from a
    ta->nverts = 8;
    ta->radius = a->radius / 2.0;
    // Flt r2 = ta->radius / 2.0; removed to make circles
    Flt angle = 0.0f;
    Flt inc = (PI * 2.0) / ta->nverts; //(Flt)ta->nverts;

    for (int i=0; i<ta->nverts; i++) {
        ta->vert[i][0] = sin(angle) * ta->radius;
        ta->vert[i][1] = cos(angle) * ta->radius;
        angle += inc;


        //ta->vert[i][0] = sin(angle) * (r2 + rnd() * ta->radius);
        //ta->vert[i][1] = cos(angle) * (r2 + rnd() * ta->radius);
        //angle += inc;
    }
    ta->pos[0] = a->pos[0] + rnd()*10.0-5.0;
    ta->pos[1] = a->pos[1] + rnd()*10.0-5.0;
    //ta->pos[2] = 0.0f;
    ta->angle = 0.0;
    ta->rotate = a->rotate + (rnd() * 4.0 - 2.0);
    ta->color[0] = 0.8;
    ta->color[1] = 0.8;
    ta->color[2] = 0.7;
    ta->vel[0] = a->vel[0] + (rnd()*2.0-1.0);
    ta->vel[1] = a->vel[1] + (rnd()*2.0-1.0);
    //std::cout << "frag" << std::endl;
}

// --------------------------------------------------------------------
// added health bar function 
// --------------------------------------------------------------------
/*void drawHealthBar(float health) 
  {
  if (health > 1.0f) 
  health = 1.0f;
  if (health < 0.0f)
  health = 0.0f;

// position for health bar at bottom right corner
float healthBarWidth = 200.0f;
float healthBarHeight = 15.0f;
float barX = gl.xres - healthBarWidth - 10;     // shows 10px from the right
float barY = 10;    // shows 10px from the bottom

Rect r;
r.bot = barY + healthBarHeight + 5;
r.left = barX;
r.center = 0;
ggprint8b(&r, 16, 0x00ffff00, "HEALTH");

// draw background bar (r, g, b)
glColor3f(0.0f, 0.0f, 0.0f);    // black
glBegin(GL_QUADS);
glVertex2f(barX, barY);
glVertex2f(barX + healthBarWidth, barY);
glVertex2f(barX + healthBarWidth, barY + healthBarHeight);
glVertex2f(barX, barY + healthBarHeight);
glEnd();

// draw red health bar based on current ship health
glColor3f(1.0f, 0.0f, 0.0f);
glBegin(GL_QUADS);
glVertex2f(barX, barY);
glVertex2f(barX + (healthBarWidth * g.ship.health), barY);
glVertex2f(barX + (healthBarWidth * g.ship.health), barY + healthBarHeight);
glVertex2f(barX, barY + healthBarHeight);
glEnd();

// draw black lines to represent increments of health lost
glColor3f(0.0f, 0.0f, 0.0f);
float lineSpacing = healthBarWidth / 10.0f;
for (int i = 1; i < 10; i++) {
float xPos = barX + i * lineSpacing;
glBegin(GL_LINES);
glVertex2f(xPos, barY);
glVertex2f(xPos, barY + healthBarHeight);
glEnd();
}
}*/

void physics()
{
    Flt d0,d1,dist;
    // //Update ship position
    // g.ship.pos[0] += g.ship.vel[0];
    // g.ship.pos[1] += g.ship.vel[1];
    // // Check for collision with window edges and stop the ship at the boundary
    // if (g.ship.pos[0] < 0.0f) {
    //     g.ship.pos[0] = 0.0f;
    //     g.ship.vel[0] = 0.0f;
    // } else if (g.ship.pos[0] > gl.xres) {
    //     g.ship.pos[0] = gl.xres;
    //     g.ship.vel[0] = 0.0f;
    // }

    // if (g.ship.pos[1] < 0.0f) {
    //     g.ship.pos[1] = 0.0f;
    //     g.ship.vel[1] = 0.0f;
    // } else if (g.ship.pos[1] > gl.yres) {
    //     g.ship.pos[1] = gl.yres;
    //     g.ship.vel[1] = 0.0f;
    // }	
    // //
    // //

    // Bullet firing

    if (gl.keys[XK_space]) {  
        struct timespec bt;
        clock_gettime(CLOCK_REALTIME, &bt);
        double ts = timeDiff(&g.bulletTimer, &bt);

        if (ts > 0.01) { // rate of fire
            timeCopy(&g.bulletTimer, &bt);
            if (g.nbullets < MAX_BULLETS) {
                Bullet *b = &g.barr[g.nbullets];
                timeCopy(&b->time, &bt);

                b->pos[0] = g.ship.pos[0];
                b->pos[1] = g.ship.pos[1];

                Vec aimDir;
                aimDir[0] = mousePos[0] - g.ship.pos[0];
                aimDir[1] = mousePos[1] - g.ship.pos[1];

                float length = sqrt(aimDir[0] * aimDir[0] + aimDir[1] * aimDir[1]);
                if (length > 0.0) {
                    aimDir[0] /= length;
                    aimDir[1] /= length;
                } else {
                    aimDir[0] = 1.0;
                    aimDir[1] = 0.0;
                }

                b->vel[0] = aimDir[0] * 6.0 + g.ship.vel[0] * 0.5;
                b->vel[1] = aimDir[1] * 6.0 + g.ship.vel[1] * 0.5;

                b->color[0] = 1.0;
                b->color[1] = 1.0;
                b->color[2] = 1.0;

                ++g.nbullets;
            }
        }
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
        // bouncy bullets
        if (b->pos[0] < 0.0f) {
            b->pos[0] = 0.0f;
            b->vel[0] = -b->vel[0];
        } else if (b->pos[0] > (float)gl.xres) {
            b->pos[0] = gl.xres;
            b->vel[0] = -b->vel[0];
        }

        // bullets bounce off the edges
        if (b->pos[1] < 0.0f) {
            b->pos[1] = 0.0f;
            b->vel[1] = -b->vel[1];
        } else if (b->pos[1] > (float)gl.yres) {
            b->pos[1] = gl.yres;
            b->vel[1] = -b->vel[1];
        }
        ++i;
    }

    if (gl.keys[XK_Up] || gl.keys[XK_w]) {
        g.ship.vel[1] += 0.1f;  // Up
    }
    if (gl.keys[XK_Down] || gl.keys[XK_s]) {
        g.ship.vel[1] -= 0.1f;  // Down
    }
    if (gl.keys[XK_Left] || gl.keys[XK_a]) {
        g.ship.vel[0] -= 0.1f;  // Left
    }
    if (gl.keys[XK_Right] || gl.keys[XK_d]) {
        g.ship.vel[0] += 0.1f;  // Right
    }

    g.ship.pos[0] += g.ship.vel[0];
    g.ship.pos[1] += g.ship.vel[1];

    // adds some friction
    g.ship.vel[0] *= 0.99f; 
    g.ship.vel[1] *= 0.99f;

    // keep ship within window
    if (g.ship.pos[0] < 0) g.ship.pos[0] = 0;
    if (g.ship.pos[0] > gl.xres) g.ship.pos[0] = gl.xres;
    if (g.ship.pos[1] < 0) g.ship.pos[1] = 0;
    if (g.ship.pos[1] > gl.yres) g.ship.pos[1] = gl.yres;


    //Update asteroid positions
    Asteroid *a = g.ahead;
    while (a) {
        a->pos[0] += a->vel[0];
        a->pos[1] += a->vel[1];

        // Bounce off the left/right boundaries
        if (a->pos[0] - a->radius < 0.0f) {
            a->pos[0] = a->radius;     
            a->vel[0] = -a->vel[0];    
        } else if (a->pos[0] + a->radius > gl.xres) {
            a->pos[0] = gl.xres - a->radius; 
            a->vel[0] = -a->vel[0];
        }


        // this is to bounce off the top and bottom edges
        if (a->pos[1] - a->radius < 0.0f) {
            a->pos[1] = a->radius;     
            a->vel[1] = -a->vel[1];   
        } else if (a->pos[1] + a->radius > gl.yres) {
            a->pos[1] = gl.yres - a->radius; 
            a->vel[1] = -a->vel[1];
        }

        a->angle += a->rotate;
        a = a->next;

    }
    //extra comment
    //
    //Asteroid collision with bullets?
    //If collision detected:
    //     1. delete the bullet
    //     2. break the asteroid into pieces
    //        if asteroid small, delete it
    a = g.ahead;
    while (a) {
        //is there a bullet within its radius?
        int i=0;
        while (i < g.nbullets) {
            Bullet *b = &g.barr[i];
            d0 = b->pos[0] - a->pos[0];
            d1 = b->pos[1] - a->pos[1];
            dist = (d0*d0 + d1*d1);
            if (dist < (a->radius*a->radius)) {
                //===================================================
                //tracks score: adds point after shooting an enemy
                //===================================================
                //
                g.score += 5;
                //
                //===================================================
                //std::cout << "asteroid hit." << std::endl;
                //this asteroid is hit.
                if (a->radius > MINIMUM_ASTEROID_SIZE) {
                    //break it into pieces.
                    Asteroid *ta = a;
                    buildAsteroidFragment(ta, a);
                    int r = rand()%10+2;
                    for (int k=0; k<r; k++) {
                        //get the next asteroid position in the array
                        Asteroid *ta = new Asteroid;
                        buildAsteroidFragment(ta, a);
                        //add to front of asteroid linked list
                        ta->next = g.ahead;
                        if (g.ahead != NULL)
                            g.ahead->prev = ta;
                        g.ahead = ta;
                        g.nasteroids++;
                    }
                } else {
                    a->color[0] = 1.0;
                    a->color[1] = 0.1;
                    a->color[2] = 0.1;
                    //asteroid is too small to break up
                    //delete the asteroid and bullet
                    Asteroid *savea = a->next;
                    deleteAsteroid(&g, a);
                    a = savea;
                    g.nasteroids--;
                }
                //delete the bullet...
                memcpy(&g.barr[i], &g.barr[g.nbullets-1], sizeof(Bullet));
                g.nbullets--;
                if (a == NULL)
                    break;
            }
            i++;
        }
        if (a == NULL)
            break;
        a = a->next;
    }
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
}

// -------------------------------------------------------------------
// add updateGame logic to implement health bar functionality
// -------------------------------------------------------------------
void updateGame() {

}

// -------------------------------------------------------------------
// add menu screen to choose endless mode or boss mode before starting
// -------------------------------------------------------------------
void drawMenu() {
    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);
    if (gl.title) {
        glBindTexture(GL_TEXTURE_2D, gl.titleTexture);
        //width and height for title
        int titleWidth = 692;
        int titleHeight = 47;
        int titlexStart = (gl.xres - titleWidth) / 2;
        int titlexEnd = titlexStart + titleWidth;
        int titleyStart = gl.yres - titleHeight - 300;
        int titleyEnd = gl.yres - 300;
        /*glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(0, gl.yres);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(gl.xres, gl.yres);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(gl.xres, 0);
        glEnd();*/
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(titlexStart, titleyStart);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(titlexStart, titleyEnd);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(titlexEnd, titleyEnd);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(titlexEnd, titleyStart);
        glEnd();
    }

    //glClear(GL_COLOR_BUFFER_BIT);
    r.bot = gl.yres / 2;
    r.left = gl.xres / 2 - 100;
    r.center = 0;
    glEnable(GL_TEXTURE_2D);

    // title
    ggprint8b(&r, 24, 0x00ffff00, "Game Menu");

    // options
    ggprint8b(&r, 16, (g.menuSelection == 0) ? 0x00ff0000 : 0x00ffffff, "Endless Mode");
    ggprint8b(&r, 16, (g.menuSelection == 1) ? 0x00ff0000 : 0x00ffffff, "Boss Mode");
    ggprint8b(&r, 16, (g.menuSelection == 2) ? 0x00ff0000 : 0x00ffffff, "Ship Selection");
    ggprint8b(&r, 16, (g.menuSelection == 3) ? 0x00ff0000 : 0x00ffffff, "Exit");

}

//-----------------------------------------------------------------
// add pause menu
//-----------------------------------------------------------------
void drawPauseMenu() {
    // draw "resume", "main menu", and "exit" options
    Rect r;
    r.bot = gl.yres / 2;
    r.left = gl.xres / 2 - 100;
    r.center = 0;

    ggprint8b(&r, 24, 0x00ffff00, "Pause Menu");
    ggprint8b(&r, 16, (g.menuSelection == 0) ? 0x00ff0000 : 0x00ffffff, "Resume");
    ggprint8b(&r, 16, (g.menuSelection == 1) ? 0x00ff0000 : 0x00ffffff, "Return to Main Menu");
    ggprint8b(&r, 16, (g.menuSelection == 2) ? 0x00ff0000 : 0x00ffffff, "Exit");

}

//----------------------------------------------------------------
//added function to render menu before game and pause menu during
//----------------------------------------------------------------
void renderMenu() {
    if (g.inMenu) {
        //extern void drawMenu();
        drawMenu();
        handleMainMenuInput();
        return;
    }
    if (g.isPaused) {
        // pause menu logic
        //extern void drawPauseMenu();
        drawPauseMenu();
        handlePauseMenuInput();
        return;
    }

    if (g.gameMode == Game::MAIN_MENU) {
        drawMenu();
        handleMainMenuInput();
        return;
    } else if (g.gameMode == Game::ENDLESS_MODE) {
        // add calls to functions that spawn certain enemies, etc..
        render();
    } else if (g.gameMode == Game::BOSS_MODE) {
        // add calls to functions that spawns boss enemy, etc..
        render();        
    } else if (g.gameMode == Game::SHIP_SELECTION) { 
        static bool firstTime = true;
        if (firstTime) {
            gl.keys[XK_Return] = 0;
            firstTime = false;
        }
        render_ship_selection();
        handle_ship_selection_input();
    }
}

//---------------------------------------------------------------
//added function to handle main menu choices
//--------------------------------------------------------------
void handleMainMenuInput() {
    // handle menu navigation with keyboard input
    if (gl.keys[XK_Up] && !g.prevKeys[XK_Up]) 
        g.menuSelection = (g.menuSelection == 0) ? 2 : g.menuSelection - 1; 
    if (gl.keys[XK_Down] && !g.prevKeys[XK_Down])
        g.menuSelection = (g.menuSelection == 2) ? 0 : g.menuSelection + 1;
    //g.menuSelection ^= 1;   //XOR to toggle between 0 and 1

    // update prevKeys; prevents multiple presses per key press
    memcpy(g.prevKeys, gl.keys, sizeof(gl.keys));

    // confirm selection with Enter key
    if (gl.keys[XK_Return]) {
        g.inMenu = false;

        // start selected game mode
        switch (g.menuSelection) {
            case 0: // endless mode
                g.gameMode = Game::ENDLESS_MODE;
                break;
            case 1: // boss mode
                g.gameMode = Game::BOSS_MODE;
                break;
            case 2: // ship selection
                g.gameMode = Game::SHIP_SELECTION;
                break;
            case 3: // exit
                exit(0);
                break;
        }
    }
}

// ---------------------------------------------------------
// add function to handle pause menu input
// ---------------------------------------------------------
void handlePauseMenuInput() {
    if (gl.keys[XK_Up] && !g.prevKeys[XK_Up])
        g.menuSelection = (g.menuSelection == 0) ? 2 : g.menuSelection - 1;
    if (gl.keys[XK_Down] && !g.prevKeys[XK_Down]) 
        g.menuSelection = (g.menuSelection == 2) ? 0 : g.menuSelection + 1;

    // update prevKeys; prevents multiple presses per key press
    memcpy(g.prevKeys, gl.keys, sizeof(gl.keys));

    // selecting option
    if (gl.keys[XK_Return]) {
        g.isPaused = false;
        switch (g.menuSelection) {
            case 0: // resume
                g.isPaused = false;
                break;
            case 1: // go to main menu
                g.inMenu = true;
                g.isPaused = false;
                //renderMenu();
                g.gameMode = Game::MAIN_MENU;
                break;
            case 2: // exit game
                exit(0);
                break;
        }
    }
}

void drawUFO(float x, float y, GLuint texture) {
    float w = 32.0f, h = 32.0f;

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

// Draw background
if (gl.background) {
    glBindTexture(GL_TEXTURE_2D, gl.backgroundTexture);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);  
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, gl.yres);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(gl.xres, gl.yres);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(gl.xres, 0);  
    glEnd();
}

// Game info text
r.bot = gl.yres - 20;
r.left = 10;
r.center = 0;
//ggprint8b(&r, 16, 0x00ffff00, "n bullets: %i", g.nbullets);
    //ggprint8b(&r, 16, 0x00ffff00, "n asteroids: %i", g.nasteroids);
    // ----------------------------------------------------------------
    // ggprint8b(&r, 16, 0x00ffff00, "enemies left: %i", g.nenemies);
    // ----------------------------------------------------------------
    ggprint8b(&r, 16, 0x00ffff00, "Press c for Credits");
    ggprint8b(&r, 16, 0x00ffff00, "Press i for Instructions");

    //=================================================================
    //track score: score display
    //=================================================================
    //
    r.bot = gl.yres - 20;
    r.left = gl.xres - 100;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ffffff, "SCORE: %d", g.score);
    //
    //=================================================================
    //positioning reset
    //=================================================================
    r.left = 10;
    r.bot = gl.yres - 60;
    //
    //
    //=================================================================

    if (gl.credits) {
        extern void show_christine(Rect *r);
        extern void show_davalos(Rect *r);
        extern void show_edwin(Rect *r);
        extern void show_bbarrios(Rect *r);
        extern void show_mgarris(Rect *r);
        show_christine(&r);
        show_davalos(&r);
        show_edwin(&r);
        show_bbarrios(&r);
        show_mgarris(&r);
    }

    if (gl.instructions) {
        extern void show_instructions(Rect *r);
        show_instructions(&r);
    }

//-------------------------------------------------------------------------
    //Draw the ship
    /*
	glColor3fv(g.ship.color);
	glPushMatrix();
	glTranslatef(g.ship.pos[0], g.ship.pos[1], g.ship.pos[2]);
	//float angle = atan2(ship.dir[1], ship.dir[0]);
	glRotatef(g.ship.angle, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	//glVertex2f(-10.0f, -10.0f);
	//glVertex2f(  0.0f, 20.0f);
	//glVertex2f( 10.0f, -10.0f);
	glVertex2f(-12.0f, -10.0f);
	glVertex2f(  0.0f,  20.0f);
	glVertex2f(  0.0f,  -6.0f);
	glVertex2f(  0.0f,  -6.0f);
	glVertex2f(  0.0f,  20.0f);
	glVertex2f( 12.0f, -10.0f);
	glEnd();
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	glVertex2f(0.0f, 0.0f);
	glEnd();
	glPopMatrix();
	if (gl.keys[XK_Up] || g.mouseThrustOn || gl.keys[XK_Down]) {
		int i;
		//draw thrust
		Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
		//convert angle to a vector
		Flt xdir = cos(rad);
		Flt ydir = sin(rad);
		Flt xs,ys,xe,ye,r;
		glBegin(GL_LINES);
		for (i=0; i<16; i++) {
			xs = -xdir * 11.0f + rnd() * 4.0 - 2.0;
			ys = -ydir * 11.0f + rnd() * 4.0 - 2.0;
			r = rnd()*40.0+40.0;
			xe = -xdir * r + rnd() * 18.0 - 9.0;
			ye = -ydir * r + rnd() * 18.0 - 9.0;
			glColor3f(rnd()*.3+.7, rnd()*.3+.7, 0);
			glVertex2f(g.ship.pos[0]+xs,g.ship.pos[1]+ys);
			glVertex2f(g.ship.pos[0]+xe,g.ship.pos[1]+ye);
		}
		glEnd();
	}
	*/

	//-------------------------------------------------------------------------
	//Draw the asteroids
	{
		Asteroid *a = g.ahead;
		while (a) {
			//Log("draw asteroid...\n");
			glColor3fv(a->color);
			glPushMatrix();
			glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
			glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			//Log("%i verts\n",a->nverts);
			for (int j=0; j<a->nverts; j++) {
				glVertex2f(a->vert[j][0], a->vert[j][1]);
			}
			glEnd();
			//glBegin(GL_LINES);
			//	glVertex2f(0,   0);
			//	glVertex2f(a->radius, 0);
			//glEnd();
			glPopMatrix();
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_POINTS);
			glVertex2f(a->pos[0], a->pos[1]);
			glEnd();
			a = a->next;
		}
	}

	//-------------------------------------------------------------------------
	//Draw the bullets
	/*
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
	*/

    drawUFO(g.ship.pos[0], g.ship.pos[1], shipTextures[selectedShip]);


    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
    glVertex2f(0.0f, 0.0f);
    glEnd();
    glPopMatrix();
    if (gl.keys[XK_Up] || g.mouseThrustOn || gl.keys[XK_Down]) {
        int i;
        //draw thrust
        Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
        //convert angle to a vector
        Flt xdir = cos(rad);
        Flt ydir = sin(rad);
        Flt xs,ys,xe,ye,r;
        glBegin(GL_LINES);
        for (i=0; i<16; i++) {
            xs = -xdir * 11.0f + rnd() * 4.0 - 2.0;
            ys = -ydir * 11.0f + rnd() * 4.0 - 2.0;
            r = rnd()*40.0+40.0;
            xe = -xdir * r + rnd() * 18.0 - 9.0;
            ye = -ydir * r + rnd() * 18.0 - 9.0;
            glColor3f(rnd()*.3+.7, rnd()*.3+.7, 0);
            glVertex2f(g.ship.pos[0]+xs,g.ship.pos[1]+ys);
            glVertex2f(g.ship.pos[0]+xe,g.ship.pos[1]+ye);
        }
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
    //-------------------------------------------------------------------------
    //Draw the asteroids
    {
        Asteroid *a = g.ahead;
        while (a) {
            //Log("draw asteroid...\n");
            glColor3fv(a->color);
            glPushMatrix();
            glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
            glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
            glBegin(GL_LINE_LOOP);
            //Log("%i verts\n",a->nverts);
            for (int j=0; j<a->nverts; j++) {
                glVertex2f(a->vert[j][0], a->vert[j][1]);
            }
            glEnd();
            //glBegin(GL_LINES);
            //	glVertex2f(0,   0);
            //	glVertex2f(a->radius, 0);
            //glEnd();
            glPopMatrix();
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_POINTS);
            glVertex2f(a->pos[0], a->pos[1]);
            glEnd();
            a = a->next;
        }
    }
    glEnable(GL_TEXTURE_2D);
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
    drawHealthBar(g.ship.health);
    glEnable(GL_TEXTURE_2D);

}



