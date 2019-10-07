// **********************************************************************
//
//  T1 - Fundamentos de Computação Gráfica
//
//  Willian Schmiele Dias
//
//  2019/2
//
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
#include <math.h>

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

typedef struct{
    float x, y;
} Instancia;

float baseDx = 0, baseDy = 0, rotacaoN2 = 0, rotacaoN3 = 0, rotacaoN4 = 0, caixaCaindo = -1;
int larguraLogica = 192, alturaLogica = 108, segurando = -1, numObjetos = 0;
bool podeMover = false;

vector< vector< vector<float> > > cores; // [objeto][cor][componente]
vector< vector< vector<int> > > objetos; // [objeto][x][y]
vector<Instancia> deslocamento; // [objetos]
vector<Ponto> pa, pb, cLocal, cUniverso; // [objeto]

bool VerificaLivreEmbaixo(int caixa);

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

int intersec2d(Ponto k, Ponto l, Ponto m, Ponto n)
{
    double det;

    det = (n.x - m.x) * (l.y - k.y)  -  (n.y - m.y) * (l.x - k.x);
    if (det == 0.0) return 0 ; // não há intersecção
    else return 1; // há intersecção
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

// -------------------------------------------------- //

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
        for (j = 0; j < 3; j++)
        {
            arquivo >> cores[index][i][j];
            cores[index][i][j] /= 255.0;
        }
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

// -------------------------------------------------- //

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
            glPushMatrix();
            glColor3f(cores[index][cor][0], cores[index][cor][1], cores[index][cor][2]);
            glBegin(GL_QUADS);
                glVertex2f(j-dx,i+dy);
                glVertex2f(j-dx,i+dy+1);
                glVertex2f(j-dx+1,i+dy+1);
                glVertex2f(j-dx+1,i+dy);
            glEnd();
            glPopMatrix();
        }
    }
}

// -------------------------------------------------- //

void DesenhaCaixas()
{
    Ponto a, b;
    int i;

    if (VerificaLivreEmbaixo(15)) deslocamento[15].y -= 0.1;

    for (i = 14; i < numObjetos; i++)
    {
        if (i != segurando)
        {
            glPushMatrix();
                glTranslatef(deslocamento[i].x, deslocamento[i].y, 0.0);
                a = {(objetos[i][0].size()/2.0), objetos[i].size()/2.0, 0.0};

                DesenhaObjeto(i, 0.0, 0.0);
                CalculaPonto(a, b);
                pa[i] = a;
                pb[i] = b;
                cLocal[i] = {(objetos[i][0].size()/2.0), objetos[i].size()/2.0, 0.0};
                CalculaPonto(cLocal[i], cUniverso[i]);
            glPopMatrix();
        }
    }
}

// -------------------------------------------------- //

void DesenhaNivel4Robo()
{
    Ponto a, b;

    glPushMatrix();

        glTranslatef(0.0, objetos[6].size() - 1.0, 0.0); // Posiciona verticalmente em relação ao objeto anterior
        glRotatef(rotacaoN4, 0.0, 0.0, 1.0);
        a = {(objetos[7][0].size()/2.0), objetos[7].size(), 0.0};
        cLocal[7] = {(objetos[7][0].size()/2.0), objetos[7].size()/2.0, 0.0};
        DesenhaObjeto(7, objetos[7][0].size()/2.0, 0.0); // Cria instância com parametros desejados (posição no SRO)
        glTranslated((-1) * (objetos[7][0].size()/2.0),0,0);

        CalculaPonto(a, b);
        CalculaPonto(cLocal[7], cUniverso[7]);
        pa[7] = a;
        pb[7] = b;

        if (segurando != -1)
        {
            cLocal[segurando] = a;
            deslocamento[segurando].x = b.x;
            deslocamento[segurando].y = b.y;
            cUniverso[segurando].x = deslocamento[segurando].x;
            cUniverso[segurando].y = deslocamento[segurando].y;
            DesenhaObjeto(segurando, a.x, a.y);
        }

    glPopMatrix();
}

// -------------------------------------------------- //

void DesenhaNivel3Robo()
{
    glPushMatrix();
        glTranslatef(0.0, objetos[5].size() - 2.0, 0.0); // Posiciona verticalmente em ralação ao objeto anterior
        glRotatef(rotacaoN3, 0.0, 0.0, 1.0);
        DesenhaObjeto(6, objetos[6][0].size()/2.0, 0.0); // Cria instância com parametros desejados (posição no SRO)
        cLocal[6] = {0.0, objetos[6].size()/2.0, 0.0};
        CalculaPonto(cLocal[6], cUniverso[6]);
        DesenhaNivel4Robo();
    glPopMatrix();
}

