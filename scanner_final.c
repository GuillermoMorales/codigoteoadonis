/**********************************************************/
/* Scanner-Analizador Lexicografico para el lenguaje PL-0 */
/*                  Febrero de 2007                       */
/*             Uso : scanner progfuente                   */
/* Microsoft Visual C++ versi�n 6.0 Enterprise Edition    */
/**********************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/***************************************/
/*          Globales                   */
/***************************************/

#define MAXLINEA 1000 /*Tama�o maximo de una linea del programa fuente*/
#define MAXPAL 26     /*Numero de palabras reservadas                 */
#define MAXDIGIT 5    /*Maximo numero de digitos en los enteros       */
#define MAXID 10      /*Maxima longitud de los identificadores        */

/*Lexemes de las palabras reservadas para lenguaje Adonis (carrito detector de colores y obstáculos)*/
char *lexpal[MAXPAL] = {"IF",
                        "ELSE",
                        "WHILE",
                        "VAR",
                        "INVOCAR",
                        "FUNCION",
                        "MAIN",
                        "RETURN",
                        "AVANZA",
                        "LIBRE",
                        "COLOR",
                        "ROJO",
                        "AZUL",
                        "VERDE",
                        "DETENER",
                        "ENTERO",
                        "BOOL",
                        "DIRECCION",
                        "VOID",
                        "VERDADERO",
                        "FALSO",
                        "ATRAS",
                        "ADELANTE",
                        "IZQUIERDA",
                        "DERECHA",
                        "DETECTAR"};

char linea[MAXLINEA]; /*Buffer de lineas                    */
int offset, ll;       /*Corrimiento y contador de caracteres*/
int fin_de_archivo;   /*bandera de fin de archivo (obtch)   */
int ch;               /*caracter actual                     */

/*Lexeme del ultimo identificador leido*/
char lex[MAXID + 1]; /* +1 para colocar \0 */

/*Valor numerico del ultimo numero leido*/
long int valor;

FILE *fp; /*Archivo fuente*/

/*Lista de Tokens de pl-0*/
// Debemos poner string?
enum simbolo
{
  nulo,
  ident,
  numero,
  enterotok,
  mas,
  menos,
  por,
  barra,
  igl,
  nig,
  mnr,
  mei,
  myr,
  mai,
  parena,
  parenc,
  coma,
  puntoycoma,
  punto,
  asignacion,
  iftok,
  elsetok,
  whiletok,
  invtok,
  vartok,
  funtok,
  falsotok,
  verdadtok,
  rojotok,
  azultok,
  verdetok,
  atrastok,
  adelantetok,
  izqtok,
  dertok,
  llavedtok,
  llaveizqtok,
  returntok,
  colortok,
  avanzatok,
  detenertok,
  maintok,
  booltok,
  direcciontok,
  detectar_colortok,
  voidtok,
  libretok,
  invocartok
};

enum simbolo token;

/*Tabla de tokens de palabras reservadas*/
enum simbolo tokpal[MAXPAL] = {iftok, elsetok, whiletok, vartok, invocartok, funtok, maintok,
                               returntok, avanzatok, libretok, colortok, rojotok, azultok, verdetok, detenertok,
                               enterotok, booltok, direcciontok, voidtok, verdadtok, falsotok, atrastok, adelantetok,
                               izqtok, dertok, detectar_colortok};

/*Tabla de tokens de operadores y simbolos especiales*/
enum simbolo espec[255];

/*             Prototipos de funciones               */
void init_spec(), obtoken(), imprime_token(), error(int no);
int obtch(), getline(char s[], int lim);

/***************************************/
/*              main                   */
/***************************************/

main(int argc, char *argv[])
{
  FILE *fopen();
  /*Analizar argumentos de main*/
  if (argc != 2)
    printf("\nUso : scanner progama fuente");
  else
  {
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
      printf("\n\n--No se encuentra el Programa Fuente--");
      exit(1);
    }
    printf("\n\nCompilador para Adonis\n\n");
    init_spec();        /*inicializacion de simbolos especiales*/
    ch = ' ';           // ch: caracter actual
    fin_de_archivo = 0; // bandera de fin de archivo
    offset = -1;
    ll = 0; // offset: corrimiento, ll: contador de caracteres
    while (1)
    {                  // ciclo infinito
      obtoken();       // obtiene el siguiente token
      imprime_token(); // imprime el token
    }
  }
  return (0);
}

/***************************************/
/*              obtch                  */
/* Obtiene el siguiente caracter del   */
/* programa fuente.                    */
/***************************************/

