#include "Render.h"
#include "GUItextRectangle.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>

#ifdef _DEBUG
#include <Debugapi.h>
struct debug_print
{
    template <class C> debug_print& operator<<(const C& a)
    {
        OutputDebugStringA((std::stringstream() << a).str().c_str());
        return *this;
    }
} debout;
#else
struct debug_print
{
    template <class C> debug_print& operator<<(const C& a)
    {
        return *this;
    }
} debout;
#endif

#include "MyOGL.h"
extern OpenGL gl;
#include "Light.h"
Light light;
#include "Camera.h"
Camera camera;

bool lightning = true;      // Включение/выключение освещения
bool alpha = false;         // Включение/выключение прозрачности

void switchModes(OpenGL* sender, KeyEventArg arg)
{
    auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));

    switch (key)
    {
    case 'L':
        lightning = !lightning;
        break;
    case 'A':
        alpha = !alpha;
        break;
    }
}

GuiTextRectangle text;

// Функция для вычисления нормали через векторное произведение
void calculateNormal(double a[], double b[], double c[], double normal[]) {
    double ab[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
    double ac[3] = { c[0] - a[0], c[1] - a[1], c[2] - a[2] };

    normal[0] = ab[1] * ac[2] - ab[2] * ac[1];
    normal[1] = ab[2] * ac[0] - ab[0] * ac[2];
    normal[2] = ab[0] * ac[1] - ab[1] * ac[0];

    double length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (length != 0) {
        normal[0] /= length;
        normal[1] /= length;
        normal[2] /= length;
    }
}

// Отрисовка четырёхугольника с нормалью 
void drawQuad(double a[], double b[], double c[], double d[], double normal[]) {
    glNormal3dv(normal);
    glBegin(GL_QUADS);
    glVertex3dv(a);
    glVertex3dv(b);
    glVertex3dv(c);
    glVertex3dv(d);
    glEnd();
}

// Отрисовка четырёхугольника с нормалью и альфа-каналом 
void drawQuadAlpha(double a[], double b[], double c[], double d[], double normal[], float alphaVal) {
    glColor4f(0.2f, 0.7f, 0.2f, alphaVal); 
    glNormal3dv(normal);
    glBegin(GL_QUADS);
    glVertex3dv(a);
    glVertex3dv(b);
    glVertex3dv(c);
    glVertex3dv(d);
    glEnd();
}

// Отрисовка призмы с правильными нормалями
void drawPrism() {
    // Вершины нижнего основания 
    double A[]{ -6, 0, 0 };
    double B[]{ -1, 1, 0 };
    double C[]{ 1, 8, 0 };
    double D[]{ 5, 4, 0 };
    double E[]{ 2, 0, 0 };
    double F[]{ 5, -6, 0 };
    double G[]{ 1, -8, 0 };
    double H[]{ -1, -1, 0 };

    // Вершины верхнего основания 
    double A1[]{ -6, 0, 5 };
    double B1[]{ -1, 1, 5 };
    double C1[]{ 1, 8, 5 };
    double D1[]{ 5, 4, 5 };
    double E1[]{ 2, 0, 5 };
    double F1[]{ 5, -6, 5 };
    double G1[]{ 1, -8, 5 };
    double H1[]{ -1, -1, 5 };

    // Статические нормали для дна и крышки
    double bottomNormal[3] = { 0, 0, -1 };
    double topNormal[3] = { 0, 0, 1 };

    double normal[3];

    glColor3d(0.2, 0.7, 0.2);
    drawQuad(A, H, E, B, bottomNormal);
    drawQuad(B, E, D, C, bottomNormal);
    drawQuad(H, G, F, E, bottomNormal);

    if (alpha) {
  
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 
        drawQuadAlpha(A1, H1, E1, B1, topNormal, 0.5f);
        drawQuadAlpha(B1, E1, D1, C1, topNormal, 0.5f);
        drawQuadAlpha(H1, G1, F1, E1, topNormal, 0.5f);

        glDisable(GL_BLEND);
    }
    else {
  
        drawQuad(A1, H1, E1, B1, topNormal);
        drawQuad(B1, E1, D1, C1, topNormal);
        drawQuad(H1, G1, F1, E1, topNormal);
    }

    glColor3d(0.2, 0.7, 0.2);

    // Для грани A-H-H1-A1
    calculateNormal(A, H, H1, normal);
    drawQuad(A, H, H1, A1, normal);

    // Для грани H-E-E1-H1
    calculateNormal(H, E, E1, normal);
    drawQuad(H, E, E1, H1, normal);

    // Для грани E-B-B1-E1
    calculateNormal(E, B, B1, normal);
    drawQuad(E, B, B1, E1, normal);

    // Для грани B-A-A1-B1
    calculateNormal(B, A, A1, normal);
    drawQuad(B, A, A1, B1, normal);

    // B-E-E1-B1
    calculateNormal(B, E, E1, normal);
    drawQuad(B, E, E1, B1, normal);

    // E-D-D1-E1
    calculateNormal(E, D, D1, normal);
    drawQuad(E, D, D1, E1, normal);

    // D-C-C1-D1
    calculateNormal(D, C, C1, normal);
    drawQuad(D, C, C1, D1, normal);

    // C-B-B1-C1
    calculateNormal(C, B, B1, normal);
    drawQuad(C, B, B1, C1, normal);

    // H-G-G1-H1
    calculateNormal(H, G, G1, normal);
    drawQuad(H, G, G1, H1, normal);

    // G-F-F1-G1
    calculateNormal(G, F, F1, normal);
    drawQuad(G, F, F1, G1, normal);

    // F-E-E1-F1
    calculateNormal(F, E, E1, normal);
    drawQuad(F, E, E1, F1, normal);

    // E-H-H1-E1
    calculateNormal(E, H, H1, normal);
    drawQuad(E, H, H1, E1, normal);
}

void initRender()
{
    
    camera.caclulateCameraPos();
    gl.WheelEvent.reaction(&camera, &Camera::Zoom);
    gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
    gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
    gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
    gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);

    gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
    gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
    gl.KeyUpEvent.reaction(&light, &Light::StopDrug);

    gl.KeyDownEvent.reaction(switchModes);
    text.setSize(512, 200);


    camera.setPosition(10, 8, 12);
}

