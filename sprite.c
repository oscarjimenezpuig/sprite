#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "sprite.h"

// Screen

color_t BLACK=0;
color_t WHITE=0;

static uint16_t scr_w=0;
static uint16_t scr_h=0;
static Display* display=NULL;
static Colormap colormap;
static Window window;
static Pixmap virtual;
static GC graphic;
static color_t background;
static int min_key_code=0;
static int max_key_code=0;

int scr_ini(uint16_t w,uint16_t h) {
	int screenum=0;
	display=XOpenDisplay(0);
	if(display) {
		int screennum=XDefaultScreen(display);
		colormap=XDefaultColormap(display,screenum);
		BLACK=col_new(0,0,0);
		WHITE=col_new(255,255,255);
		window=XCreateSimpleWindow(display,RootWindow(display,screennum),0,0,w,h,0,0,0);
		XWindowAttributes xwa;
		XGetWindowAttributes(display,window,&xwa);
		virtual=XCreatePixmap(display,window,w,h,xwa.depth);
		static XGCValues gv;
		gv.line_width=1;
		gv.foreground=WHITE;
		gv.background=background=BLACK;
		graphic=XCreateGC(display,window,GCForeground|GCBackground|GCLineWidth,&gv);
		XSelectInput(display,window,StructureNotifyMask|KeyPressMask|KeyReleaseMask);
		XMapWindow(display,window);
		XEvent event;
		int mapped=0;
		while(XEventsQueued(display,QueuedAlready) || !mapped) {
			XNextEvent(display,&event);
			mapped=(!mapped)?(event.type==MapNotify):1;
		}
		XDisplayKeycodes(display,&min_key_code,&max_key_code);
		XSetForeground(display,graphic,gv.background);
		XFillRectangle(display,virtual,graphic,0,0,w,h);
		scr_fls();
        scr_w=w;
        scr_h=h;
		return 1;
	}
	return 0;
}

void scr_end() {
	XUnmapWindow(display,window);
	XDestroyWindow(display,window);
	XFreePixmap(display,virtual);
	XFreeColormap(display,colormap);
	XFreeGC(display,graphic);
	XEvent e;
	while(XEventsQueued(display,QueuedAlready)) {
			XNextEvent(display,&e);
	}
	XCloseDisplay(display);
}

void scr_fls() {
   XCopyArea(display,virtual,window,graphic,0,0,scr_w,scr_h,0,0);
	while(XPending(display)==0);
	XFlush(display);
}

void scr_clr() {
    XSetForeground(display,graphic,background);
    XFillRectangle(display,virtual,graphic,0,0,scr_w,scr_h);
    scr_fls();
}

void scr_bkg(color_t c) {
	background=c;
	scr_clr();
}

static void poi_drw(int x,int y,color_t c) {
    //dibuja un punto de color dado
    XSetForeground(display,graphic,c);
    XDrawPoint(display,virtual,graphic,x,y);
}

static void sqr_drw(int x,int y,unsigned int d,color_t c) {
    //dibuja un cuadrado de dimension d de color c
	XSetForeground(display,graphic,c);
	XFillRectangle(display,virtual,graphic,x,y,d,d);
}

// Color

color_t col_new(uint8_t r,uint8_t g,uint8_t b) {
    const uint16_t FAC=257;
	XColor xc;
	xc.flags=DoRed|DoGreen|DoBlue;
	xc.red=FAC*r;
	xc.green=FAC*g;
	xc.blue=FAC*b;
	XAllocColor(display,colormap,&xc);
	return xc.pixel;
}

// Teclado

#define FKEY 'a' //primera de las letras
#define KEYS ('z'-'a'+1) //numero de teclas que pueden ser pulsadas
#define ARKD ((KEYS/8)+((KEYS%8==0)?0:1)) //dimension del array de teclas

static uint8_t keyprs[ARKD];

int key_lis() {
    XEvent ev;
    KeySym ks;
    int ty=0;
    while(XPending(display)>0) {
        XNextEvent(display,&ev);
        ty=(ev.type==KeyPress)?1:(ev.type==KeyRelease)?-1:0;
        if(ty) {
            ks=XLookupKeysym(&ev.xkey,0);
            char k='a'+(ks-XK_a);
            if(k>='a' && k<='z') {
                uint8_t* ptr=keyprs+(k-FKEY)/8;
                uint8_t flg=1<<(k-FKEY)%8;
                if(ty==1) *ptr|=flg;
                else *ptr&=(~flg);
            }
        }
    }
    return ty;
}                 

int key_in(key_t k) {
    if(k>=FKEY && k<FKEY+KEYS) return keyprs[(k-FKEY)/8] & (1<<(k-FKEY)%8);
    else return 0;
}