// -------------------------------------------------- //

void DesenhaNivel2Robo()
{
    glPushMatrix();
        glTranslatef(0.0, objetos[4].size() - 2.0, 0.0); // Posiciona acima do nível, deslocando uma unidade abaixo (sobrepõe)
        glRotatef(rotacaoN2, 0.0, 0.0, 1.0);
        DesenhaObjeto(5, objetos[5][0].size()/2.0, 0.0); // Cria instância com parametros desejados (posição no SRO)
        cLocal[5] = {0.0, objetos[5].size()/2.0, 0.0};
        CalculaPonto(cLocal[5], cUniverso[5]);
        DesenhaNivel3Robo();
    glPopMatrix();
}

// -------------------------------------------------- //

void DesenhaNivel1Robo()
{
    glPushMatrix();
        glTranslatef(0.0, objetos[3].size(), 0.0); // Posiciona acima da base
        DesenhaObjeto(4, objetos[4][0].size()/2.0, 0.0); // Desenha de forma centralizada (em relação ao SRO)
        cLocal[4] = {0.0, objetos[4].size()/2.0, 0.0};
        CalculaPonto(cLocal[4], cUniverso[4]);
        DesenhaNivel2Robo();
    glPopMatrix();
}

// -------------------------------------------------- //

void DesenhaRobo()
{
    glPushMatrix();
        glTranslatef(deslocamento[3].x + baseDx, deslocamento[3].y + baseDy, 0.0); // Posicina e movimenta em relação universo
        DesenhaObjeto(3, objetos[3][0].size()/2.0, 0.0);
        DesenhaNivel1Robo();
        cLocal[3] = {0.0, objetos[3].size()/2.0, 0.0};
        CalculaPonto(cLocal[3], cUniverso[3]);
    glPopMatrix();
}

// -------------------------------------------------- //

void DesenhaPisoParedesPrateleiras()
{
    Ponto a, b;
    int i;

    // Piso e paredes
    for (i = 0; i < 3; i++)
    {
        glPushMatrix();
            glTranslatef(deslocamento[i].x, deslocamento[i].y, 0.0);
            a = {(objetos[i][0].size()/2.0), objetos[i].size()/2.0, 0.0};
            DesenhaObjeto(i, 0.0, 0.0);
            CalculaPonto(a, b);
            pa[i] = cLocal[i] = a;
            pb[i] = cUniverso[i] = b;
        glPopMatrix();
    }

    // Prateleiras
    for (i = 8; i < 14; i++)
    {
        glPushMatrix();
            glTranslatef(deslocamento[i].x, deslocamento[i].y, 0.0);
            a = {(objetos[i][0].size()/2.0), objetos[i].size()/2.0, 0.0};
            DesenhaObjeto(i, 0.0, 0.0);
            CalculaPonto(a, b);
            pa[i] = cLocal[i] = a;
            pb[i] = cUniverso[i] = b;
        glPopMatrix();
    }
}

// -------------------------------------------------- //

void DesenhaContornos()
{
    Ponto a, b, c, d;
    int i;

    for (i = 0; i < numObjetos; i++)
    {
        a = b = c = d = cUniverso[i];
        a.x -= (objetos[i][0].size()/2.0); a.y -= (objetos[i].size()/2.0);
        b.x -= (objetos[i][0].size()/2.0); b.y += (objetos[i].size()/2.0);
        c.x += (objetos[i][0].size()/2.0); c.y += (objetos[i].size()/2.0);
        d.x += (objetos[i][0].size()/2.0); d.y -= (objetos[i].size()/2.0);
        glPushMatrix();
            glColor3f(1, 0, 0);
            glLineWidth(2);
            glBegin(GL_LINES);
                glVertex2d(a.x, a.y);
                glVertex2d(b.x, b.y);
                glVertex2d(b.x, b.y);
                glVertex2d(c.x, c.y);
                glVertex2d(c.x, c.y);
                glVertex2d(d.x, d.y);
                glVertex2d(d.x, d.y);
                glVertex2d(a.x, a.y);
            glEnd();
        glPopMatrix();
    }
}

// -------------------------------------------------- //

