// **********************************************************************
// PUCRS/Escola PolitŽcnica
// COMPUTA‚ÌO GRçFICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************


// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <ctime>

using namespace std;

int larguraLogica = 192, alturaLogica = 108;
float baseDx = 92, baseDy = 8, rotacaoN1 = 0, rotacaoN2 = 0, rotacaoN3 = 0, rotacaoN4 = 0;

#ifdef WIN32
#include <windows.h>
#include <glut.h>
static DWORD last_idle_time;
#else
#include <sys/time.h>
static struct timeval last_idle_time;
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

// **********************************************************************
//  void animate ( unsigned char key, int x, int y )
//
//
// **********************************************************************
void animate()
{
    static float dt;
    static float AccumTime=0;

#ifdef _WIN32
    DWORD time_now;
    time_now = GetTickCount();
    dt = (float) (time_now - last_idle_time) / 1000.0;
#else
    // Figure out time elapsed since last call to idle function
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
    1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
#endif
    AccumTime +=dt;
    if (AccumTime >=3) // imprime o FPS a cada 3 segundos
    {
        cout << 1.0/dt << " FPS"<< endl;
        AccumTime = 0;
    }
    // cout << "AccumTime: " << AccumTime << endl;
    // Anima cubos
    //AngY++;
    // Salva o tempo para o pr—ximo ciclo de rendering
    last_idle_time = time_now;

    //if  (GetAsyncKeyState(32) & 0x8000) != 0)
    //  cout << "Espaco Pressionado" << endl;

    // Redesenha
    glutPostRedisplay();
}
// **********************************************************************
//  void init(void)
//  Inicializa os parâmetros globais de OpenGL
//
// **********************************************************************
void init(void)
{
	// Define a cor do fundo da tela (BRANCO)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

// **********************************************************************
//  void reshape( int w, int h )
//  trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela área OpenGL dentro da Janela
    glViewport(0, 0, w, h);

    // Define os limites lógicos da área OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0,10,0,10,0,1);
}

void DesenhaObjeto(string diretorio)
{
    int i, j, cor, nCores, linhas, colunas;
    ifstream arquivo;
    string conteudo;

    arquivo.open(diretorio);
    arquivo >> conteudo;
    arquivo >> nCores;
    float cores[nCores][3];

    for (i = 0; i < nCores; i++)
    {
        arquivo >> conteudo;
        for (j = 0; j < 3; j++) arquivo >> cores[i][j];
    }

    arquivo >> conteudo;
    arquivo >> linhas;
    arquivo >> colunas;

    for (i = linhas; i > 0; i--)
    {
        for (j = colunas; j > 0; j--)
        {
            arquivo >> cor;
            cor--;
            //cout << cor << " ";
            //cout << cor << " --> " << cores[cor][0] << " " << cores[cor][1] << " " << cores[cor][2] << " : ";
            glPushMatrix();
            glColor3f(cores[cor][0],cores[cor][1],cores[cor][2]);
            glBegin(GL_QUADS);
                glVertex2f(j-1,i);
                glVertex2f(j,i);
                glVertex2f(j,i-1);
                glVertex2f(j-1,i-1);
            glEnd();
            glPopMatrix();
            //cout << "(" << i << ", " << j << ") ";
        }
        //cout << endl;
    }
    arquivo.close();
}

void DesenhaNivel4Guindaste()
{
    glPushMatrix();
        glTranslatef(3,39,0);
            glTranslatef(1.5,0,0); // Transformação para rotação
            glRotatef(rotacaoN4,0,0,1);
            glTranslatef(-1.5,0,0); // Transformação para rotação
            DesenhaObjeto("gameObjects/nivel4Guindaste.txt");
        glTranslatef(-3,-39,0);
    glPopMatrix();
}

void DesenhaNivel3Guindaste()
{
    glPushMatrix();
        glTranslatef(3,25,0);
            glTranslatef(2.5,0,0);
            glRotatef(rotacaoN3,0,0,1);
            glTranslatef(-2.5,0,0);
            DesenhaObjeto("gameObjects/nivel3Guindaste.txt");
        glTranslatef(-3,-25,0);
        DesenhaNivel4Guindaste();
    glPopMatrix();
}