int obtch()
{
  if (fin_de_archivo == 1)
  { // si es fin de archivo
    printf("\nFinal del programa");
    exit(1); // termina el programa
  }

  if (offset == ll - 1)
  {                                // si es el ultimo caracter de la linea
    ll = getline(linea, MAXLINEA); // ll: contador de caracteres, getline: lee la siguiente linea del fuente y regresa su tama�o (incluyendo '\n') o 0 si EOF. (eje. para VAR regresa 4)
    if (ll == 0)                   // si es fin de archivo
      fin_de_archivo = 1;          /*se retrasa en un blanco la deteccion de EOF, porque*/
                                   /*obtoken lleva un caracter adelantado. si no, en    */
                                   /*algunos casos, p/ejem, no se reconoceria el punto  */
                                   /*del programa (...end.)                             */
    /*printf("%s",linea);*/        /*Quitar el comentario para obtener listado del fuente*/
    offset = -1;                   // offset: corrimiento, se inicializa en -1 para que obtch regrese el primer caracter de la linea
  }

  ++offset;                                             // aumenta el offset, para que obtch regrese el siguiente caracter de la linea
  if ((linea[offset] == '\0') || (fin_de_archivo == 1)) // si es fin de archivo o es un caracter nulo
    return (' ');                                       // regresa un espacio
  else                                                  // si no es fin de archivo ni es un caracter nulo
    return (toupper(linea[offset]));                    // regresa el caracter en mayuscula
}

/****************************************/
/*              getline                 */
/* lee la siguiente linea del fuente y  */
/* regresa su tama�o (incluyendo '\n') o*/
/* 0 si EOF. (eje. para VAR regresa 4)  */
/* pag 28 de K&R                        */
/****************************************/

int getline(char s[], int lim) // s: buffer de lineas, lim: maximo de caracteres por linea
{
  int c, i;                                                           // c: caracter, i: contador de caracteres
  for (i = 0; i < lim - 1 && (c = getc(fp)) != EOF && c != '\n'; ++i) // c= caracter actual, fp= archivo fuente
    s[i] = c;                                                         // s[i]: caracter i de la linea
  if (c == '\n')
  {           // si es un salto de linea
    s[i] = c; // s[i]: caracter i de la linea
    ++i;
  }
  s[i] = '\0'; // s[i]: caracter i de la linea
  return (i);  // regresa el numero de caracteres de la linea
}

/***************************************/
/*              obtoken                */
/* Obtiene el siguiente token del      */
/* programa fuente.                    */
/***************************************/

void obtoken()
{
  char lexid[MAXID + 1]; /* +1 para colocar \0 */
  int i, j;
  int ok = 0;

  while (ch == ' ' || ch == '\n' || ch == '\t')
    ch = obtch(); /*quitar blancos*/
  /*Si comienza con una letra es identificador o palabra reservada*/
  if (isalpha(ch))
  {
    lexid[0] = ch;
    i = 1;
    while (isalpha((ch = obtch())) || isdigit(ch))
      if (i < MAXID)
        lexid[i++] = ch;
    lexid[i] = '\0';

    /*�es identificador o palabra reservada?*/
    for (j = 0; j < MAXPAL; ++j)
      if (strcmp(lexid, lexpal[j]) == 0)
      {
        ok = 1;
        break;
      }

    if (ok == 1)
      token = tokpal[j];
    else
      token = ident;

    strcpy(lex, lexid);
    /* printf("\nLast Lex %s",lex); */
  }
  else /*si comienza con un digito...*/
    if (isdigit(ch))
    {
      lexid[0] = ch;
      i = j = 1;
      while (isdigit((ch = obtch())))
      {
        if (i < MAXDIGIT)
          lexid[i++] = ch;
        j++;
      }
      lexid[i] = '\0';
      if (j > MAXDIGIT)
        error(30); /*Este numero es demasiado grande*/
      token = numero;
      valor = atol(lexid);
      /*printf("\n numero --> %ld",valor);*/
    }
    else /*simbolos especiales, incluyendo pares de simbolos (Lookahead Symbol) */
      if (ch == '<')
      { // si es menor
        ch = obtch();
        if (ch == '=')
        {
          token = mei;
          ch = obtch();
        }
        else if (ch == '>')
        { // si es diferente
          token = nig;
          ch = obtch();
        }
        else
          token = mnr;
      }
      else if (ch == '>')
      {
        ch = obtch();
        if (ch == '=')
        {
          token = mai;
          ch = obtch();
        }
        else
          token = myr;
      }
      else if (ch == ':')
      {
        ch = obtch();
        if (ch == '=')
        {
          token = asignacion;
          ch = obtch();
        }
        else
          token = nulo;
      }
      else
      {
        token = espec[ch];
        ch = obtch();
      }

  /*printf("\nLexeme %s",lexid);*/ /*Si quieres ver la ultima lexeme*/
}

/***************************************/
/*          imprime_token              */
/* utilitario del programador para     */
/* imprimir un token.                  */
/***************************************/