void Render(double delta_time)
{
    glEnable(GL_DEPTH_TEST);


    if (gl.isKeyPressed('F'))
    {
        light.SetPosition(camera.x(), camera.y(), camera.z());
    }

    camera.SetUpCamera();
    light.SetUpLight();
    gl.DrawAxes();

 
    if (lightning)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);

    
    float amb[] = { 0.2f, 0.4f, 0.2f, 1.0f };
    float dif[] = { 0.3f, 0.7f, 0.3f, 1.0f };
    float spec[] = { 0.5f, 0.8f, 0.5f, 1.0f };
    float sh = 64.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT, GL_SHININESS, sh);
    glShadeModel(GL_SMOOTH);

 
    drawPrism();

    
    light.DrawLightGizmo();

  
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(2)
        << L"=== УПРАВЛЕНИЕ ===\n"
        << L"L - " << (lightning ? L"[вкл]выкл" : L"вкл[выкл]") << L" освещение\n"
        << L"A - " << (alpha ? L"[вкл]выкл" : L"вкл[выкл]") << L" прозрачность верхнего основания\n"
        << L"F - переместить свет в позицию камеры\n"
        << L"\n=== КООРДИНАТЫ ===\n"
        << L"Свет: (" << std::setw(6) << light.x() << L"," << std::setw(6) << light.y() << L"," << std::setw(6) << light.z() << L")\n"
        << L"Камера: (" << std::setw(6) << camera.x() << L"," << std::setw(6) << camera.y() << L"," << std::setw(6) << camera.z() << L")\n"
        << L"\n=== ПОЯСНЕНИЕ ===\n"
        << L"Нормали вычислены через векторное произведение\n"
        << L"Альфа-смешивание для верхней грани: " << (alpha ? L"ВКЛ (крышка прозрачная)" : L"ВЫКЛ (крышка непрозрачная)") << L"\n";

    text.setPosition(10, gl.getHeight() - 10 - 200);
    text.setText(ss.str().c_str());
    text.Draw();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}