// Sprite

#define CORMAX 7 //maxima coordenada
#define SPRDIM ((CORMAX+1)*(CORMAX+1)) //numero maximo de pixels que puede tener un sprite

struct pixel_s {
    uint8_t x : 3; //posicion 
    uint8_t y : 3;
    uint8_t c : 2; //color de la paleta de 0 a 3
};

struct sprite_s {
    struct {
        uint8_t xo : 3; //extremo superior izquierdo del sprite
        uint8_t yo : 3;
        uint8_t xf : 3; //extremo inferior derecho del sprite
        uint8_t yf : 3;
        uint8_t siz: 6; //numero de pixeles
    };
    struct pixel_s* pix;
};

sprite_t spr_new(uint8_t s) {
    sprite_t spr=NULL;
    if(s<=SPRDIM && (spr=malloc(sizeof(struct sprite_s)))) {
        spr->pix=malloc(sizeof(struct pixel_s)*s);
        if(spr->pix==NULL) {
            free(spr);
            spr=NULL;
        } else {
            spr->siz=0;
            spr->xo=spr->yo=spr->xf=spr->yf=0;
        }
    }
    return spr;
}

void spr_del(sprite_t* s) {
    if(s && (*s)) {
        free((*s)->pix);
        free(*s);
        *s=NULL;
    }
}

static int pix_cmp(struct pixel_s a,struct pixel_s b) {
    if(a.y>b.y || (a.y==b.y && a.x>b.x)) return 1;
    else if (a.x==b.x && a.y==b.y) return 0;
    else return -1;
}


static int pix_ins(sprite_t s,struct pixel_s p) {
    for(uint8_t pos=0;pos<s->siz;pos++) {
        if(pix_cmp(s->pix[pos],p)==0) return 0;
    }
    if(p.x<s->xo) s->xo=p.x;
    if(p.x>s->xf) s->xf=p.x;
    if(p.y<s->yo) s->yo=p.y;
    if(p.y>s->yf) s->yf=p.y;
    s->pix[s->siz++]=p;
    return 1;
}

int spr_ins(sprite_t s,uint8_t x,uint8_t y,uint8_t c) {
    if(s) {
        struct pixel_s n={x,y,c};
        return pix_ins(s,n);
    }
    return 0;
}


int spr_drw(sprite_t s,palette_t p,int x,int y,uint8_t d) {
    if(s && d) {
        struct pixel_s* pix=s->pix;
        while(pix!=s->pix+s->siz) {
            int dx=x+d*pix->x;
            int dy=y+d*pix->y;
            color_t c=p[pix->c];
            if(d==1) poi_drw(dx,dy,c);
            else sqr_drw(dx,dy,d,c);
            pix++;
        }
        return 1;
    }
    return 0;
}

int spr_era(sprite_t s,int x,int y,uint8_t d) {
    if(s && d) {
        struct pixel_s* pix=s->pix;
        while(pix!=s->pix+s->siz) {
            int dx=x+d*pix->x;
            int dy=y+d*pix->y;
            if(d==1) poi_drw(dx,dy,background);
            else sqr_drw(dx,dy,d,background);
            pix++;
        }
        return 1;
    }
    return 0;
}

sprite_t spr_grd(uint8_t rows,char* data[]) {
    sprite_t r=spr_new(SPRDIM);
    if(r) {
        for(uint8_t y=0;y<rows;y++) {
            char* rdata=data[y];
            char* ptr=rdata;
            while(*ptr!='\0') {
                if(*ptr!=' ') {
                    uint8_t c=*ptr-'0';
                    uint8_t x=ptr-rdata;
                    struct pixel_s n={x,y,c};
                    pix_ins(r,n);
                }
                ptr++;
            }
        }
        void* ptr=NULL;
        if(r->siz!=SPRDIM && (ptr=realloc(r->pix,sizeof(struct pixel_s)*r->siz))) r->pix=ptr;
    }
    return r;
}           

static struct pixel_s pix_six(struct pixel_s p) {
    return (struct pixel_s){p.x,CORMAX-p.y,p.c};
}

static struct pixel_s pix_siy(struct pixel_s p) {
    return (struct pixel_s){CORMAX-p.x,p.y,p.c};
}

static struct pixel_s pix_rot(struct pixel_s p) {
    return (struct pixel_s){CORMAX-p.y,p.x,p.c};
}

sprite_t spr_mov(sprite_t s,char* m) {
    sprite_t r=NULL;
    if(s && m) {
        r=spr_new(s->siz);
        for(uint8_t pos=0;pos<s->siz;pos++) {
            struct pixel_s pa=s->pix[pos];
            char* pm=m;
            while(*pm!='\0') {
                switch(*pm) {
                    case 'x':
                        pa=pix_six(pa);
                        break;
                    case 'y':
                        pa=pix_siy(pa);
                        break;
                    case 'r':
                        pa=pix_rot(pa);
                        break;
                }
                pm++;
            }
            pix_ins(r,pa);
        }
    }
    return r;
}