// Colocar todos los tokens
void imprime_token()
{
  switch (token)
  {
  case nulo: // token caso nulo
    printf("\ntoken-->nulo");
    break;
  case ident: // token caso identificador
    printf("\ntoken-->ident");
    break;
  case numero:
    printf("\ntoken-->numero");
    break;
  case mas:
    printf("\ntoken-->mas");
    break;
  case menos:
    printf("\ntoken-->menos");
    break;
  case por:
    printf("\ntoken-->por");
    break;
  case barra:
    printf("\ntoken-->barra");
    break;
  case igl:
    printf("\ntoken-->igl");
    break;
  case nig:
    printf("\ntoken-->nig");
    break;
  case mnr:
    printf("\ntoken-->mnr");
    break;
  case mei:
    printf("\ntoken-->mei");
    break;
  case myr:
    printf("\ntoken-->myr");
    break;
  case mai:
    printf("\ntoken-->mai");
    break;
  case parena:
    printf("\ntoken-->parena");
    break;
  case parenc:
    printf("\ntoken-->parenc");
    break;
  case coma:
    printf("\ntoken-->coma");
    break;
  case puntoycoma:
    printf("\ntoken-->puntoycoma");
    break;
  case punto:
    printf("\ntoken-->punto");
    break;
  case asignacion:
    printf("\ntoken-->asignacion");
    break;
  case iftok:
    printf("\ntoken-->iftok");
    break;
  case whiletok:
    printf("\ntoken-->whiletok");
    break;
  case vartok:
    printf("\ntoken-->vartok");
    break;
  case llavedtok:
    printf("\ntoken-->llavedtok");
    break;
  case llaveizqtok:
    printf("\ntoken-->llaveizqtok");
    break;
  case maintok:
    printf("\ntoken-->maintok");
    break;
  case returntok:
    printf("\ntoken-->returntok");
    break;
  case rojotok:
    printf("\ntoken-->rojotok");
    break;
  case verdetok:
    printf("\ntoken-->verdetok");
    break;
  case azultok:
    printf("\ntoken-->azultok");
    break;
  case voidtok:
    printf("\ntoken-->voidtok");
    break;
  case enterotok:
    printf("\ntoken-->enterotok");
    break;
  case falsotok:
    printf("\ntoken-->falsotok");
    break;
  case verdadtok:
    printf("\ntoken-->verdaderotok");
    break;
  case adelantetok:
    printf("\ntoken-->adelantetok");
    break;
  case atrastok:
    printf("\ntoken-->atrasotok");
    break;
  case dertok:
    printf("\ntoken-->derechatok");
    break;
  case izqtok:
    printf("\ntoken-->izquierdotok");
    break;
  case direcciontok:
    printf("\ntoken-->direcciontok");
    break;
  case detenertok:
    printf("\ntoken-->detenertok");
    break;
  case avanzatok:
    printf("\ntoken-->avanzartok");
    break;
  case libretok:
    printf("\ntoken-->libretok");
    break;
  case elsetok:
    printf("\ntoken-->elsetok");
    break;
  }
}

/********************************************************************/
/*                            error                                 */
/*                manejador de errores con panico                   */
/*               para un unico error en el scanner                  */
/*                                                                  */
/*           Se marca el error y salida de panico.                  */
/*En algunos casos la marca de error se "barrera" hasta el siguiente*/
/*token, pues se sabe que se tiene el error hasta que se encuentra  */
/*el siguiente token. Ejemplo :                                     */
/* ..End                                                            */
/*   pi = 3 ...                                                     */
/*     ^ Error 5 : Falta un punto y coma                            */
/********************************************************************/

void error(int no)
{
  printf("\n^ Error %d: Este n�mero es demasiado grande", no);
  exit(1);
}

/*****************************************/
/*          init_spec                    */
/* Construccion de la tabla de operadores*/
/* y simboles especiales.                */
/*****************************************/

void init_spec()
{
  int i;
  for (i = 0; i <= 254; ++i) /*inicializacion de lex especiales*/
    espec[i] = nulo;
  // simbolo +
  espec[43] = mas;
  // simbolo -
  espec[45] = menos;
  // simbolo *
  espec[42] = por;
  // simbolo /
  espec[47] = barra;
  // simbolo (
  espec[40] = parena;
  // simbolo )
  espec[41] = parenc;
  // simbolo =
  espec[61] = igl;
  // simbolo .
  espec[46] = punto;
  // simbolo ,
  espec[44] = coma;
  // simbolo ;
  espec[59] = puntoycoma;
  // simbolo <
  espec[60] = mnr;
  // simbolo >
  espec[62] = myr;
  // simbolo {
  espec[123] = llaveizqtok;
  // simbolo }
  espec[125] = llavedtok;
}
