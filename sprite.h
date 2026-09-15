// SPRITE: Motor grafico para realizar juegos en 2D basados en poner sprites de diferentes dimensiones
// en la pantalla mediante X11

// Includes

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Constantes

#define PALDIM 4 // numero de colores guardados en una paleta


// Tipos

struct sprite_s;

typedef struct sprite_s* sprite_t;

typedef unsigned long color_t;

typedef color_t palette_t[PALDIM];

// Variables

extern color_t WHITE;
extern color_t BLACK;

// Funciones

// Screen

int scr_ini(uint16_t w,uint16_t h);
//crea una nueva ventana de dimensiones en puntos wxh

void scr_end();
//libera el espacio de la ventana

void scr_fls();
//se actualiza la pantalla

void scr_clr();
//limpia la pantalla (necesita flush para actualizar)

void scr_bkg(color_t color);
//define el color del fondo (necesita flush para actualizar)

// Color

color_t col_new(uint8_t r,uint8_t g,uint8_t b);
//creacion de color

// Teclado

int key_lis();
//se escucha si hay algun fenomeno de entrada de teclado y se guarda en la matriz de teclado
//solo se registran teclas de la a a la z

int key_in(key_t k);
//dice si se ha pulsado alguna letra entre 'a' y 'z'

// Sprite

sprite_t spr_new(uint8_t siz);
//se define el sprite con espacio para el numero de pixels que se introduciran

void spr_del(sprite_t* spr);
//se libera el espacio del sprite

int spr_ins(sprite_t spr,uint8_t x,uint8_t y,uint8_t c);
//introducimos un pixel en el sprite, los pixels se ordenan de extremo superior izquierdo a
//extremo inferior derecho

int spr_drw(sprite_t spr,palette_t pal,int x,int y,uint16_t pix_dim);
//se dibuja un sprite en la posicion x,y considerando la paleta pal y con un pixel de dimension pixdim

int spr_era(sprite_t spr,int x,int y,uint16_t pix_dim);
//borra los pixels que conforman un sprite colocando el color del fondo

sprite_t spr_grd(uint8_t rows,char* data[]);
//definicion de un sprite a partir de una malla donde el espacio es transparente y 
//los numero de 0 a 3 son los que corresponden al color de la paleta
//rows indica el numero de filas que tiene los datos

sprite_t spr_mov(sprite_t spr,char* move);
//se hacen una serie de movimientos estos son los siguientes
//x: simetria respecto el eje de las x
//y: simetria respecto el eje de las y
//r: rotacion en sentido de las agujas del reloj
//se considera siempre el sprite maximo como referencia

int spr_bin(sprite_t spr,uint8_t rows,uint8_t* data,uint8_t code_col);
//añadimos una capa al sprite spr que se saca a partir de los datos en binario asignando un color de code_col

int spr_col(sprite_t sa,int xa,int ya,uint16_t pa,sprite_t sb,int xb,int yb,uint16_t pb);
//colision de dos sprites que ocupan una posicion y con una dimension de pixel dada

// Texto

void txt_ini();

void txt_end();

int txt_drw(char* str,color_t ink,int* x,int y,uint16_t pix_dim);
//escribimos un texto con color ink  en la posicion x,y (la x final devuelve la posicion de la 
//ultima letra, pixdim indica el tamaño del pixel

// Aleatorio

int rnd(int a,int b);
//numero aleatorio entre a y b

// Tiempo

void pause(double time);
//pausa de un determinado periodo de tiempo
