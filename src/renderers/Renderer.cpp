#include "Renderer.h"
#include "../entities/Entity.h"
#include "../shaders/RectShader.h"
#include "../Loader.h"


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
    const State ndcPos = metersToNDC(e.getPosX(), e.getPosY());
    const State ndcSize = rectSizeToNDC(e.getWidth(), e.getHeight());

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
State Renderer::metersToNDC(float x_m, float y_m) const {
    const float Wm = fbW / ppm, Hm = fbH / ppm;
    return State{ 
        x_m / (Wm * 0.5f),
        y_m / (Hm * 0.5f)
    };
};

// convcrts a full size (meters) into an NDC full size. for uScale
// ------------------------------------------------------------------------
State Renderer::rectSizeToNDC(float width_m, float height_m) const {
    return State{ 
        (2.0f * width_m * ppm / fbW), 
        (2.0f * height_m * ppm / fbH)
    };
};
