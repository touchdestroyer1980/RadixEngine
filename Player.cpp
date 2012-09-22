#include <math.h>
#include <stdio.h>
#include <GL/glut.h>
#include "Player.hpp"

#define RADDEG 57.29577951308232088 // 180/PI
#define PLAYER_MOVESPEED 6.0
#define GRAVITY 16.0
#define MAXSPEED 12.0
#define JUMPPOWER 7.0

Player::Player() {
	x = y = z = 0.f;
	xrot = yrot = 0.f;
	onGround = false;

	portals[0].set(1.75,1.45,0,PD_FRONT);
	portals[1].set(0,2.45,1.75,PD_RIGHT);
}

void Player::create(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
	onGround = false;
}

void Player::update(float dt, bool *keystates, float mousedx, float mousedy, Map &map) {
	// Apply rotation of view
	yrot += mousedx*0.0015f;
	xrot += mousedy*0.0015f;

	// Restrict rotation in x-axis
	if(xrot < -1.5f) xrot = -1.5f;
	if(xrot >  1.5f) xrot =  1.5f;

	// Reset x and z speed
	xspeed = zspeed = 0.f;
	// Apply gravity to yspeed
	yspeed -= GRAVITY*dt;
	if(yspeed < -MAXSPEED) yspeed = -MAXSPEED;

	// Move player
	if(keystates['w']) {
		zspeed -= cos(yrot)*PLAYER_MOVESPEED;
		xspeed -= sin(yrot)*PLAYER_MOVESPEED;
	}
	if(keystates['s']) {
		zspeed += cos(yrot)*PLAYER_MOVESPEED;
		xspeed += sin(yrot)*PLAYER_MOVESPEED;
	}
	if(keystates['a']) {
		xspeed -= cos(yrot)*PLAYER_MOVESPEED;
		zspeed += sin(yrot)*PLAYER_MOVESPEED;
	}
	if(keystates['d']) {
		xspeed += cos(yrot)*PLAYER_MOVESPEED;
		zspeed -= sin(yrot)*PLAYER_MOVESPEED;
	}
	// Jump is space is pressed and player is standing on ground
	if(keystates[' '] && onGround) {
		yspeed = JUMPPOWER;
	}

	float newx = x + xspeed*dt;
	float newy = y + yspeed*dt;
	float newz = z + zspeed*dt;

	Box xbox(newx-0.5, y, z-0.5, newx+0.5, y+1.8, z+0.5);
	if(map.collidesWithWall(xbox) == false) {
		x = newx;
	}

	Box zbox(x-0.5, y, newz-0.5, x+0.5, y+1.8, newz+0.5);
	if(map.collidesWithWall(zbox) == false) {
		z = newz;
	}

	Box ybox(x-0.5, newy, z-0.5, x+0.5, newy+1.8, z+0.5);
	onGround = false;
	if(map.collidesWithWall(ybox) == false) {
		y = newy;
	} else {
		if(yspeed < 0) {
			onGround = true;
		}
		yspeed = 0.f;
	}

	// Update shots
	for(int i = 0; i < 2; i++) {
		if(shots[i].active) shots[i].update(dt);

		Box sbox;
		if(map.pointInWall(shots[i].x, shots[i].y, shots[i].z, &sbox)) {
			shots[i].placePortal(sbox, portals[i]);
		}
	}
}

void Player::mousePressed(int button) {
	switch(button) {
		case GLUT_LEFT_BUTTON:
			// Shoot blue portal
			shots[0].shoot(0,x,y,z,xrot,yrot);
			break;
		case GLUT_RIGHT_BUTTON:
			// Shoot orange portal
			shots[1].shoot(1,x,y,z,xrot,yrot);
			break;
		case GLUT_MIDDLE_BUTTON:
			// Disable both portals
			portals[0].active = false;
			portals[1].active = false;
			break;
	}
}

void Player::setView() {
	glRotatef(-xrot*RADDEG, 1,0,0);
	glRotatef(-yrot*RADDEG, 0,1,0);
	glTranslatef(-x, -(y+1.7f), -z);
}

bool Player::portalsActive() {
	return (portals[0].active && portals[1].active);
}

void Player::drawPortalStencils() {
	if(portals[0].active) portals[0].drawStencil();
	if(portals[1].active) portals[1].drawStencil();
}

void Player::drawPortalOutlines(GLuint *textures) {
	glEnable(GL_BLEND);
	if(portals[0].active) portals[0].drawOutline(PC_BLUE,   textures);
	if(portals[1].active) portals[1].drawOutline(PC_ORANGE, textures);
	glDisable(GL_BLEND);
}

void Player::drawShots(GLuint *textures) {
	if(shots[0].active) shots[0].draw(textures, xrot, yrot);
	if(shots[1].active) shots[1].draw(textures, xrot, yrot);
}
