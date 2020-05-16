//
//  Created by Lukasz Karluk on 14/09/2015.
//
//

#include "ofxPoly.h"

//--------------------------------------------------------------
void ofxPolyGrow(ofPolyline & poly, const ofPolyline & polySource, float amount) {
    
    poly.clear();
    poly.setClosed(polySource.isClosed());
    
    if(polySource.size() < 2) {
        poly = polySource;
        return;
    }
    
    int numPoints = polySource.getVertices().size();
    auto & points = polySource.getVertices();
    
    bool bClosed = true;
    bClosed = bClosed && (polySource.isClosed() == true);
    bClosed = bClosed && (numPoints >= 3);
    
    for(int i=0; i<numPoints; i++) {
        
        bool bEndings = false;
        bEndings = bEndings || (i == 0);
        bEndings = bEndings || (i == numPoints-1);
        bEndings = bEndings && (bClosed == false);
        
        if(bEndings == true) {
            const glm::vec3 & p0 = points[i];
            glm::vec3 n0 = polySource.getNormalAtIndex(i);
            glm::vec3 point = p0 + (n0 * amount);
            poly.addVertex(point);
            
            continue;
        }
        
        int i0 = i-1;
        if(i0 < 0) {
            i0 += numPoints;
        }
        
        const glm::vec3 & p0 = points[i0];
        const glm::vec3 & p1 = points[i];
        ofVec3f n0 = ofVec2f(p0 - p1).getPerpendicular();
        ofVec3f n1 = polySource.getNormalAtIndex(i);
        
        float angle = ofVec2f(n0).angle(ofVec2f(n1));
        float length = amount / cos(angle * DEG_TO_RAD);
        
        glm::vec3 point = p1 + (n1 * length);
        poly.addVertex(point);
    }
    
}

//--------------------------------------------------------------
void ofxPolyGrowAlongNormals(ofPolyline & poly, const ofPolyline & polySource, float normalLength) {
    
    vector<float> thicknesses;
    thicknesses.insert(thicknesses.begin(), polySource.size(), normalLength);
    ofxPolyGrowAlongNormals(poly, polySource, thicknesses);
    
}

//--------------------------------------------------------------
void ofxPolyGrowAlongNormals(ofPolyline & poly, const ofPolyline & polySource, const vector<float> & normalLengths) {
    
    poly = polySource;
    
    if(poly.size() < 2) {
        return;
    }
    
    int numPoints = poly.getVertices().size();
    auto & points = poly.getVertices();
    
    for(int i=0; i<numPoints; i++) {
        
        float normalLength = 0.0;
        if(i < normalLengths.size()) {
            normalLength = normalLengths[i];
        }
        
        glm::vec3 & point = points[i];
        glm::vec3 normal = poly.getNormalAtIndex(i);
        point += (normal * normalLength);
    }
    
}

//--------------------------------------------------------------
void ofxPolyToMesh(ofMesh & mesh, const ofPolyline & polySource, float normalLength) {
    
    float normalLength0 = -normalLength;
    float normalLength1 = normalLength;
    
    ofPolyline poly0, poly1;
    ofxPolyGrowAlongNormals(poly0, polySource, normalLength0);
    ofxPolyGrowAlongNormals(poly1, polySource, normalLength1);
    
    ofxPolyToMesh(mesh, poly0, poly1);
    
}

//--------------------------------------------------------------
void ofxPolyToMesh(ofMesh & mesh, const ofPolyline & polySource, const vector<float> & normalLengths) {
    
    vector<float> normalLengths0;
    vector<float> normalLengths1;
    
    for(int i=0; i<normalLengths.size(); i++) {
        float normalLength = normalLengths[i];
        float normalLength0 = -normalLength;
        float normalLength1 = normalLength;
        
        normalLengths0.push_back(normalLength0);
        normalLengths1.push_back(normalLength1);
    }
    
    ofPolyline poly0, poly1;
    ofxPolyGrowAlongNormals(poly0, polySource, normalLengths0);
    ofxPolyGrowAlongNormals(poly1, polySource, normalLengths1);
    
    ofxPolyToMesh(mesh, poly0, poly1);
    
}

//--------------------------------------------------------------
void ofxPolyToMesh(ofMesh & mesh, const ofPolyline & poly0, const ofPolyline & poly1) {
    
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    
    int numPoints = MIN(poly0.size(), poly1.size());
    int numCycles = numPoints;
    if(poly0.isClosed() == true) {
        numCycles += 1;
    }
    
    for(int i=0; i<numCycles; i++) {
        int j = i % numPoints;
        const glm::vec3 & p0 = poly0.getVertices()[j];
        const glm::vec3 & p1 = poly1.getVertices()[j];
        
        mesh.addVertex(p0);
        mesh.addVertex(p1);
    }
    
}

//--------------------------------------------------------------
void ofxPolyDrawNormals(const ofPolyline & poly, float normalLength) {
    
    auto & points = poly.getVertices();
    
    for(int i=0; i<points.size(); i++) {
        const glm::vec3 & point = points[i];
        glm::vec3 normal = poly.getNormalAtIndex(i);
        
        ofDrawLine(point, point + (normal * normalLength));
    }
    
}

//--------------------------------------------------------------
void ofxPolySave(const ofPolyline & poly, string xmlPath) {
    
    ofXml xml;
    auto polylineXml = xml.appendChild("poly");
    xml.setAttribute("closed", ofToString(poly.isClosed()));
    
    for(unsigned int i=0; i<poly.size(); i++) {
        const glm::vec3 & point = poly.getVertices()[i];
        auto pointXml = polylineXml.appendChild("point");
        pointXml.setAttribute("x", int(point.x));
        pointXml.setAttribute("y", int(point.y));
    }
    
    xml.save(xmlPath);
    
}

//--------------------------------------------------------------
void ofxPolyLoad(ofPolyline & poly, string xmlPath) {
    
    ofXml xml;
    bool bLoaded = xml.load(xmlPath);
    if(bLoaded == false) {
        return;
    }
    
    xml.getChild("poly");
    
    bool bClosed = xml.getAttribute("closed").getIntValue();
    
    poly.clear();
    
    auto pointXml = xml.find("//poly/point");
    for(auto & point: pointXml) {
        auto x = point.getAttribute("x").getIntValue();
        auto y = point.getAttribute("y").getIntValue();
        
        poly.addVertex(x, y);
    }
    
    poly.setClosed(bClosed);
    
}