void DesenhaNivel2Guindaste()
{
    glPushMatrix();
        glTranslatef(2,17,0);
            glTranslatef(3.5,0,0);
            glRotatef(rotacaoN2,0,0,1);
            glTranslatef(-3.5,0,0);
            DesenhaObjeto("gameObjects/nivel2Guindaste.txt");
        glTranslatef(-2,-17,0);
        DesenhaNivel3Guindaste();
    glPopMatrix();
}

void DesenhaNivel1Guindaste()
{
    glPushMatrix();
        glTranslatef(1,9,0);
        DesenhaObjeto("gameObjects/nivel1Guindaste.txt");
        glTranslatef(-1,-9,0);
        DesenhaNivel2Guindaste();
    glPopMatrix();
}

void DesenhaBaseGuindaste()
{
    glPushMatrix();
        DesenhaObjeto("gameObjects/baseGuindaste.txt");
        DesenhaNivel1Guindaste();
    glPopMatrix();
}

void DesenhaGuindaste()
{
    glTranslatef(baseDx,baseDy,0);
    DesenhaBaseGuindaste();
}

void DesenhaPisoParedes()
{
    glPushMatrix();
        DesenhaObjeto("gameObjects/piso.txt");
        glTranslatef(0,8,0);
        DesenhaObjeto("gameObjects/paredes.txt");
        glTranslatef(182,0,0);
        DesenhaObjeto("gameObjects/paredes.txt");
    glPopMatrix();
}

// **********************************************************************
//  void display( void )
//
// **********************************************************************
void display( void )
{

	// Limpa a tela coma cor de fundo
	glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites lógicos da área OpenGL dentro da Janela
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0,larguraLogica,0,alturaLogica,0,1);

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Coloque aqui as chamadas das rotinas que desenha os objetos
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    DesenhaPisoParedes();
    DesenhaGuindaste();

	glutSwapBuffers();
}


// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
// **********************************************************************

void keyboard ( unsigned char key, int x, int y )
{

	switch ( key )
	{
		case 27:        // Termina o programa qdo
			exit ( 0 );   // a tecla ESC for pressionada
			break;

        case 'q':
            rotacaoN4 += rotacaoN4 < 90 ? 5 : 0;
            cout << rotacaoN4 << endl;
            break;

        case 'w':
            rotacaoN4 -= rotacaoN4 > -90 ? 5 : 0;
            cout << rotacaoN4 << endl;
            break;

        case 'a':
            rotacaoN3 += rotacaoN3 < 45 ? 5 : 0;
            cout << rotacaoN3 << endl;
            break;

        case 's':
            rotacaoN3 -= rotacaoN3 > -45 ? 5 : 0;
            cout << rotacaoN3 << endl;
            break;

        case 'z':
            rotacaoN2 += rotacaoN2 < 45 ? 5 : 0;
            cout << rotacaoN2 << endl;
            break;

        case 'x':
            rotacaoN2 -= rotacaoN2 > -45 ? 5 : 0;
            cout << rotacaoN2 << endl;
            break;

		default:
			break;
	}
}


// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
//
//
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
		case GLUT_KEY_UP:       // Se pressionar UP
			glutFullScreen ( ); // Vai para Full Screen
			break;
	    case GLUT_KEY_DOWN:     // Se pressionar UP
								// Reposiciona a janela
            glutPositionWindow (50,50);
			glutReshapeWindow ( 700, 500 );
			break;

        case GLUT_KEY_LEFT:
            baseDx -= 1;
            break;

        case GLUT_KEY_RIGHT:
            baseDx += 1;
            break;

		default:
			break;
	}
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
//
// **********************************************************************
int  main ( int argc, char** argv )
{
    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de título da janela.
    glutCreateWindow    ( "Primeiro Programa em OpenGL" );

    // executa algumas inicializações
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // será chamada automaticamente quando
    // for necessário redesenhar a janela
    glutDisplayFunc ( display );
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // será chamada automaticamente quando
    // o usuário alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // será chamada automaticamente sempre
    // o usuário pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" será chamada
    // automaticamente sempre o usuário
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