bool VerificaLivreEmCima(int caixa)
{
    Ponto a, b, c, d, e, f, g, h;
    float largCaixa, altCaixa, larg, alt;
    int i;

    largCaixa = objetos[caixa][0].size()/2.0;
    altCaixa = objetos[caixa].size()/2.0;

    e = f = g = h = cUniverso[caixa];
    e.x -= largCaixa; e.y -= altCaixa;
    f.x -= largCaixa; f.y += altCaixa;
    g.x += largCaixa; g.y += altCaixa;
    h.x += largCaixa; h.y -= altCaixa;

    for (i = 14; i < numObjetos; i++)
    {
        if (i != caixa)
        {
            larg = objetos[i][0].size()/2.0;
            alt = objetos[i].size()/2.0;

            a = b = c = d = cUniverso[i];
            a.x -= larg; a.y -= alt;
            b.x -= larg; b.y += alt;
            c.x += larg; c.y += alt;
            d.x += larg; d.y -= alt;

            if (((f.x >= a.x && f.x <= d.x) || (g.x >= a.x && g.x <= d.x)) && (f.y <= a.y))
            {
                cout << "# " << i << " está em cima de " << caixa << endl;
                return false;
            }
        }
    }
    return true;
}

// -------------------------------------------------- //

bool VerificaLivreEmbaixo(int caixa)
{
    Ponto a, b, c, d, e, f, g, h;
    float largCaixa, altCaixa, larg, alt;
    int i;

    largCaixa = objetos[caixa][0].size()/2.0;
    altCaixa = objetos[caixa].size()/2.0;

    e = f = g = h = cUniverso[caixa];
    e.x -= largCaixa; e.y -= altCaixa;
    f.x -= largCaixa; f.y += altCaixa;
    g.x += largCaixa; g.y += altCaixa;
    h.x += largCaixa; h.y -= altCaixa;

    for (i = 14; i < numObjetos; i++)
    {
        if (i != caixa)
        {
            larg = objetos[i][0].size()/2.0;
            alt = objetos[i].size()/2.0;

            a = b = c = d = cUniverso[i];
            a.x -= larg; a.y -= alt;
            b.x -= larg; b.y += alt;
            c.x += larg; c.y += alt;
            d.x += larg; d.y -= alt;

            if (((e.x >= a.x && f.x <= d.x) || (h.x >= a.x && h.x <= d.x)) && (e.y < b.y))
            {
                cout << "# " << caixa << " está embaixo de " << i << endl;
                return false;
            }
        }
    }
    return true;
}

// -------------------------------------------------- //

bool VerificarSePodePegar()
{
    float dist;
    int i;

    for (i = 14; i < numObjetos; i++)
    {
        dist = sqrt(pow((pb[7].x - pb[i].x), 2) + pow((pb[7].y - pb[i].y), 2));
        if (dist < pa[i].x + 1)
        {
            if (VerificaLivreEmCima(i))
            {
                segurando = i;
                return true;
            }
        }
    }
    return false;
}

// -------------------------------------------------- //

void CarregaCenario()
{
    int i;

    // Piso e paredes
    CarregaObjeto("gameObjects/piso.txt", 0);
    CarregaObjeto("gameObjects/paredes.txt", 1);
    CarregaObjeto("gameObjects/paredes.txt", 2);

    // Robo
    CarregaObjeto("gameObjects/baseRobo.txt", 3);
    CarregaObjeto("gameObjects/nivel1Robo.txt", 4);
    CarregaObjeto("gameObjects/nivel2Robo.txt", 5);
    CarregaObjeto("gameObjects/nivel3Robo.txt", 6);
    CarregaObjeto("gameObjects/nivel4Robo.txt", 7);

    // Prateleiras
    CarregaObjeto("gameObjects/prateleiraCinza.txt", 8);
    CarregaObjeto("gameObjects/prateleiraCinza.txt", 9);
    CarregaObjeto("gameObjects/prateleiraCinza.txt", 10);
    CarregaObjeto("gameObjects/prateleiraCinza.txt", 11);
    CarregaObjeto("gameObjects/prateleiraCinza.txt", 12);
    CarregaObjeto("gameObjects/prateleiraCinza.txt", 13);

    // Caixas
    CarregaObjeto("gameObjects/caixaAmarela.txt", 14);
    CarregaObjeto("gameObjects/caixaAmarela.txt", 15);
    CarregaObjeto("gameObjects/caixaAmarela.txt", 16);
    CarregaObjeto("gameObjects/caixaVerde.txt", 17);
    CarregaObjeto("gameObjects/caixaVerde.txt", 18);
    CarregaObjeto("gameObjects/caixaVerde.txt", 19);

    numObjetos = 20;

    pa.resize(numObjetos+1);
    pb.resize(numObjetos+1);
    cLocal.resize(numObjetos+1);
    cUniverso.resize(numObjetos+1);
    deslocamento.resize(numObjetos+1);

    for (i = 0; i < numObjetos; i++)
    {
        pa[i] = {0, 0, 0};
        pb[i] = {0, 0, 0};
        cLocal[i] = {0, 0, 0};
        cUniverso[i] = {0, 0, 0};
    }
}

