#include "Renderer.h"


// constructor
// ------------------------------------------------------------------------
Renderer::Renderer(float ppm, int fbW, int fbH) : ppm(ppm), fbW(fbW), fbH(fbH) {};

// destructor
// ------------------------------------------------------------------------
// default destructor is used. 

// draw
// ------------------------------------------------------------------------
void Renderer::draw(const Entity& e) const {
    
    // 1. material/program
    e.rectShader->use();

    // 2. convert meters to NDC
    const Position2D ndcPos = metersToNDC(e.getPosX(), e.getPosY());

    const Position2D ndcSize = rectSizeToNDC(e.getWidth(), e.getLength());

    e.rectShader->setOffset(ndcPos.x, ndcPos.y);
    e.rectShader->setYaw(e.getYaw());
    e.rectShader->setScale(ndcSize.x, ndcSize.y);
    const auto& c = e.getColor();
    e.rectShader->setColor(c[0], c[1], c[2], c[3]);
    
    // 3. mesh and draw
    glBindVertexArray(e.loader->getVAO()); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
};

// converts a point (the object center in meters) into an NDC position for uOffset
// ------------------------------------------------------------------------
Position2D Renderer::metersToNDC(float x_m, float y_m) const {
    const float Wm = fbW / ppm, Hm = fbH / ppm;
    return Position2D{ 
        x_m / (Wm * 0.5f),
        y_m / (Hm * 0.5f)
    };
};


// Position2D Renderer::rectSizeToNDC(float length_m, float width_m) const {
//     return Position2D{ 2.0f * length_m * ppm / fbW, 2.0f * width_m  * ppm / fbH };}
// convcrts a full size (meters) into an NDC full size. for uScale
// ------------------------------------------------------------------------
Position2D Renderer::rectSizeToNDC(float width_m, float length_m) const {
    return Position2D{ 
        (2.0f * width_m * ppm / fbW), 
        (2.0f * length_m * ppm / fbH)
    };
};
