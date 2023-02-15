#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*variables globales*/

#define MAXLINEA 1000  /*Tamaño maximo de una line del programa fuente*/
#define MAXPAL 26      /*Numero de palabras reservadas del lenguaje*/
#define MAXID 10       /*Maxima longitud de los identificadores*/
#define MAXDIGIT 5     /*Maximo numero de digitos en los enteros*/
#define MAXIT 100      /*Máximo longitud de la tabla de simbolos*/
#define LONG_FECHAD 26 /*Longitud de la fecha a imprimir en el listado*/

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
    numero,
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
    char nombre[MAXID + 1]; // Nombre del identificador
    enum objeto clase;      // Clase del identificador
} registro;

registro tabla[MAXIT]; // Tabala de simbolos (+1 porque tabla[0] no se usa)
int it;                // variable global sobre la tabla de simbolos

/*Prototipo de funciones*/

void init_spec(), obtoken(), error(int no);
int obtch(), getlineC(char s[], int lim);

void bloque(), instruccion(), declaracion(), invocar_expresion(), detener(), expresion(),
    declaracion_ciclo(), declaracion_condicional(), expresion_numerica(), expresion_conjuncion(),
    expresion_relacional(), declaracion_asignacion(), funcion(), mimain(), declaracion_variable(),
    expresion_aritmetica(), termino(), factor(), tipo_dato(), direccion(), parametros(),
    bool(), entero(), string(), dato_color(), dato_libre(), dato_avanzar(), numero(), comparativos(),
    detectar_color(), returnfun();

/* main */

int main(int argc, char *argv[])
{
    FILE *fopen();           // Declaracion de la funcion fopen
    time_t timer;            // Variable para obtener la fecha y hora
    char fecha[LONG_FECHAD]; // Variable para almacenar la fecha y hora
    /*Analizar argumentos de main*/
    if (argc != 2)
    {
        printf("Uso: %s <archivo fuente>\n", argv[0]);
    }
    else
    {
        /*Abrir el archivo fuente*/
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            printf("No se puede abrir el archivo %s");
            exit(1);
        }
        timer = time(NULL);                        // Obtener la fecha y hora
        strcpy(fecha, asctime(localtime(&timer))); // Convertir la fecha y hora a string
        printf("Compilador Parser1 Adonis");
        printf("   %s", fecha);
        init_spec(); // Inicializar la tabla de tokens de operadores y simbolos especiales
        ch = ' ';
        fin_de_archivo = 0;
        offset = -1;
        ll = 0;

        obtoken(); // Obtener el primer token
        it = 0;
        bloque(); // Iniciar el analisis sintactico

        printf("\n\n *** Estadisticas globales ***\n");
        printf("*** No se detectaron errores ***");
        fclose(fp);
    }
    return 0;
}

/****************************************/
/*              obtch                   */
/* Obtener el siguiente caracter del    */
/* Programa fuente.                     */
/****************************************/

int obtch()
{
    if (fin_de_archivo == 1)
    {
        printf("\n El programa fuente esta incompleto");
        exit(1);
    }

    if (offset == ll - 1)
    {
        ll = getlineC(linea, MAXLINEA);
        if (ll == 0)
            fin_de_archivo = 1; /*Se retrasa en un blanco la deteccion de EOF, porque*/
                                /* obtoken lleva un caracter adelantado, si no, en */
                                /* algunos casos, por ejemplo, no se recomienda el punto*/
                                /* del programa (....end.)*/
        printf("%s", linea);
        offset = -1;
    }

    ++offset;
    if ((linea[offset] == '\0') || (fin_de_archivo == 1))
        return (' ');
    else
        return (toupper(linea[offset]));
}

/*******************************************************/
/*              getlineC                               */
/* Obtener la siguiente linea del código fuente y      */
/* regresa su tamaño (incluyendo '\n') o               */
/* 0 si es EOF. (ejemplo para VAR regresa 4)           */
/*******************************************************/

int getlineC(char s[], int lim)
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

/****************************************/
/*              obtoken                 */
/* Obtiene el siguiente token del       */
/* programa fuente                      */

