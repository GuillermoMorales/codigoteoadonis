
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "error.h"

/*variables globales*/

#define MAXLINEA 1000  /*Tamaño maximo de una line del programa fuente*/
#define MAXPAL 25      /*Numero de palabras reservadas del lenguaje*/
#define MAXID 10       /*Maxima longitud de los identificadores*/
#define MAXDIGIT 5     /*Maximo numero de digitos en los enteros*/
#define MAXIT 100      /*Máximo longitud de la tabla de simbolos*/
#define LONG_FECHAD 26 /*Longitud de la fecha a imprimir en el listado*/
#define NOTOKENS 51    /*Numero de tokens que se pueden almacenar en la pila*/
#define MAXD 32767     /*Máximo entero en Pl-0*/
#define MANIV 3        /*Máxima profundidad de anidamiento*/
#define MAXIC 200      /*Tamaño del arreglo codigo: instrucciones codigo-p*/

/*Palabras reservadas*/

char *lexpal[MAXPAL] =
    {
        "IF",
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
int ch;               /*caracter leido                      */

/*Lexeme del ultimo identificador leido*/
char lex[MAXID + 1]; /* +1 para colocar \0 */

/*Valor numerico del ultimo numero leido*/
long int valor;

FILE *fp; /*Apuntador al archivo fuente*/

/*Lista de Tokens de pl-0*/

enum simbolo
{
    nulo,
    ident,
    numerotok,
    enterotok,
    mas,
    menos,
    por,
    dif,
    barra,
    igl,
    mnr,
    mei,
    myr,
    mai,
    parena,
    andtok,
    ortok,
    parenc,
    coma,
    puntoycoma,
    punto,
    dospuntos,
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
enum simbolo tokpal[MAXPAL] = {
    iftok, elsetok, whiletok, vartok, invocartok, funtok, maintok,
    returntok, avanzatok, libretok, colortok, rojotok, azultok, verdetok, detenertok,
    enterotok, booltok, direcciontok, voidtok, verdadtok, falsotok, atrastok, adelantetok,
    izqtok, dertok, detectar_colortok};

/*Tabla de tokens de operadores y simbolos especiales*/
enum simbolo espec[255];

/*Definicion de la tabla de simbolos - organizacion y acceso lineal*/

enum objeto // Objetos del lenguaje adonis
{
    VARIABLE,
    FUNCION
};
typedef struct
{
    int nivel;
    int dir;
} nivel_y_direccion;
typedef struct
{
    char nombre[MAXID + 1];
    enum objeto tipo;
    union
    {
        int val;
        nivel_y_direccion nivdir;
    } variante;
} registro;

registro tabla[MAXIT]; // Tabala de simbolos (+1 porque tabla[0] no se usa)
int it;                // variable global sobre la tabla de simbolos

/*        Conjuntos de tokens para manejo de errores         */
/*Tokens iniciales de declaracion, de instruccion y de factor*/
int tokinidecl[NOTOKENS], tokiniinst[NOTOKENS], tokinifact[NOTOKENS];
int no_de_errores;

/*    Instrucciones del ensamblador (codigo-p)    */
enum fcn
{
    LIT,
    OPR,
    CAR,
    ALM,
    LLA,
    INS,
    SAL,
    SAC
};
typedef struct
{
    enum fcn f; /*Mnemotecnico                       */
    int ni;     /*Nivel 0..MAXNIV                    */
    int di;     /*Direccion o desplazamiento 0..32767*/
} ensamblador;
ensamblador codigo[MAXIC]; /*array con las instrucciones de codigo-p*/
int ic;                    /*indice sobre el array codigo       */
int niv;                   /*nivel de anidamiento de los bloques*/

/*Prototipo de funciones*/

void init_spec(), obtoken();
int obtch(), error(int no), getline(char s[], int lim), estadisticas();

/*funciones de parser1

void bloque(), instruccion(), declaracion(), invocar_expresion(), detener(), expresion(),
    declaracion_ciclo(), declaracion_condicional(), expresion_numerica(), expresion_conjuncion(),
    expresion_relacional(), declaracion_asignacion(), funcion(), mimain(), declaracion_variable(),
    expresion_aritmetica(), termino(), factor(), tipo_dato(), direccion(), parametros(),
    bool(), entero(), string(), dato_color(), dato_libre(), dato_avanzar(), numero(), comparativos(),
    detectar_color(), returnfun();
*/

/*funciones nuevas de pl-0*/
void bloque(int toksig[]), instruccion(int toksig[]), declaracion(int toksig[]),
    invocar_expresion(int toksig[]), detener(int toksig[]), expresion(int toksig[]),
    declaracion_ciclo(int toksig[]), declaracion_condicional(int toksig[]),
    expresion_numerica(int toksig[]), expresion_conjuncion(int toksig[]),
    expresion_relacional(int toksig[]), declaracion_asignacion(int toksig[]),
    funcion(int toksig[]), mimain(int toksig[]), declaracion_variable(int toksig[]),
    expresion_aritmetica(int toksig[]), termino(int toksig[]), factor(int toksig[]),
    tipo_dato(int toksig[]), direccion(int toksig[]), parametros(int toksig[]),
    bool(int toksig[]), entero(int toksig[]), string(int toksig[]), dato_color(int toksig[]),
    dato_libre(int toksig[]), dato_avanzar(int toksig[]), numero(int toksig[]),
    comparativos(int toksig[]), detectar_color(int toksig[]), returnfun(int toksig[]),
    test(int conjunto1[], int conjunto2[], int n), init_set(int conjunto[]),
    copia_set(int conjunto1[], int conjunto2[]), union_set(int conjunto1[], int conjunto2[], int conjunto3[]),
    listar_p(), escribe_obp(), gen(enum fcn x, int y, int z), posicion();

// main
main(int argc, char *argv[])
{
    FILE *fopen();
    time_t timer;
    char fecha[LONG_FECHAD];
    int set_arranque[NOTOKENS]; /*conjunto de tokens de arranque*/

    /*Analizar argumentos de main */
    if (argc != 2)
    {
        printf("\nUso : pl0 progfuente");
    }
    else
    {
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            printf("\nNo se encuentra el programa fuente");
            exit(1);
        }
        timer = time(NULL);
        strcpy(fecha, asctime(localtime(&timer)));
        printf("\nCompilador PL0 - %s", fecha);
        printf("%s - %s\n", argv[1], fecha);
        init_spec(); /*Inicializar tabla de simbolos*/
        ch = ' ';
        fin_de_archivo = 0;
        offset = -1;
        ll = 0;
        no_de_errores = 0;

        // Inicializar conjunto de tokens iniciales
        init_set(tokinidecl);
        tokinidecl[vartok] = tokinidecl[funtok] = tokinidecl[maintok] = 1;

        init_set(tokiniinst);
        tokiniinst[iftok] = tokiniinst[whiletok] = tokiniinst[avanzatok] = tokiniinst[libretok] = tokiniinst[colortok] = tokiniinst[rojotok] = tokiniinst[azultok] = tokiniinst[verdetok] = tokiniinst[detenertok] = tokiniinst[atrastok] = tokiniinst[adelantetok] = tokiniinst[izqtok] = tokiniinst[dertok] = tokiniinst[detectar_colortok] = 1;

        init_set(tokinifact);
        tokinifact[ident] = tokinifact[numerotok] = tokinifact[parena] = 1;

        union_set(set_arranque, tokinidecl, tokiniinst);
        set_arranque[punto] = 1;

        obtoken(); // arrancar el scanner

        it = 0;  // Inicializar el indice sobre la tabla de simbolos
        niv = 0; // Inicializar el nivel de anidamiento de los bloques
        ic = 0;  // Inicializar el indice sobre el array codigo

        bloque(set_arranque); // arrancar el parser
        if (token != punto)
        {
            error(9); // El programa debe terminar con un punto
        }
        estadisticas();
        fclose(fp);
        if (no_de_errores == 0)
        {
            listar_p();           // listar el codigo-p o intermedio
            escribe_obp(argv[1]); // escribir el codigo objeto
        }
    }
    return (0);
}

// obtch obtiene el caracter siguiente del programa fuente
int obtch()
{
    if (fin_de_archivo)
    {
        error(32); // programa fuente incompleto
        estadisticas();
        exit(1);
    }

    if (offset == ll - 1)
    {
        ll = getline(linea, MAXLINEA);
        if (ll == 0)
        {
            fin_de_archivo = 1;
            printf("%s", linea);
        }
        offset = -1;
    }

    offset++;
    if ((linea[offset] == '\0') || (fin_de_archivo == 1))
    {
        return (' ');
    }
    else
    {
        return (toupper(linea[offset]));
    }
}

/*getline*/
int getline(char s[], int lim)
{
    int c, i;
    for (i = 0; i < lim - 1 && (c = getc(fp)) != EOF && c != '\n'; ++i)
        s[i] = c;
    if (c == '\n')
    {
        s[i] = c;
        ++i;
    }
    s[i] = '\0';
    return (i);
}

/*obtoken*/
void obtoken()
{
    char lexid[MAXID + 1]; /* +1 para colocar \0 */
    int i, j;
    int ok = 0;

    while (ch == ' ' || ch == '\n' || ch == '\t')
        ch = obtch(); /*quitar blancos*/
    if (isalpha(ch))
    {
        lexid[0] = ch;
        i = 1;
        while (isalpha((ch = obtch())) || isdigit(ch))
            if (i < MAXID)
                lexid[i++] = ch;
        lexid[i] = '\0';

        for (j = 0; j < MAXPAL; ++j)
            if (strcmp(lexid, lexpal[j]) == 0)
            {
                ok = 1;
                break;
            }
        if (ok == 1)
        {
            token = tokpal[j];
        }
        else
        {
            token = ident;
        }
        strcpy(lex, lexid);
    }
    else
    {
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
                error(30);
            token = numerotok;
            valor = atol(lexid);
        }
        else /*Simbolos especales, incluyendo pares de simbolos (lookahead symbol)*/
        {
            if (ch == '<')
            {
                ch = obtch();
                if (ch == '=')
                {
                    token = mei;
                    ch = obtch();
                }
                else
                {
                    token = mnr;
                }
            }
            else if (ch == ">")
            {
                ch = obtch();
                if (ch == '=')
                {
                    token = mai;
                    ch = obtch();
                }
                else
                {
                    token = myr;
                }
            }
            else if (ch == "=")
            {
                ch = obtch();
                if (ch == '=')
                {
                    token = igl;
                    ch = obtch();
                }
                else
                {
                    token = asignacion;
                }
            }
            else if (ch == "!")
            {
                ch = obtch();
                if (ch == '=')
                {
                    token = dif;
                    ch = obtch();
                }
                else
                {
                    error(31); // Error: Se esperaba un '='
                }
            }
            else
            {
                token = espec[ch];
                ch = obtch();
            }
        }
    }

    // estadisticas

    int estadisticas()
    {
        printf("\n\n***   Estadisticas globales   ***\n");
        if (no_de_errores == 0)
            printf("***  No se detectaron errores ***");
        else
        {
            printf("*** %d error(es) detectado(s)  ***\n", no_de_errores);
            printf("*** El codigo objeto no sirve ***");
        }
        return (no_de_errores);
    }

    // error
    int error(int no)
    {
        ++no_de_errores;
        printf("%*s^", offset, " ");
        printf(" Error %d: %s\n", no, mensaje_de_error[no]);
        return (no_de_errores);
    }

    /*init_spec. Construcción de la tabla de operadores y simbolos especiales*/
    void init_spec()
    {
        int i;
        for (i = 0; i < 256; i++)
        {
            espec[i] = nulo;
        }
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
        // simbolo ,
        espec[44] = coma;
        // simbolo ;
        espec[59] = puntoycoma;
        // simbolo {
        espec[123] = llaveizqtok;
        // simbolo }
        espec[125] = llavedtok;
        // simbolo =
        espec[61] = igl;
        // simbolo <
        espec[60] = mnr;
        // simbolo >
        espec[62] = myr;
        // simbolo !
        espec[33] = dif;
        // simbolo .
        espec[46] = punto;
        // simbolo &
        espec[38] = andtok;
        // simbolo |
        espec[124] = ortok;
        // simbolo :
        espec[58] = dospuntos;
    }

    // init_set
    void init_set(int conjunto[])
    {
        int i;
        for (i = 0; i < NOTOKENS; i++)
        {
            conjunto[i] = 0;
        }
    }

    // copia_set
    void copa_set(int conjunto1[], int conjunto2[])
    {
        int i;
        for (i = 0; i < NOTOKENS; i++)
        {
            conjunto1[i] = conjunto2[i];
        }
    }

    // union_set
    void union_set(int conjunto1[], int conjunto2[], int conjunto3[])
    {
        int i;
        copia_set(conjunto1, conjunto2);
        for (i = 0; i < NOTOKENS; i++)
        {
            if (conjunto3[i] == 1)
            {
                conjunto1[i] = 1;
            }
        }
    }

    // test
    void test(int conjunto1[], int conjunto2[], int n)
    {
        int conj_union[NOTOKENS];
        if (conjunto1[token] == 0)
        {
            error(n);
            union_set(conj_union, conjunto1, conjunto2);
            while (conj_union[token] == 0)
            {
                obtoken();
            }
        }
    }

    // bloque(), puede ser instruccion() o instruccion() return
    void bloque(int toksig[])
    {
        int temp, tempniv;
        int setpaso[NOTOKENS];
        int vacio[NOTOKENS];

        int idat;
        int it0;

        init_set(vacio);

        idat = 3;
        it0 = it;

        tabla[it].tipo = FUNCION;
        tabla[it].variante.nivdir - dir = ic;

        gen(SAL, 0, 0);

        if (niv > MAXNIV)
        {
            error(34); /*Anidamiento muy profundo*/
            estadisticas();
            exit(1); /*error fatal*/
        }
        do
        {
            instruccion();
            if (token == returntok)
            {
                obtoken();
            }
            else
            {
                bloque(int toksig[]);
            }
        }while(tokinidecl[token] == 1);

        codigo[tabla[it0].variante.nivdir.dir].di=ic;
        tabla[it0].variante.nivdir.dir=ic;

        gen(INS,0, idat);

        copia_set(setáso, toksig);
        setpas[puntoycoma]=1;
        instruccion(setpaso);

        gen(OPR,0,0);

        copia_set(setpaso,toksig);
        test(setpaso,vacio,8);
    }