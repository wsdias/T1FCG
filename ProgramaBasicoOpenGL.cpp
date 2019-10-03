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
#include <vector>

using namespace std;

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

typedef struct
{
    GLfloat x, y, z;
} Ponto;

int larguraLogica = 192, alturaLogica = 108;
float baseDx = 0, baseDy = 0, rotacaoN2 = 0, rotacaoN3 = 0, rotacaoN4 = 0;
vector<vector<vector<float>>> cores; // [objeto][cor][componente]
vector<vector<vector<int>>> objetos; // [objeto][x][y]
vector<vector<Ponto>> pontos; // [objeto][ponto]

void CalculaPonto(Ponto p, Ponto &out) {

    GLfloat ponto_novo[4];
    GLfloat matriz_gl[4][4];
    int  i;

    glGetFloatv(GL_MODELVIEW_MATRIX,&matriz_gl[0][0]);

    for(i=0; i<4; i++) {
        ponto_novo[i] = matriz_gl[0][i] * p.x +
                        matriz_gl[1][i] * p.y +
                        matriz_gl[2][i] * p.z +
                        matriz_gl[3][i];
    }
    out.x = ponto_novo[0];
    out.y = ponto_novo[1];
    out.z = ponto_novo[2];
}

int intersec2d(Ponto k, Ponto l, Ponto m, Ponto n, double &s, double &t)
{
    double det;

    det = (n.x - m.x) * (l.y - k.y)  -  (n.y - m.y) * (l.x - k.x);

    if (det == 0.0) return 0 ; // não há intersecção

    cout << "s: " << ((n.x - m.x) * (m.y - k.y) - (n.y - m.y) * (m.x - k.x))/ det << endl;
    cout << "t: " << ((l.x - k.x) * (m.y - k.y) - (l.y - k.y) * (m.x - k.x))/ det << endl;

    //s = ((n.x - m.x) * (m.y - k.y) - (n.y - m.y) * (m.x - k.x))/ det ;
    //t = ((l.x - k.x) * (m.y - k.y) - (l.y - k.y) * (m.x - k.x))/ det ;

    return 1; // há intersecção
}


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
    //glOrtho((-1)*larguraLogica,larguraLogica,(-1)*alturaLogica,alturaLogica,0,1);
}

void CarregaObjeto(string diretorio, int index)
{
    int i, j, cor, nCores, linhas, colunas;
    ifstream arquivo;
    string conteudo;

    arquivo.open(diretorio.c_str());

    arquivo >> conteudo;
    arquivo >> nCores;

    cores.resize(cores.size() + 1);
    cores[index].resize(nCores);

    for (i = 0; i < nCores; i++)
    {
        arquivo >> conteudo;
        cores[index][i].resize(3);
        for (j = 0; j < 3; j++) arquivo >> cores[index][i][j];
    }

    arquivo >> conteudo;
    arquivo >> linhas;
    arquivo >> colunas;

    objetos.resize(objetos.size() + 1);
    objetos[index].resize(linhas);
    for (i = linhas-1; i >= 0; i--)
    {
        objetos[index][i].resize(colunas);
        for (j = 0; j < colunas; j++)
        {
            arquivo >> cor;
            objetos[index][i][j] = cor;
        }
    }

    arquivo.close();
}

void DesenhaObjeto(int index, float dx, float dy)
{
    int i, j, linhas, colunas, cor;

    linhas = objetos[index].size();
    colunas = objetos[index][0].size();

    for (i = 0; i < linhas; i++)
    {
        for (j = 0; j < colunas; j++)
        {
            cor = objetos[index][i][j] - 1;
            //glPushMatrix();
            glColor3f(cores[index][cor][0], cores[index][cor][1], cores[index][cor][2]);
            glBegin(GL_QUADS);
                glVertex2f(j-dx,i+dy);
                glVertex2f(j-dx,i+dy+1);
                glVertex2f(j-dx+1,i+dy+1);
                glVertex2f(j-dx+1,i+dy);
            glEnd();
            //glPopMatrix();
        }
    }
}

// -------------------------------------------------- //

