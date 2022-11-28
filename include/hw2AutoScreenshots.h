/**************************************************
The helper function that generates screenshots for HW2
*****************************************************/
void keyboard(unsigned char key, int x, int y);
void specialKey(int key, int x, int y);
void display(void);
void saveScreenShot(const char* filename);
void hw2AutoScreenshots(){
    camera.reset();
    model_selection = 1;
    enable_perspective = 0;
    display();
    saveScreenShot("image-00.png");
    
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    display();
    saveScreenShot("image-01.png");
    
    specialKey(GLUT_KEY_DOWN,0,0);
    specialKey(GLUT_KEY_DOWN,0,0);
    display();
    saveScreenShot("image-02.png");
    
    specialKey(GLUT_KEY_LEFT,0,0);
    specialKey(GLUT_KEY_LEFT,0,0);
    specialKey(GLUT_KEY_LEFT,0,0);
    specialKey(GLUT_KEY_LEFT,0,0);
    specialKey(GLUT_KEY_LEFT,0,0);
    display();
    saveScreenShot("image-03.png");
    
    keyboard('p',0,0);
    display();
    saveScreenShot("image-04.png");
    
    keyboard('2',0,0);
    specialKey(GLUT_KEY_LEFT,0,0);
    specialKey(GLUT_KEY_LEFT,0,0);
    specialKey(GLUT_KEY_UP,0,0);
    specialKey(GLUT_KEY_UP,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_DOWN,0,0);
    specialKey(GLUT_KEY_DOWN,0,0);
    specialKey(GLUT_KEY_DOWN,0,0);
    display();
    saveScreenShot("image-05.png");
    
    keyboard('3',0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_RIGHT,0,0);
    specialKey(GLUT_KEY_UP,0,0);
    specialKey(GLUT_KEY_UP,0,0);
    specialKey(GLUT_KEY_UP,0,0);
    specialKey(GLUT_KEY_UP,0,0);
    display();
    saveScreenShot("image-06.png");
    
    camera.reset();
    model_selection = 1;
    enable_perspective = 0;
    display();
}
