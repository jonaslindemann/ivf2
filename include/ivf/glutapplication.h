#pragma once

#ifdef __APPLE__
#include <GL/glew.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <string>

#include "transformmanager.h"

class GlutWindow {
private:
    std::string m_title;
    int m_id;
    bool m_leftButtonDown;
    bool m_middleButtonDown;
    bool m_rightButtonDown;
    int m_mouseDownPos[2];
    int m_mouseUpPos[2];
    double m_eyePos[3];
    double m_eyeDist;
    double m_eyeAlfa;
    double m_eyeBeta;
    double m_startAlfa;
    double m_startBeta;
    double m_deltaAlfa;
    double m_deltaBeta;
    int m_width;
    int m_height;
    
    ivf::TransformManager* m_transformMgr;
public:
    GlutWindow(int x, int y, int width, int height);
    virtual ~GlutWindow();
    
    void setTitle(const std::string& title);
    
    void setSize(int width, int height);

    virtual void onInit();
    virtual void onPreRender();
    virtual void onRender();
    virtual void onPostRender();
    virtual void onIdleProcessing ();
    virtual void onMouse(int button, int state, int x, int y);
    virtual void onMouseMove (int mx, int my);
    virtual void onResize (int width, int height);
    virtual void onKeys (unsigned char key, int x, int y);
    virtual void onFunctionKeys (int key, int x, int y);
  
};

class GlutApplication {
private:
    GlutWindow* m_window;
    bool m_initWindow;;
public:
    static GlutApplication* instance(int *argc, char **argv)
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new GlutApplication(argc, argv);
        }
        return m_instance;
    }
    
    static GlutApplication* instance()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new GlutApplication();
        }
        return m_instance;        
    }
    
    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }
    
    void setWindow(GlutWindow* window);
    GlutWindow* window();
    
    bool initWindow()
    {
        return m_initWindow;
    }
    
    void initWindowComplete()
    {
        m_initWindow = false;
    }
    
    void mainLoop();
        
    virtual ~GlutApplication()
    {
        this->drop();
    }
    
private:
    GlutApplication(int *argc, char **argv);
    GlutApplication();
    GlutApplication(const GlutApplication &); // hide copy constructor
    GlutApplication& operator=(const GlutApplication &); // hide assign op
    
    static GlutApplication* m_instance;
};

void doRender();
void doIdleProcessing ();
void doMouse(int button, int state, int x, int y);
void doMouseMove (int mx, int my);
void doResize (int width, int height);
void doKeys (unsigned char key, int x, int y);
void doFunctionKeys (int key, int x, int y);