void CalcInterRoboCaixas()
{
    double *s, *t;
    Ponto a = pontos[6][0], b = pontos[6][0],
          c = pontos[7][0], d = pontos[7][0];

    a.x -= 1; //a.y += 1;
    b.x += 1; //b.y += 1;

    c.x -= 4; c.y += 0;
    d.x += 4; d.y += 0;

    cout << "A(" << a.x << ", " << a.y << ")" << endl;
    cout << "B(" << b.x << ", " << b.y << ")" << endl;

    cout << "C(" << c.x << ", " << c.y << ")" << endl;
    cout << "D(" << d.x << ", " << d.y << ")" << endl;

    cout << "Intersec1: " << endl << intersec2d(a, b, c, d, *s, *t) << endl;

    /* TESTE */
    Ponto k = {10, 5, 0}, l = {20, 4, 0}, m = {7, 6, 0}, n = {15, 4, 0};
    cout << "Intersec2: " << endl << intersec2d(k, l, m, n, *s, *t) << endl;

    glPushMatrix();
        glLineWidth(10);
        glColor3f(0, 1, 0);
        glBegin(GL_LINES);
            glVertex2d(a.x, a.y);
            glVertex2d(b.x, b.y);
        glEnd();
        glColor3f(1, 0, 0);
        glBegin(GL_LINES);
            glVertex2d(c.x, c.y);
            glVertex2d(d.x, d.y);
        glEnd();
    glPopMatrix();
}

void DesenhaCaixa1()
{
    //Ponto a = {objetos[7][0].size()/2.0, objetos[7].size(), 0.0}, b;
    Ponto a = {0, objetos[7].size(), 0.0}, b;

    glPushMatrix();
        //glTranslatef(larguraLogica/2.0 - 15.0, 0.0, 0.0);
        //glTranslatef(4.5, 0.0, 0.0);
        DesenhaObjeto(7, objetos[7][0].size()/2.0, 0.0);
        CalculaPonto(a,b);
        pontos[7][0] = b;
        //cout << "(" << b.x << ", " << b.y << ")" << endl;
    glPopMatrix();
}

void DesenhaCaixas()
{
    glPushMatrix();
        glTranslatef(10.0, 0.0, 0.0);
        DesenhaCaixa1();
    glPopMatrix();
}

// -------------------------------------------------- //

void DesenhaNivel4Robo()
{
    //Ponto a = {objetos[6][0].size()/2.0, objetos[6].size(), 0.0}, b;
    //Ponto a = {0, objetos[6].size(), 0.0}, b;

    glPushMatrix();
        glTranslatef(0.0, objetos[5].size() - 1.0, 0.0); // Posiciona verticalmente em ralação ao objeto anterior
        glRotatef(rotacaoN4, 0.0, 0.0, 1.0);
        Ponto a = {0, objetos[6].size(), 0.0}, b;
        DesenhaObjeto(6, objetos[6][0].size()/2.0, 0.0); // Cria instância com parametros desejados (posição no SRO)

        glTranslatef(objetos[6][0].size()/2.0, 15.0, 0.0);
        glRotatef(rotacaoN4, 0.0, 0.0, 1.0);
        glTranslatef(-(objetos[6][0].size()/2.0), -15.0, 0.0);

        CalculaPonto(a,b);
        pontos[6][0] = b;
        //cout << "A:(" << a.x << ", " << a.y << ")" << endl;
        //cout << "B:(" << b.x << ", " << b.y << ")" << endl;
    glPopMatrix();
}

void DesenhaNivel3Robo()
{
    //Ponto a = {objetos[5][0].size()/2.0, objetos[5].size(), 0.0}, b;

    glPushMatrix();
        glTranslatef(0.0, objetos[4].size() - 2.0, 0.0); // Posiciona verticalmente em ralação ao objeto anterior
        glRotatef(rotacaoN3, 0.0, 0.0, 1.0);
        DesenhaObjeto(5, objetos[5][0].size()/2.0, 0.0); // Cria instância com parametros desejados (posição no SRO)
        DesenhaNivel4Robo();
        //CalculaPonto(a,b);
        //cout << "(" << b.x << ", " << b.y << ")" << endl;
    glPopMatrix();
}

void DesenhaNivel2Robo()
{
    //Ponto a = {objetos[4][0].size()/2.0, objetos[4].size(), 0.0}, b;

    glPushMatrix();
        glTranslatef(0.0, objetos[3].size() - 2.0, 0.0); // Posiciona acima do nível, deslocando uma unidade abaixo (sobrepõe)
        glRotatef(rotacaoN2, 0.0, 0.0, 1.0);
        DesenhaObjeto(4, objetos[4][0].size()/2.0, 0.0); // Cria instância com parametros desejados (posição no SRO)
        //CalculaPonto(a,b);
        //cout << "(" << b.x << ", " << b.y << ")" << endl;
        DesenhaNivel3Robo();
    glPopMatrix();
}

