#include "Object.h"

Object::Object(void)
{

}

Object::~Object(void)
{

}

Object::Object(string name){

	setType(name);

	if(name=="Block Bar"){
	
        setHSVmin(Scalar(164,122,51));
        setHSVmax(Scalar(212,248,148));

        //setHSVmin(Scalar(158,50,190));
        //setHSVmax(Scalar(180,255,255));

	setColour(Scalar(0,255,0));

	}

	if(name=="traffic_light"){
	
	setHSVmin(Scalar(0,44,167));
	setHSVmax(Scalar(40,100,255));

	setColour(Scalar(255,0,0));

	}

}

int Object::getXPos(){
        return Object::xPos;
}

void Object::setXPos(int x){
	Object::xPos = x;
}

int Object::getYPos(){
	return Object::yPos;
}

void Object::setYPos(int y){
	Object::yPos = y;
}

Scalar Object::getHSVmin(){
	return Object::HSVmin;
}

void Object::setHSVmin(Scalar min){
	Object::HSVmin = min;
}

Scalar Object::getHSVmax(){
	return Object::HSVmax;
}

void Object::setHSVmax(Scalar max){
	Object::HSVmax = max;
}
