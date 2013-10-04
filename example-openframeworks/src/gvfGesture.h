//
//  ofGesture.h
//  ofgvfVisualiser
//
//  Created by Igor Correa on 03/07/2013.
//
//

#ifndef __ofgvfVisualiser__ofGesture__
#define __ofgvfVisualiser__ofGesture__

#include <iostream>
#include "ofMain.h"

#endif /* defined(__ofgvfVisualiser__ofGesture__) */

// the gvfGesture class is used to represent the gesture throughout the application.
// it contains the coordinates of all the points the form the gesture and also the coordinates of a special point, the initial point.
// a gvfGesture should also contain all the information needed in order to draw the gesture to the screen.
// all gestures on the application should be drawn by calling the draw method of this class.
// the coordinates of the points that form the gesture always start at 0.5 and can reach the extremes 0.0 and 1.0.
// the initial point has coordinates between 0.0 and 1.0 by which all the other points on the gesture will be translated when the gesture drawn.
// the initial point can represent the position on the screen where the gesture was first drawn or can represent any other arbitrary point.
class gvfGesture
{
    public:
        gvfGesture();
        gvfGesture(ofRectangle _drawArea);
        ~gvfGesture();
    
        // the initial point is required to initialise the gesture
        // 4 methods are provided:
    
        // the point passed should be normalised
        void initialise(ofPoint _initialPoint); // coordinates range: [0,1]
        void initialise(float x, float y); // x and y range: [0,1]

        // the point passed does not need to be normalised
        // (the draw area previously set will be used to normalise the point)
        void initialiseNonNormalised(ofPoint _initialPoint);
        void initialiseNonNormalised(float x, float y);
    
        // used to add a new point to gesture
        void addNonNormalisedPoint(float x, float y);
        void addPoint(ofPoint p);

        // a few options are provided to draw
        void draw();
        void draw(float scale);
        void draw(ofRectangle drawArea);
        void draw(ofRectangle drawArea, float scale);
    
        bool isPointInGestureArea(float x, float y);
        bool isPointInArea(ofRectangle area, float x, float y);
    
        // ----------- sets -------------
    
        // all the points on the gesture can be set at once with this method
        void setData(std::vector<std::vector<float> > _data);
    
        // sets the area where the gesture should be drawn
        void setDrawArea(ofRectangle _drawArea);
    
        // sets the point by which all the other points are goning to be translated when drawing
        void setInitialPoint(ofPoint _initialPoint);
    
        // sets visual properties
        // the transparency of each line forming the gesture will vary depending on the distance between each two points:
        // if the distance is very small, it will be _maxAlpha [0, 255], if it is very large, it will be _minAlpha [0, 255].
        // the _alpha value will be used to weight the previously calculated alpha
        // (if _alpha is 0, the final alpha will be 0, if _alpha is 1, the final apha will be the value previously calculated)
        void setAppearance(ofColor _color, float _lineWidth, float _maxAlpha, float _minAlpha, float _alpha);
        void setColor(ofColor _color);

        // ----------- gets ------------
        // multiple options to get the initial point
        ofPoint getInitialOfPoint();
        std::vector<float> getInitialVfPoint();
    
        ofRectangle getDrawArea();
    
        // returns the data representing all the points
        std::vector<std::vector<float> > getData();

        ofPoint getLastPointAdded();

        float getGreatestDimension();
    
        ofColor getColor();
    
        // a gesture is considered valid only when its draw area is set
        bool isValid = false;
    
        // when true, the initial point will be ignored and the first point will be the center of the draw area
        bool centraliseDrawing = false;
    
        // when true, a border representing the draw area will be visible
        bool drawBoundaries = false;
    
    private:
        // uses the draw area to normalise the point given
        ofPoint normalisePoint(float x, float y);
    
        // the rectangle on the screen where the gesture will be drawn
        ofRectangle drawArea;
    
        // this value is used in order correctly normalise the points
        // without distortion even if the aspect ratio of the screen is not 1:1, which is usualy the case.
        float greatestDimension;
    
        // the initial point has coordinates between 0.0 and 1.0
        // by which all the other points on the gesture will be translated when the gesture drawn.
        ofPoint initialPoint;
    
        // where all the points that form the gesture will be stored
        std::vector<std::vector<float> > data;
    
        ofColor color = ofColor(240, 30, 30, 255);
        float lineWidth = 2;
        float maxAlpha = 255;
        float minAlpha = 50;
        float alpha = 1;
};