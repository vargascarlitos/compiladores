/*
 *	Analizador Léxico	
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Práctica de Programación Nro. 1
 *	
 *	Descripcion:
 *	Implementa un analizador léxico que reconoce números, identificadores, 
 * 	palabras reservadas, operadores y signos de puntuación para un lenguaje
 * 	con sintaxis tipo JSON.
 *	
 */
/*********** Inclusión de cabecera **************/
#include "anlex.h"


/************* Variables globales **************/

int consumir;			/* 1 indica al analizador lexico que debe devolver
						el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];		// string utilizado para cargar mensajes de error
token t;				// token global para recibir componentes del Analizador Lexico
int ban_error = 0; 

// variables para el analizador lexico

FILE *archivo;			// Fuente json
FILE * output;			// Salida txt
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char id[TAMLEX];		// Utilizado por el analizador lexico
int delantero=-1;		// Utilizado por el analizador lexico
int fin=0;				// Utilizado por el analizador lexico
int numLinea=1;			// Numero de Linea
int espacioTabulador=0;
/**************** Funciones **********************/


// Rutinas del analizador lexico

void error(const char* mensaje)
{
	printf("Lin %d: Error Lexico. %s.\n",numLinea,mensaje);
	fprintf(output,"Lin %d: Error Lexico. %s.\n",numLinea,mensaje);
	ban_error = 1;
	char c;

	while(c!='\n' && c!=EOF){
		c=fgetc(archivo);
	}
}

