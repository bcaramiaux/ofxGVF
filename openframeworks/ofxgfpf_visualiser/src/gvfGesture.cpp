//
//  gfpfGesture.cpp
//  ofGfpfVisualiser
//
//  Created by Igor Correa on 03/07/2013.
//
//

#include "gfpfGesture.h"

gfpfGesture::gfpfGesture(){}

gfpfGesture::gfpfGesture(ofRectangle _drawArea)
{
    setDrawArea(_drawArea);
}

gfpfGesture::~gfpfGesture(){}

void gfpfGesture::initialise(ofPoint _initialPoint)
{
    setInitialPoint(_initialPoint);
    data.clear();
}

void gfpfGesture::initialise(float x, float y)
{
    initialise(ofPoint(x, y));
}


void gfpfGesture::initialiseNonNormalised(ofPoint _initialPoint)
{
    initialise(normalisePoint(_initialPoint.x, _initialPoint.y));
}

void gfpfGesture::initialiseNonNormalised(float x, float y)
{
    initialiseNonNormalised(ofPoint(x, y));
}

void gfpfGesture::addNonNormalisedPoint(float x, float y)
{
    // the point is first normalised and then translated using the initial point
    ofPoint p = normalisePoint(x, y);
    p[0] = 0.5 + (p[0] - initialPoint[0]) * 0.5;
    p[1] = 0.5 + (p[1] - initialPoint[1]) * 0.5;
    addPoint(p);
}
void gfpfGesture::addPoint(ofPoint p)
{
    std::vector<float> pointVf(2);
    pointVf[0] = p.x;
    pointVf[1] = p.y;
    data.push_back(pointVf);
}

void gfpfGesture::draw()
{
    draw(drawArea, 1.0f);
}

void gfpfGesture::draw(float scale)
{
    draw(drawArea, scale);
}


void gfpfGesture::draw(ofRectangle drawArea)
{
    draw(drawArea, 1.0f);
}

void gfpfGesture::draw(ofRectangle drawArea, float scale)
{
    ofSetColor(150, 150, 150);
    ofNoFill();
    ofSetLineWidth(2);
    
    // draw border if needed
    if(drawBoundaries)
        ofRect(drawArea);

    // the amount of points
    int gestureSize = data.size();

    if(gestureSize > 1)
    {
        ofPoint prevPoint(data[0][0],data[0][1]);

        ofEnableSmoothing();
        ofEnableAlphaBlending();
        ofSetLineWidth(lineWidth);
        static float counter = 0;
        
        ofPoint firstPoint;
        
        // if the drawing is centralised, ignore the initialPoint
        if(centraliseDrawing)
            firstPoint = ofPoint(1.5, 1.5);
        else
            firstPoint = initialPoint;
        
        // the gesture will be drawn as a mesh. Each point will be a vertex
        ofMesh gestureMesh;
        gestureMesh.setMode(OF_PRIMITIVE_LINE_STRIP);

        for(int i = 1; i < gestureSize; i++)
        {
            ofPoint point(data[i][0], data[i][1]);

            // if the distance between two points is large, the line between them will be thinner
            float thinness = ofNormalize(point.distance(prevPoint), 0.01, 0.15);
            
            // the thinner the line, the more transparent it is
            color.a = (maxAlpha - (thinness * (maxAlpha - minAlpha))) * alpha;
            ofSetColor(color);

            // the previously normalised points are scaled and translated before being added to the mesh
            float ix = (((prevPoint.x * scale - 0.5) * 2) + firstPoint[0]) * greatestDimension + drawArea.x;
            float iy = (((prevPoint.y * scale - 0.5) * 2) + firstPoint[1]) * greatestDimension + drawArea.y;

            gestureMesh.addColor(color);
            gestureMesh.addVertex(ofVec2f(ix, iy));
            
            // we always keep track of two points, in order to calculate the thinness
            prevPoint = point;
        }

        // the last point is calculated and added
        float ix = (((prevPoint.x * scale - 0.5) * 2) + firstPoint[0]) * greatestDimension + drawArea.x;
        float iy = (((prevPoint.y * scale - 0.5) * 2) + firstPoint[1]) * greatestDimension + drawArea.y;

        gestureMesh.addColor(color);
        gestureMesh.addVertex(ofVec2f(ix, iy));

        // draw the "mesh" (line)
        ofEnableSmoothing();
        gestureMesh.draw();

    
        ofDisableSmoothing();
        ofDisableAlphaBlending();
    }


}

bool gfpfGesture::isPointInGestureArea(float x, float y)
{
    return isPointInArea(drawArea, x, y);
}

bool gfpfGesture::isPointInArea(ofRectangle area, float x, float y)
{
    return (x > area.x && x < area.x + area.width &&
            y > area.y && y < area.y + area.height);
}

// sets

void gfpfGesture::setData(std::vector<std::vector<float> > _data)
{
    data.clear();
    data = _data;
}

void gfpfGesture::setDrawArea(ofRectangle _drawArea)
{
    drawArea = _drawArea;
    isValid = true;
    
    greatestDimension = (drawArea.width > drawArea.height)?
                         drawArea.width:
                         drawArea.height;
}

void gfpfGesture::setInitialPoint(ofPoint _initialPoint)
{
    initialPoint = _initialPoint;
}

void gfpfGesture::setAppearance(ofColor _color = ofColor(240, 30, 30),
                                float _lineWidth = 2,
                                float _maxAlpha = 255,
                                float _minAlpha = 50,
                                float _alpha = 1)
{
    color = _color;
    lineWidth = _lineWidth;
    maxAlpha = _maxAlpha;
    minAlpha = _minAlpha;
    alpha = _alpha;
}

void gfpfGesture::setColor(ofColor _color)
{
    color = _color;
}

// gets

ofPoint gfpfGesture::getInitialOfPoint()
{
    return initialPoint;
}

ofRectangle gfpfGesture::getDrawArea()
{
    return drawArea;
}

std::vector<std::vector<float> > gfpfGesture::getData(){
    return data;
}

ofPoint gfpfGesture::getLastPointAdded()
{
    return ofPoint(data.back()[0],data.back()[1]);
}

float gfpfGesture::getGreatestDimension()
{
    return greatestDimension;
}

ofColor gfpfGesture::getColor()
{
    return color;
}

// private
ofPoint gfpfGesture::normalisePoint(float x, float y)
{
    ofPoint p;
    
    p.x = (x - drawArea.x)/greatestDimension;
    p.y = (y - drawArea.y)/greatestDimension;
    return p;
}