void DesenhaNivel1Robo()
{
    glPushMatrix();
        glTranslatef(0.0, objetos[2].size(), 0.0); // Posiciona acima da base
        DesenhaObjeto(3, objetos[3][0].size()/2.0, 0.0); // Desenha de forma centralizada (em relação ao SRO)
        DesenhaNivel2Robo();
    glPopMatrix();
}

void DesenhaBaseRobo()
{
    glPushMatrix();
        DesenhaObjeto(2, objetos[2][0].size()/2.0, 0.0);
        DesenhaNivel1Robo();
    glPopMatrix();
}

void DesenhaRobo()
{
    glPushMatrix();
        glTranslatef(baseDx, baseDy, 0.0); // Movimenta em relação ao universo
        //glTranslatef(larguraLogica/2.0, 0.0, 0.0); // Posicina em relação universo
        glTranslatef(40.0, 0.0, 0.0); // Posicina em relação universo
        DesenhaBaseRobo();
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
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, larguraLogica, 0, alturaLogica, 0, 1);

    // Realy needed?
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Coloque aqui as chamadas das rotinas que desenha os objetos
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	// Axis
	glPushMatrix();
        glColor3f(0,0,0);
        glLineWidth(10);
        glBegin(GL_LINES);
            // Vertical axis
            glVertex2d(larguraLogica/2.0, 0);
            glVertex2d(larguraLogica/2.0, alturaLogica);
            // Horizontal axis
            glVertex2d(0, alturaLogica/2.0);
            glVertex2d(larguraLogica, alturaLogica/2.0);
        glEnd();
	glPopMatrix();

    //DesenhaPisoParedes();
    DesenhaCaixas();
    DesenhaRobo();

    for (int i = 0; i < 8; i++) cout << "P" << i << ": (" << pontos[i][0].x  << ", " << pontos[i][0].y << ")" << endl;
    CalcInterRoboCaixas();

    /*Ponto pa = {5, 10, 0}, pb = {0, 0, 0};
    glColor3f(0.25, 0.75, 0.50);
    glBegin(GL_QUADS);
        glVertex2d(0,0);
        glVertex2d(0,10);
        glVertex2d(10,10);
        glVertex2d(10,0);
    glEnd();
    CalculaPonto(pa, pb);
    cout << "PA:(" << pa.x << ", " << pa.y << ", " << pa.z << ")" << endl;
    cout << "PB:(" << pb.x << ", " << pb.y << ", " << pb.z << ")" << endl;*/

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
            //cout << rotacaoN4 << endl;
            break;

        case 'w':
            rotacaoN4 -= rotacaoN4 > -90 ? 5 : 0;
            //cout << rotacaoN4 << endl;
            break;

        case 'a':
            rotacaoN3 += rotacaoN3 < 45 ? 5 : 0;
            //cout << rotacaoN3 << endl;
            break;

        case 's':
            rotacaoN3 -= rotacaoN3 > -45 ? 5 : 0;
            //cout << rotacaoN3 << endl;
            break;

        case 'z':
            rotacaoN2 += rotacaoN2 < 45 ? 5 : 0;
            //cout << rotacaoN2 << endl;
            break;

        case 'x':
            rotacaoN2 -= rotacaoN2 > -45 ? 5 : 0;
            //cout << rotacaoN2 << endl;
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

void CarregaCenario()
{
    int i;

    CarregaObjeto("gameObjects/piso.txt", 0);
    CarregaObjeto("gameObjects/paredes.txt", 1);
    CarregaObjeto("gameObjects/baseRobo.txt", 2);
    CarregaObjeto("gameObjects/nivel1Robo.txt", 3);
    CarregaObjeto("gameObjects/nivel2Robo.txt", 4);
    CarregaObjeto("gameObjects/nivel3Robo.txt", 5);
    CarregaObjeto("gameObjects/nivel4Robo.txt", 6);
    CarregaObjeto("gameObjects/caixaAmarela.txt", 7);

    for (i = 0; i < 8; i++)
    {
        pontos.resize(pontos.size()+1);
        pontos[i].resize(1);
        pontos[i][0] = {0, 0, 0};
    }
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
//
// **********************************************************************
int  main ( int argc, char** argv )
{
    // Carrega objetos
    CarregaCenario();

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