void sigLex()
{
	int i=0;
	char c=0;
	int acepto=0;
	int estado=0;
	char msg[41];
	entrada e;

	while((c=fgetc(archivo))!=EOF)
	{
		
		if (c=='\t'){
			espacioTabulador = 1;
			break;
		}			
		else if(c==' ')
		{
			espacioTabulador = 2;
			break;
		}

		else if(c=='\n')
		{
			//incrementar el numero de linea
			//que imprima tambien en el archivo la linea
			espacioTabulador = 3;
			numLinea++;
			break;
		}
		else if (isalpha(c))
		{
			//es una palabra reservada
			i=0;
			do{
				id[i]=tolower(c);
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tamaño de buffer");
			}while(isalpha(c));
			id[i]='\0';
			if (c!=EOF) ungetc(c,archivo);
			else c=0;
			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			//Si no encuentra el token, y todo fue aceptado, significa
			//que es un identificador, y lo agrega dentro del token LITERAL_CADENA

			if (t.pe->compLex==-1)
			{
				error("No se reconoce como palabra reservada");

			}
			break;
		}
		else if (c=='"') //LITERAL_CADENA
		{
			
			i=0;
			do{
				id[i]=c;
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tamaño de buffer");
			}while(c!='"' && c!=EOF);
			//en caso de que el ".*" no incluya caracteres especiales
			//while(isalpha(c) || isdigit(c));
			
			if(c!=EOF){
				id[i]=c;

				
				id[i+1]='\0';
			}
			
			if(c!='"'){
				error("No se esperaba EOF");
				
				break; //no agrega en la tabla de simbolos
			}
			else c=0;

			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			//Si no encuentra el token, y todo fue aceptado, significa
			//que es un identificador, y lo agrega dentro del token LITERAL_CADENA
			if (t.pe->compLex==-1)
			{
				
				
				strcpy(e.lexema,id);
				e.compLex=LITERAL_CADENA;
				insertar(e);
				t.pe=buscar(id);
				t.compLex=LITERAL_CADENA;
				

			}
			break;
		}

		else if (isdigit(c))
		{
				//es un numero
				i=0;
				estado=0;
				acepto=0;
				id[i]=c;
				
				while(!acepto)
				{
					switch(estado){
					case 0: //una secuencia netamente de digitos, puede ocurrir . o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=0;
						}
						else if(c=='.'){
							id[++i]=c;
							estado=1;
						}
						else if(tolower(c)=='e'){
							id[++i]=c;
							estado=3;
						}
						else{
							estado=6;
						}
						break;
					
					case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
						c=fgetc(archivo);						
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2; //esto deberia llevar al 1 de vuelta
						}
						else if(c=='.')//este no hace falta para el tp
						{
							i--;
							fseek(archivo,-1,SEEK_CUR);
							estado=6;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 2://la fraccion decimal, pueden seguir los digitos o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(tolower(c)=='e')//puede ser mayuscula anche
						{
							id[++i]=c;
							estado=3;
						}
						else
							estado=6;
						break;
					case 3://una e, puede seguir +, - o una secuencia de digitos
						c=fgetc(archivo);
						if (c=='+' || c=='-')
						{
							id[++i]=c;
							estado=4;
						}
						else if(isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 4://necesariamente debe venir por lo menos un digito
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 5://una secuencia de digitos correspondiente al exponente
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							estado=6;
						}break;
					case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
						if (c!=EOF)
							ungetc(c,archivo);
						else
							c=0;
						id[++i]='\0';
						acepto=1;
						t.pe=buscar(id);
						if (t.pe->compLex==-1)
						{
							strcpy(e.lexema,id);
							e.compLex=LITERAL_NUM;
							insertar(e);
							t.pe=buscar(id);
						}
						t.compLex=LITERAL_NUM;
						break;
					case -1:
						if (c==EOF)
							error("No se esperaba el fin de archivo");
						else
							error(msg);
						exit(1);
					}
				}
			break;
		}
		
		else if (c=='[')
		{
			t.compLex=L_CORCHETE;
			t.pe=buscar("[");
			break;
		}
		else if (c==']')
		{
			t.compLex=R_CORCHETE;
			t.pe=buscar("]");
			break;
		}
		else if (c=='{')
		{
			t.compLex=L_LLAVE;
			t.pe=buscar("{");
			break;
		}
		else if (c=='}')
		{
			t.compLex=R_LLAVE;
			t.pe=buscar("}");
			break;
		}
		else if (c==',')
		{
			t.compLex=COMA;
			t.pe=buscar(",");
			break;
		}
		else if (c==':')
		{
			t.compLex=DOS_PUNTOS;
			t.pe=buscar(":");
			break;
		}

		
	}
	

	if (c==EOF)
	{
		t.compLex=EOF;
		ban_error = 0;
		sprintf(e.lexema,"EOF");
		t.pe=&e;
	}
	
}
char *componentes_tok[]={
		"STRING",
		"NUMBER",
		"L_LLAVE",
		"R_LLAVE",
		"COMA",
		"DOS_PUNTOS",
		"L_CORCHETE",
		"R_CORCHETE",
		"PR_TRUE",
		"PR_FALSE",
		"PR_NULL"
};

int main(int argc,char* args[])
{
	// inicializar analizador lexico

	initTabla();
	initTablaSimbolos();
	
	
	if(argc > 1)
	{
		output = fopen("output.txt", "w");
		if (!(archivo=fopen(args[1],"rt")))
		{
			printf("Archivo no encontrado.\n");
			exit(1);
		}
		
		while (t.compLex!=EOF || ban_error == 1){
			

			sigLex();
			
			
			
			if(espacioTabulador==1){
				printf("\t");
				fprintf(output,"\t");

				espacioTabulador=0;
			
			}
			else if(espacioTabulador==2)
			{
				printf(" ");
				fprintf(output," ");
				espacioTabulador = 0;
			}
			else if(espacioTabulador==3)
			{
				printf("\n");
				fprintf(output,"\n");
				espacioTabulador = 0;
			}
			else
			{
				if(t.compLex!=-1){
					printf("%s",componentes_tok[t.compLex-256]);
					printf(" ");

					fprintf(output,"%s",componentes_tok[t.compLex-256]);
					fprintf(output," ");
				}
				
				
			}
			
		}
		printf("\n");
		fclose(archivo);
		fclose(output);
	}else{
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}

	return 0;
}