// -------------------------------------------------- //

void DefinirPropriedades()
{
    // Cenário (piso e paredes)
    deslocamento[0].x = 0; deslocamento[0].y = 0;
    deslocamento[1].x = 0; deslocamento[1].y = objetos[0].size();
    deslocamento[2].x = larguraLogica - objetos[2][0].size(); deslocamento[2].y = objetos[0].size();

    // Robo
    deslocamento[3].x = larguraLogica/2.0; deslocamento[3].y = objetos[0].size();

    // Prateleiras
    deslocamento[8].x = objetos[1][0].size() - 2; deslocamento[8].y = larguraLogica/8.0;
    deslocamento[9].x = objetos[1][0].size() - 2; deslocamento[9].y = 2 * (larguraLogica/8.0);
    deslocamento[10].x = objetos[1][0].size() - 2; deslocamento[10].y = 3 * (larguraLogica/8.0);

    deslocamento[11].x = deslocamento[2].x - objetos[11][0].size() + 2; deslocamento[11].y = larguraLogica/8.0;
    deslocamento[12].x = deslocamento[2].x - objetos[12][0].size() + 2; deslocamento[12].y = 2 * (larguraLogica/8.0);
    deslocamento[13].x = deslocamento[2].x - objetos[13][0].size() + 2; deslocamento[13].y = 3 * (larguraLogica/8.0);

    // Caixas
    deslocamento[14].x = 50; deslocamento[14].y = objetos[0].size();
    deslocamento[15].x = 50; deslocamento[15].y = objetos[0].size() + objetos[14].size();
    //deslocamento[16].x = 60; deslocamento[16].y = objetos[0].size();
    //deslocamento[17].x = (larguraLogica/2.0) + 30; deslocamento[17].y = objetos[0].size();
    //deslocamento[18].x = (larguraLogica/2.0) + 40; deslocamento[18].y = objetos[0].size();
    //deslocamento[19].x = (larguraLogica/2.0) + 40; deslocamento[19].y = objetos[0].size() + objetos[18].size();
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

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Coloque aqui as chamadas das rotinas que desenha os objetos
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    DesenhaPisoParedesPrateleiras();
    DesenhaCaixas();
    DesenhaRobo();

    // Desenha uma linha da origem até a ponta da garra do robo
    /*glColor3f(1,0,0);
    glBegin(GL_LINES);
        glVertex2d(0,0);
        glVertex2d(pb[7].x, pb[7].y);
    glEnd();*/

    //DesenhaContornos();

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
            rotacaoN4 += rotacaoN4 < 90 ? 15 : 0;
            break;

        case 'w':
            rotacaoN4 -= rotacaoN4 > -90 ? 15 : 0;
            break;

        case 'a':
            rotacaoN3 += rotacaoN3 < 45 ? 15 : 0;
            break;

        case 's':
            rotacaoN3 -= rotacaoN3 > -45 ? 15 : 0;
            break;

        case 'z':
            rotacaoN2 += rotacaoN2 < 90 ? 15 : 0;
            break;

        case 'x':
            rotacaoN2 -= rotacaoN2 > -90 ? 15 : 0;
            break;

        case ' ':
            podeMover = !podeMover;
            if (!podeMover && segurando != -1) segurando = -1;
            podeMover = VerificarSePodePegar();
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
            baseDx -= 2;
            break;

        case GLUT_KEY_RIGHT:
            baseDx += 2;
            break;

        /*case GLUT_KEY_PAGE_UP:
            yAux += 1;
            break;

        case GLUT_KEY_PAGE_DOWN:
            yAux -= 1;
            break;*/

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
    // Carrega objetos
    CarregaCenario();
    DefinirPropriedades();

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