int spr_bin(sprite_t s,uint8_t r,uint8_t* d,uint8_t c) {
    if(s && r && d) {
        for(uint8_t y=0;y<r;y++) {
            uint8_t val=d[y];
            uint8_t msc=128;
            uint8_t x=0;
            while(msc) {
                if(val & msc) {
                    struct pixel_s p={x,y,c};
                    if(!pix_ins(s,p)) return 0;
                }
                x++;
                msc=msc>>1;
            }
        }
        return 1;
    }
    return 0;
}

#define min(A,B) (((A)<(B))?(A):(B))
#define max(A,B) (((A)>(B))?(A):(B))

int spr_col(sprite_t sa,int xa,int ya,uint8_t pa,sprite_t sb,int xb,int yb,uint8_t pb) {
    int xo=max(xa+pa*sa->xo,xb+pb*sb->xo);
    int yo=max(ya+pa*sa->yo,yb+pb*sb->yo);
    int xf=min(xa+pa*sa->xf,xb+pb*sb->xf);
    int yf=min(ya+pa*sa->yf,yb+pb*sb->yf);
    return (xo<=xf && yo<=yf);
}

#undef min
#undef max

// Texto

#define CHRINF 32 //caracter inferior
#define CHRSUP 126 //caracter superior
#define CHRDIM (CHRSUP-CHRINF+1) //numero de caracteres

static sprite_t ascchr[CHRDIM];