void obtoken()
{
    char lexid[MAXID + 1]; // Variable para almacenar el lexema del identificador
    int i, j;
    int ok = 0;

    while (ch == ' ' || ch == '\t' || ch == '\n')
        ch = obtch(); // Ignorar espacios en blanco
    /*Si comienza con una letra es identificador o palabra reservada*/
    if (isalpha(ch))
    {
        lexid[0] = ch;
        i = 1;
        while (isalpha((ch = obtch())) || isdigit(ch))
        {
            if (i < MAXID)
                lexid[i++] = ch;
        }
        lexid[i] = '\0';

        /*Verificar si es palabra reservada o identificador*/
        for (j = 0; j < MAXPAL; j++)
        {
            if (strcmp(lexid, lexpal[j]) == 0)
            {
                ok = 1;
                break;
            }
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
                if (i < MAXID)
                {
                    lexid[i++] = ch;
                }
                j++;
            }
            lexid[i] = '\0';
            if (j > MAXDIGIT)
            {
                error(30); // Error: Numero demasiado grande
            }
            token = numero;
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
}

/*Manejador de errores en modo pánico, es decir unsolo error*/

void error(int no)
{
    printf("\n^ Error %d:", no);

    printf("\n");
    exit(1);
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

// FUNCIONES ADONIS

// returnfun() va a retornar un token así: return expresion() o return (identificador)
void returnfun()
{
    if (token == returntok)
    {
        obtoken();
        if (token == parena)
        {
            obtoken();
            expresion();
            if (token == parenc)
            {
                obtoken();
            }
            else
            {
                error(7); // Error: Se esperaba un ')'
            }
        }
        else
        {
            expresion();
        }
    }
    else
    {
        error(8); // Error: Se esperaba un 'return'
    }
}

/*colores: rojotok, azultok,verdetok*/
void dato_color()
{
    if (token == rojotok)
    {
        obtoken();
    }
    else if (token == azultok)
    {
        obtoken();
    }
    else if (token == verdetok)
    {
        obtoken();
    }
    else
    {
        error(6); // Error: Se esperaba un color
    }
}

// detectar_color() va a detectar un color así: color dato_color return();
void detectar_color()
{
    if (token == colortok)
    {
        obtoken();
        dato_color();
        returnfun();
    }
    else
    {
        error(9); // Error: Se esperaba un 'color'
    }
}

// comparativos() serán los operadores de comparación: <, >, <=, >=, ==, !=
void comparativos()
{
    if (token == mnr)
    {
        obtoken();
    }
    else if (token == myr)
    {
        obtoken();
    }
    else if (token == mei)
    {
        obtoken();
    }
    else if (token == mai)
    {
        obtoken();
    }
    else if (token == igl)
    {
        obtoken();
    }
    else if (token == dif)
    {
        obtoken();
    }
    else
    {
        error(5); // Error: Se esperaba un operador de comparacion
    }
}

// direccion() serán las direcciones: adelante, atras, derecha, izquierda
void direccion()
{
    if (token == adelantetok)
    {
        obtoken();
    }
    else if (token == atrastok)
    {
        obtoken();
    }
    else if (token == dertok)
    {
        obtoken();
    }
    else if (token == izqtok)
    {
        obtoken();
    }
    else
    {
        error(7); // Error: Se esperaba una direccion
    }
}

// entero() serán todos los números enteros
void entero()
{
    if (token == numero)
    {
        obtoken();
    }
    else
    {
        error(3); // Error: Se esperaba un numero
    }
}

// tipodato() serán los tipos de datos: entero, booleano, color, direccion y void
void tipodato()
{
    if (token == enterotok)
    {
        obtoken();
    }
    else if (token == booltok)
    {
        obtoken();
    }
    else if (token == colortok)
    {
        obtoken();
    }
    else if (token == direcciontok)
    {
        obtoken();
    }
    else if (token == voidtok)
    {
        obtoken();
    }
    else
    {
        error(4); // Error: Se esperaba un tipo de dato
    }
}

void factor()
{
    if (token == ident)
    {
        obtoken();
    }
    else
    {
        if (token == entero)
        {
            obtoken();
        }
        else
        {
            if (token == parena)
            {
                obtoken();
                expresion();
                if (token == parenc)
                {
                    obtoken();
                }
                else
                {
                    error(1); // Error: Se esperaba un ')'
                }
            }
            else
            {
                error(2); // Error: Se esperaba un identificador, un numero o un '('
            }
        }
    }
}

void termino()
{
    factor();
    while (token == por || token == barra)
    {
        obtoken();
        factor();
    }
}

void expresion_aritmetica()
{
    if (token == mas || token == menos)
    {
        obtoken();
    }
    termino();
    while (token == mas || token == menos)
    {
        obtoken();
        termino();
    }
}

void expresion_conjuncion()
{
    expresion_aritmetica();
    while (token == andtok)
    {
        obtoken();
        expresion_aritmetica();
    }
}

void expresion_numerica()
{
    expresion_conjuncion();
    while (token == ortok)
    {
        obtoken();
        expresion_conjuncion();
    }
}

// expresion_relacional()
void expresion_relacional()
{
    expresion_aritmetica();
    comparativos();
    expresion_aritmetica();
}

// expresion() tendrá de opciones bool(), expresion_numerica(), dato_color()
void expresion()
{
    if (token == verdadtok || token == falsotok)
    {
        bool();
    }
    else if (token == ident || token == entero || token == parena)
    {
        expresion_numerica();
    }
    else if (token == colortok)
    {
        dato_color();
    }
    else
    {
        error(10); // Error: Se esperaba un valor booleano, un numero, un color o un '('
    }
}

// invocar_expresion() tiene de sintaxis invocar ident (argumentos())
void invocar_expresion()
{
    if (token == invocartok)
    {
        obtoken();
        if (token == ident)
        {
            obtoken();
            if (token == parena)
            {
                obtoken();
                argumentos();
                if (token == parenc)
                {
                    obtoken();
                }
                else
                {
                    error(1); // Error: Se esperaba un ')'
                }
            }
            else
            {
                error(12); // Error: Se esperaba un '('
            }
        }
        else
        {
            error(11); // Error: Se esperaba un identificador
        }
    }
    else
    {
        error(13); // Error: Se esperaba un 'invocar'
    }
}

// expresion_condicional()

// delaracion_asignacion() tiene de sintaxis ident = expresion();
void declaracion_asignacion()
{
    if (token == ident)
    {
        obtoken();
        if (token == igl)
        {
            obtoken();
            expresion();
        }
        else
        {
            error(8); // Error: Se esperaba un '='
        }
    }
    else
    {
        error(11); // Error: Se esperaba un identificador
    }
}

// declaracion_ciclo() tiene de sintaxis while (expresion()) {bloque()}
void declaracion_ciclo()
{
    if (token == whiletok)
    {
        obtoken();
        if (token == parena)
        {
            obtoken();
            expresion();
            if (token == parenc)
            {
                obtoken();
                if (token == llaveizqtok)
                {
                    obtoken();
                    bloque();
                    if (token == llavedtok)
                    {
                        obtoken();
                    }
                    else
                    {
                        error(9); // Error: Se esperaba un '}'
                    }
                }
                else
                {
                    error(12); // Error: Se esperaba un '{'
                }
            }
            else
            {
                error(1); // Error: Se esperaba un ')'
            }
        }
        else
        {
            error(13); // Error: Se esperaba un '('
        }
    }
    else
    {
        error(14); // Error: Se esperaba un 'while'
    }
}

// declaracion_condicional() tiene de sintaxis if (expresion()) {bloque()} o if (expresion()) else {bloque()}
void declaracion_condicional()
{
    if (token == iftok)
    {
        obtoken();
        if (token == parena)
        {
            obtoken();
            expresion();
            if (token == parenc)
            {
                obtoken();
                if (token == llaveizqtok)
                {
                    obtoken();
                    bloque();
                    if (token == llavedtok)
                    {
                        obtoken();
                        if (token == elsetok)
                        {
                            obtoken();
                            if (token == llaveizqtok)
                            {
                                obtoken();
                                bloque();
                                if (token == llavedtok)
                                {
                                    obtoken();
                                }
                                else
                                {
                                    error(9); // Error: Se esperaba un '}'
                                }
                            }
                            else
                            {
                                error(12); // Error: Se esperaba un '{'
                            }
                        }
                    }
                    else
                    {
                        error(9); // Error: Se esperaba un '}'
                    }
                }
                else
                {
                    error(12); // Error: Se esperaba un '{'
                }
            }
            else
            {
                error(1); // Error: Se esperaba un ')'
            }
        }
        else
        {
            error(13); // Error: Se esperaba un '('
        }
    }
    else
    {
        error(15); // Error: Se esperaba un 'if'
    }
}

// declaracion_variable() puede ser: var ident: tipo_dato; o var ident: tipo_dato, declaracion_variable() o
// var ident: tipo_dato = expresion(); o var ident: tipo_dato = expresion(), declaracion_variable()
void declaracion_variable()
{
    if (token == vartok)
    {
        obtoken();
        if (token == ident)
        {
            obtoken();
            if (token == dospuntos)
            {
                obtoken();
                tipo_dato();
                if (token == puntoycoma)
                {
                    obtoken();
                    if (token == ident || token == vartok)
                    {
                        declaracion_variable();
                    }
                }
                else if (token == igl)
                {
                    obtoken();
                    expresion();
                    if (token == puntoycoma)
                    {
                        obtoken();
                        if (token == ident || token == vartok)
                        {
                            declaracion_variable();
                        }
                    }
                    else
                    {
                        error(16); // Error: Se esperaba un ';'
                    }
                }
                else
                {
                    error(17); // Error: Se esperaba un ';' o un '='
                }
            }
            else
            {
                error(18); // Error: Se esperaba un ':'
            }
        }
        else
        {
            error(11); // Error: Se esperaba un identificador
        }
    }
    else
    {
        error(19); // Error: Se esperaba un 'var'
    }
}

// instruccion() puede ser: ident o declaracion_asignacion() o declaracion_ciclo() o declaracion_condicional() o
// returnfun() o declaracion_variable()
void instruccion()
{
    if (token == ident)
    {
        obtoken();
    }
    else if (token == vartok)
    {
        declaracion_variable();
    }
    else if (token == iftok)
    {
        declaracion_condicional();
    }
    else if (token == whiletok)
    {
        declaracion_ciclo();
    }
    else if (token == returntok)
    {
        returnfun();
    }
    else
    {
        declaracion_asignacion();
    }
}

// argumentos() tiene de sintaxis ident: tipo_dato o ident: tipo_dato, argumentos()
void argumentos()
{
    if (token == ident)
    {
        obtoken();
        if (token == dospuntos)
        {
            obtoken();
            tipo_dato();
            if (token == coma)
            {
                obtoken();
                argumentos();
            }
        }
        else
        {
            error(18); // Error: Se esperaba un ':'
        }
    }
    else
    {
        error(11); // Error: Se esperaba un identificador
    }
}

// parametros() tiene de sintaxis tipo_dato ident o tipo_dato ident, parametros()
void parametros()
{
    tipo_dato();
    if (token == ident)
    {
        obtoken();
        if (token == coma)
        {
            obtoken();
            parametros();
        }
    }
    else
    {
        error(11); // Error: Se esperaba un identificador
    }
}

// funcion() tiene la estructura de funcion ident (parametros()) : tipo_dato {bloque()}
void funcion()
{
    if (token == funtok)
    {
        obtoken();
        if (token == ident)
        {
            obtoken();
            if (token == parena)
            {
                obtoken();
                if (token == ident || token == entero || token == verdadtok || token == falsotok)
                {
                    parametros();
                }
                if (token == parenc)
                {
                    obtoken();
                    if (token == dospuntos)
                    {
                        obtoken();
                        tipo_dato();
                        if (token == llaveizqtok)
                        {
                            obtoken();
                            bloque();
                            if (token == llavedtok)
                            {
                                obtoken();
                            }
                            else
                            {
                                error(9); // Error: Se esperaba un '}'
                            }
                        }
                        else
                        {
                            error(12); // Error: Se esperaba un '{'
                        }
                    }
                    else
                    {
                        error(18); // Error: Se esperaba un ':'
                    }
                }
                else
                {
                    error(1); // Error: Se esperaba un ')'
                }
            }
            else
            {
                error(13); // Error: Se esperaba un '('
            }
        }
        else
        {
            error(11); // Error: Se esperaba un identificador
        }
    }
    else
    {
        error(20); // Error: Se esperaba un 'funcion'
    }
}

// dato_libre() tiene de sintaxis libre (direccion() returnfun());
void dato_libre()
{
    if (token == parena)
    {
        obtoken();
        direccion();
        returnfun();
        if (token == parenc)
        {
            obtoken();
        }
        else
        {
            error(1); // Error: Se esperaba un ')'
        }
    }
    else
    {
        error(13); // Error: Se esperaba un '('
    }
}

// dato_avanzar tiene de sintaxis avanza(direccion(), entero());
void dato_avanzar()
{
    if (token == parena)
    {
        obtoken();
        direccion();
        if (token == coma)
        {
            obtoken();
            entero();
            if (token == parenc)
            {
                obtoken();
            }
            else
            {
                error(1); // Error: Se esperaba un ')'
            }
        }
        else
        {
            error(14); // Error: Se esperaba una ','
        }
    }
    else
    {
        error(13); // Error: Se esperaba un '('
    }
}

// funcion_predeterminada() puede ser detectar_color() o dato_libre() o dato_avanzar() o detener
void funcion_predeterminada()
{
    if (token == detectar_colortok)
    {
        detectar_color();
    }
    else if (token == detenertok)
    {
        obtoken();
        if (token == parena)
        {
            obtoken();
            if (token == parenc)
            {
                obtoken();
            }
            else
            {
                error(1); // Error: Se esperaba un ')'
            }
        }
        else
        {
            error(13); // Error: Se esperaba un '('
        }
    }
    else if (token == avanzatok)
    {
        dato_avanzar();
    }
    else if (token == libretok)
    {
        dato_libre();
    }
    else
    {
        error(21); // Error: Se esperaba una funcion predeterminada
    }
}

void bool()
{
    if (token == verdadtok || token == falsotok)
    {
        obtoken();
    }
    else
    {
        if (token == ident)
        {
            obtoken();
        }
        else
        {
            if (token == parena)
            {
                obtoken();
                expresion();
                if (token == parenc)
                {
                    obtoken();
                }
                else
                {
                    error(1); // Error: Se esperaba un ')'
                }
            }
            else
            {
                error(2); // Error: Se esperaba un identificador, un numero o un '('
            }
        }
    }
}

void condicion()
{
    if (token == ident || token == entero)
    {
        obtoken();
        if (token == mnr || token == myr || token == mei || token == mai || token == dif || token == igl)
        {
            obtoken();
            if (token == ident || token == entero)
            {
                obtoken();
            }
            else
            {
                error(3); // Error: Se esperaba un identificador o un numero
            }
        }
        else
        {
            error(4); // Error: Se esperaba un operador relacional
        }
    }
    else
    {
        error(5); // Error: Se esperaba un identificador o un numero
    }
}

// bloque() puede ser instruccion() o instruccion() return
void bloque()
{
    instruccion();
    if (token == returntok)
    {
        obtoken();
    }
    else
    {
        bloque();
    }
}

// funcion inicial mimain() tiene la sintaxis de funtok main() {bloque()}
void mimain()
{
    if (token == funtok)
    {
        obtoken();
        if (token == maintok)
        {
            obtoken();
            if (token == parena)
            {
                obtoken();
                if (token == parenc)
                {
                    obtoken();
                    if (token == llaveizqtok)
                    {
                        obtoken();
                        bloque();
                        if (token == llavedtok)
                        {
                            obtoken();
                        }
                        else
                        {
                            error(9); // Error: Se esperaba un '}'
                        }
                    }
                    else
                    {
                        error(12); // Error: Se esperaba un '{'
                    }
                }
                else
                {
                    error(1); // Error: Se esperaba un ')'
                }
            }
            else
            {
                error(13); // Error: Se esperaba un '('
            }
        }
        else
        {
            error(22); // Error: Se esperaba un 'main'
        }
    }
    else
    {
        error(20); // Error: Se esperaba un 'funcion'
    }
}
// FIN FUNCIONES ADONIS