static uint8_t code_font[] = {
    0,0,0,0,0,0,0,0,                 // 32 ' '
    24,24,24,24,24,0,24,0,          // 33 '!'
    54,54,54,0,0,0,0,0,             // 34 '"'
    54,54,127,54,127,54,54,0,       // 35 '#'
    24,62,96,60,6,124,24,0,         // 36 '$'
    98,102,12,24,48,102,70,0,       // 37 '%'
    48,72,48,100,74,68,58,0,        // 38 '&'
    24,24,24,0,0,0,0,0,             // 39 '''
    12,24,48,48,48,24,12,0,         // 40 '('
    48,24,12,12,12,24,48,0,         // 41 ')'
    0,102,60,255,60,102,0,0,        // 42 '*'
    0,24,24,126,24,24,0,0,          // 43 '+'
    0,0,0,0,24,24,48,0,             // 44 ','
    0,0,0,126,0,0,0,0,              // 45 '-'
    0,0,0,0,0,24,24,0,              // 46 '.'
    2,6,12,24,48,96,64,0,           // 47 '/'
    60,102,110,118,102,102,60,0,    // 48 '0'
    24,56,24,24,24,24,60,0,         // 49 '1'
    60,102,6,12,24,48,126,0,        // 50 '2'
    60,102,6,28,6,102,60,0,         // 51 '3'
    12,28,60,108,126,12,12,0,       // 52 '4'
    126,96,124,6,6,102,60,0,        // 53 '5'
    28,48,96,124,102,102,60,0,      // 54 '6'
    126,102,12,24,24,24,24,0,       // 55 '7'
    60,102,102,60,102,102,60,0,     // 56 '8'
    60,102,102,62,6,12,56,0,        // 57 '9'
    0,24,24,0,0,24,24,0,            // 58 ':'
    0,24,24,0,0,24,24,48,           // 59 ';'
    12,24,48,96,48,24,12,0,         // 60 '<'
    0,0,126,0,126,0,0,0,            // 61 '='
    48,24,12,6,12,24,48,0,          // 62 '>'
    60,102,6,12,24,0,24,0,          // 63 '?'
    60,102,110,110,96,102,60,0,     // 64 '@'
    24,60,102,102,126,102,102,0,    // 65 'A'
    124,102,102,124,102,102,124,0,  // 66 'B'
    60,102,96,96,96,102,60,0,       // 67 'C'
    120,108,102,102,102,108,120,0,  // 68 'D'
    126,96,96,124,96,96,126,0,      // 69 'E'
    126,96,96,124,96,96,96,0,       // 70 'F'
    60,102,96,110,102,102,60,0,     // 71 'G'
    102,102,102,126,102,102,102,0,  // 72 'H'
    60,24,24,24,24,24,60,0,         // 73 'I'
    30,12,12,12,12,108,56,0,        // 74 'J'
    102,108,120,112,120,108,102,0,  // 75 'K'
    96,96,96,96,96,96,126,0,        // 76 'L'
    99,119,127,107,99,99,99,0,      // 77 'M'
    102,118,126,126,110,102,102,0,  // 78 'N'
    60,102,102,102,102,102,60,0,    // 79 'O'
    124,102,102,124,96,96,96,0,     // 80 'P'
    60,102,102,102,110,60,14,0,     // 81 'Q'
    124,102,102,124,120,108,102,0,  // 82 'R'
    60,102,96,60,6,102,60,0,        // 83 'S'
    126,24,24,24,24,24,24,0,        // 84 'T'
    102,102,102,102,102,102,60,0,   // 85 'U'
    102,102,102,102,102,60,24,0,    // 86 'V'
    99,99,99,107,127,119,99,0,      // 87 'W'
    102,102,60,24,60,102,102,0,     // 88 'X'
    102,102,60,24,24,24,24,0,       // 89 'Y'
    126,6,12,24,48,96,126,0,        // 90 'Z'
    60,48,48,48,48,48,60,0,         // 91 '['
    64,96,48,24,12,6,2,0,           // 92 '\'
    60,12,12,12,12,12,60,0,         // 93 ']'
    24,60,102,0,0,0,0,0,            // 94 '^'
    0,0,0,0,0,0,126,0,              // 95 '_'
    48,24,12,0,0,0,0,0,             // 96 '`'
    0,0,60,6,62,102,62,0,           // 97 'a'
    96,96,124,102,102,102,124,0,    // 98 'b'
    0,0,60,102,96,102,60,0,         // 99 'c'
    6,6,62,102,102,102,62,0,        // 100 'd'
    0,0,60,102,126,96,60,0,         // 101 'e'
    28,48,48,124,48,48,48,0,        // 102 'f'
    0,0,62,102,102,62,6,124,        // 103 'g'
    96,96,124,102,102,102,102,0,    // 104 'h'
    24,0,56,24,24,24,60,0,          // 105 'i'
    6,0,6,6,6,6,102,60,             // 106 'j'
    96,96,108,120,112,120,108,0,    // 107 'k'
    56,24,24,24,24,24,60,0,         // 108 'l'
    0,0,102,127,127,107,99,0,       // 109 'm'
    0,0,124,102,102,102,102,0,      // 110 'n'
    0,0,60,102,102,102,60,0,        // 111 'o'
    0,0,124,102,102,124,96,96,      // 112 'p'
    0,0,62,102,102,62,6,6,          // 113 'q'
    0,0,124,102,96,96,96,0,         // 114 'r'
    0,0,60,96,60,6,60,0,            // 115 's'
    48,48,124,48,48,48,28,0,        // 116 't'
    0,0,102,102,102,102,62,0,       // 117 'u'
    0,0,102,102,102,60,24,0,        // 118 'v'
    0,0,99,107,127,127,54,0,        // 119 'w'
    0,0,102,60,24,60,102,0,         // 120 'x'
    0,0,102,102,102,62,6,124,       // 121 'y'
    0,0,126,12,24,48,126,0,         // 122 'z'
    14,24,24,112,24,24,14,0,        // 123 '{'
    24,24,24,24,24,24,24,0,         // 124 '|'
    112,24,24,14,24,24,112,0,       // 125 '}'
    0,0,50,74,0,0,0,0               // 126 '~'
};


static void chr_new(uint8_t pos) {
    sprite_t nc=spr_new(SPRDIM);
    uint8_t* data=code_font+(8*pos);
    if(spr_bin(nc,8,data,0)) ascchr[pos]=nc;
}

void txt_ini() {
    for(uint8_t pos=0;pos<CHRDIM;pos++) chr_new(pos);
}

void txt_end() {
    for(uint8_t pos=0;pos<CHRDIM;pos++) {
        spr_del(ascchr+pos);
    }
}

int txt_drw(char* str,color_t ink,int* x,int y,uint8_t pd) {
    int ret=0;
    if(str && x) {
        ret=1;
        int xx=*x;
        palette_t pal={ink};
        char* ptr=str;
        while(*ptr!='\0') {
            uint8_t c=(*ptr>=CHRINF && *ptr<=CHRSUP)?*ptr-CHRINF:32;
            ret&=spr_drw(ascchr[c],pal,xx,y,pd);
            ptr++;
            xx+=(8*pd);
        }
        *x=xx;
    }
    return ret;
}

// Aleatorio

int rnd(int a,int b) {
    static int init=0;
    if(!init) {
        srand(time(NULL));
        init=1;
    }
    int dif=((a>b)?a-b:b-a)+1;
    return (rand()%dif)+((a>b)?b:a);
}

// Tiempo

void pause(double t) {
    clock_t end=clock()+CLOCKS_PER_SEC*t;
    while(clock()<end);
} 
