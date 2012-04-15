// This file is part of Dust Racing (DustRAC).
// Copyright (C) 2011 Jussi Lind <jussi.lind@iki.fi>
//
// DustRAC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// DustRAC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DustRAC. If not, see <http://www.gnu.org/licenses/>.

#include "car.hpp"
#include "centrifugalforcegenerator.hpp"
#include "layers.hpp"
#include "radius.hpp"
#include "slidefrictiongenerator.hpp"
#include "MiniCore/Core/MCCollisionEvent"
#include "MiniCore/Core/MCFrictionGenerator"
#include "MiniCore/Core/MCGLRectParticle"
#include "MiniCore/Core/MCRandom"
#include "MiniCore/Core/MCRectShape"
#include "MiniCore/Core/MCShape"
#include "MiniCore/Core/MCSurface"
#include "MiniCore/Core/MCTextureManager"
#include "MiniCore/Core/MCTrigonom"
#include "MiniCore/Core/MCTypes"
#include "MiniCore/Core/MCVector2d"
#include "MiniCore/Core/MCMathUtil"

namespace
{
    const MCFloat MAX_LINEAR_VELOCITY  = 15.0f;
    const MCFloat MAX_ANGULAR_VELOCITY = 10.0f;
    const MCFloat FRICTION             = 0.5f;
    const MCFloat BRAKING_FRICTION     = 1.0f;
    const MCFloat ROLLING_FRICTION     = 0.1f;
    const MCFloat ROTATION_FRICTION    = 0.5f;
    const MCFloat OFF_TRACK_FRICTION   = 0.5f;
    const MCFloat OFF_TRACK_MOMENT     = 50000.0f;

    const MCVector2dF LEFT_FRONT_TIRE_POS(20, 13);
    const MCVector2dF RIGHT_FRONT_TIRE_POS(20, -13);
    const MCVector2dF LEFT_REAR_TIRE_POS(-20, 13);
    const MCVector2dF RIGHT_REAR_TIRE_POS(-20, -13);
}

Car::Car(MCSurface & surface, MCUint index)
  : MCObject(&surface, "Car")
  , m_pBrakingFriction(new MCFrictionGenerator(BRAKING_FRICTION, 0.0f))
  , m_pOffTrackFriction(new MCFrictionGenerator(OFF_TRACK_FRICTION, 0))
  , m_leftSideOffTrack(false)
  , m_rightSideOffTrack(false)
  , m_accelerating(false)
  , m_braking(false)
  , m_reverse(false)
  , m_turnLeft(false)
  , m_turnRight(false)
  , m_index(index)
  , m_tireAngle(0)
  , m_frontTire(MCTextureManager::instance().surface("frontTire"))
  , m_brakeGlow(MCTextureManager::instance().surface("brakeGlow"))
  , m_power(5000.0f)
  , m_turningImpulse(.40f)
  , m_speedInKmh(0)
  , m_dx(0)
  , m_dy(0)
{
    setLayer(Layers::Cars);
    setMass(1000);
    setMomentOfInertia(1000 * 10);
    setMaximumVelocity(MAX_LINEAR_VELOCITY);
    setMaximumAngularVelocity(MAX_ANGULAR_VELOCITY);
    setShadowOffset(MCVector2d<MCFloat>(5, -5));
    setRestitution(0.1f);

    // Add slide friction generator
    MCWorld::instance().addForceGenerator(*new SlideFrictionGenerator(FRICTION), *this, true);

    // Add rolling friction generator
    MCWorld::instance().addForceGenerator(
        *new MCFrictionGenerator(ROLLING_FRICTION, ROTATION_FRICTION), *this, true);

    // Add braking friction generator
    MCWorld::instance().addForceGenerator(*m_pBrakingFriction, *this, true);
    m_pBrakingFriction->enable(false);

    // Add off-track friction generator
    MCWorld::instance().addForceGenerator(*m_pOffTrackFriction, *this, true);
    m_pOffTrackFriction->enable(false);

    // Add centrifugal force generator
    const MCFloat amplification = 5.0f;
    MCWorld::instance().addForceGenerator(
        *new CentrifugalForceGenerator(amplification), *this, true);

    const MCFloat width  = static_cast<MCRectShape *>(shape())->width();
    const MCFloat height = static_cast<MCRectShape *>(shape())->height();
    m_length = std::max(width, height);

    //setRenderShapeOutline(true); // FOR DEBUG
}

void Car::setPower(MCFloat power)
{
    m_power = power;
}

void Car::clearStatuses()
{
    m_braking      = false;
    m_accelerating = false;
}

MCUint Car::index() const
{
    return m_index;
}

void Car::turnLeft()
{
    if (m_tireAngle < 45) m_tireAngle++;

    m_turnLeft = true;

    if (std::abs(m_speedInKmh) > 1)
    {
        addRotationalImpulse(m_turningImpulse);
    }
}

void Car::turnRight()
{
    if (m_tireAngle > -45) m_tireAngle--;

    m_turnRight = true;

    if (std::abs(m_speedInKmh) > 1)
    {
        addRotationalImpulse(-m_turningImpulse);
    }
}

void Car::accelerate()
{
    m_pBrakingFriction->enable(false);

    MCVector2d<MCFloat> force(m_dx, m_dy);
    addForce(force * m_power);

    m_accelerating = true;
    m_braking      = false;
    m_reverse      = false;
}

void Car::brake()
{
    m_accelerating = false;

    if (m_speedInKmh < 1)
    {
        m_reverse = true;
    }

    if (m_reverse)
    {
        MCVector2d<MCFloat> force(m_dx, m_dy);
        addForce(-force * m_power / 2);
    }
    else
    {
        m_braking = true;
        m_pBrakingFriction->enable(true);
    }
}

void Car::noAction()
{
    m_pBrakingFriction->enable(false);

    m_accelerating = false;
    m_braking      = false;
    m_reverse      = false;
}

void Car::noSteering()
{
    if (m_tireAngle < 0)
    {
        m_tireAngle++;
    }
    else if (m_tireAngle > 0)
    {
        m_tireAngle--;
    }

    m_turnLeft = false;
    m_turnRight = false;
}

int Car::speedInKmh() const
{
    return m_speedInKmh;
}

MCVector3dF Car::leftFrontTireLocation() const
{
    MCVector2dF pos;
    MCTrigonom::rotated(LEFT_FRONT_TIRE_POS, pos, angle());
    return pos + MCVector2dF(location());
}

MCVector3dF Car::rightFrontTireLocation() const
{
    MCVector2dF pos;
    MCTrigonom::rotated(RIGHT_FRONT_TIRE_POS, pos, angle());
    return pos + MCVector2dF(location());
}

MCVector3dF Car::leftRearTireLocation() const
{
    MCVector2dF pos;
    MCTrigonom::rotated(LEFT_REAR_TIRE_POS, pos, angle());
    return pos + MCVector2dF(location());
}

MCVector3dF Car::rightRearTireLocation() const
{
    MCVector2dF pos;
    MCTrigonom::rotated(RIGHT_REAR_TIRE_POS, pos, angle());
    return pos + MCVector2dF(location());
}

void Car::render(MCCamera *p)
{
    // Render left front tire
    const MCVector3dF leftFrontTire(leftFrontTireLocation());
    m_frontTire.render(p, leftFrontTire, m_tireAngle + angle());

    // Render right front tire
    const MCVector3dF rightFrontTire(rightFrontTireLocation());
    m_frontTire.render(p, rightFrontTire, m_tireAngle + angle());

    // Render body
    MCObject::render(p);

    // Render brake lights
    if (m_braking)
    {
        static const MCVector2dF leftBrakeGlowPos(-36, 12);
        static const MCVector2dF rightBrakeGlowPos(-36, -12);

        MCVector2dF leftBrakeGlow;
        MCTrigonom::rotated(leftBrakeGlowPos, leftBrakeGlow, angle());
        leftBrakeGlow += MCVector2dF(location());
        m_brakeGlow.render(p, leftBrakeGlow, angle());

        MCVector2dF rightBrakeGlow;
        MCTrigonom::rotated(rightBrakeGlowPos, rightBrakeGlow, angle());
        rightBrakeGlow += MCVector2dF(location());
        m_brakeGlow.render(p, rightBrakeGlow, angle());

        doSkidMark(leftFrontTire, 0.25f, 0.25f, 0.25f, 0.5f);
        doSkidMark(rightFrontTire, 0.25f, 0.25f, 0.25f, 0.5f);

        doSmoke(leftFrontTire, 0.95f, 0.95f, 0.95f, 0.5f);
        doSmoke(rightFrontTire, 0.95f, 0.95f, 0.95f, 0.5f);
    }

    if (m_speedInKmh > 10)
    {
        if (m_leftSideOffTrack)
        {
            doSkidMark(leftFrontTire, 0.3f, 0.2f, 0.0f, 0.5f);
            doSmoke(leftFrontTire, 0.75f, 0.75f, 0.75f, 0.5f);

            if (rand() % 5 == 0)
            {
                doMud(leftRearTireLocation(), 0.3f, 0.2f, 0.0f, 0.9f);
            }
        }

        if (m_rightSideOffTrack)
        {
            doSkidMark(rightFrontTire, 0.3f, 0.2f, 0.0f, 0.5f);
            doSmoke(rightFrontTire, 0.75f, 0.75f, 0.75f, 0.5f);

            if (rand() % 5 == 0)
            {
                doMud(rightRearTireLocation(), 0.3f, 0.2f, 0.0f, 0.9f);
            }
        }
    }
}

void Car::collisionEvent(MCCollisionEvent & event)
{
    if (event.collidingObject().typeID() == typeID())
    {
        if (rand() % 10 == 0)
        {
            doSparkle(event.contactPoint(), 1.0f, 0.8f, 0.0f, 0.9f);
        }
    }

    event.accept();
}

void Car::stepTime()
{
    m_dx = MCTrigonom::cos(angle());
    m_dy = MCTrigonom::sin(angle());

    m_speedInKmh = velocity().dot(MCVector3d<MCFloat>(m_dx, m_dy, 0)) * 120 / 10;

    if (m_speedInKmh > 10)
    {
        if (m_leftSideOffTrack)
        {
            addMoment(OFF_TRACK_MOMENT);
        }

        if (m_rightSideOffTrack)
        {
            addMoment(-OFF_TRACK_MOMENT);
        }
    }
}

void Car::setLeftSideOffTrack(bool state)
{
    m_pOffTrackFriction->enable(state);
    m_leftSideOffTrack = state;
}

void Car::setRightSideOffTrack(bool state)
{
    m_pOffTrackFriction->enable(state);
    m_rightSideOffTrack = state;
}

void Car::setTurningImpulse(MCFloat impulse)
{
    m_turningImpulse = impulse;
}

void Car::doSmoke(MCVector3dFR location, MCFloat r, MCFloat g, MCFloat b, MCFloat a) const
{
    MCGLRectParticle & smoke = MCGLRectParticle::create();
    smoke.init(location, 5, 180);
    smoke.setAnimationStyle(MCParticle::Shrink);
    smoke.setColor(r, g, b, a);
    smoke.setVelocity(MCRandom::randomVector2d() * 0.1f);
    smoke.addToWorld();
}

void Car::doSkidMark(MCVector3dFR location, MCFloat r, MCFloat g, MCFloat b, MCFloat a) const
{
    MCGLRectParticle & skidMark = MCGLRectParticle::create();
    skidMark.init(location, 4, 720);
    skidMark.setAnimationStyle(MCParticle::FadeOut);
    skidMark.setColor(r, g, b, a);
    skidMark.rotate(angle());
    skidMark.setPhysicsObject(false);
    skidMark.addToWorld();
}

void Car::doMud(MCVector3dFR location, MCFloat r, MCFloat g, MCFloat b, MCFloat a) const
{
    MCGLRectParticle & mud = MCGLRectParticle::create();
    mud.init(location, 4, 120);
    mud.setAnimationStyle(MCParticle::Shrink);
    mud.setColor(r, g, b, a);
    mud.addToWorld();
    mud.setVelocity(velocity() * 0.5f + MCVector3dF(0, 0, 2.0f));
    mud.setAcceleration(MCVector3dF(0, 0, -10.0f));
}

void Car::doSparkle(MCVector3dFR location, MCFloat r, MCFloat g, MCFloat b, MCFloat a) const
{
    MCGLRectParticle & sparkle = MCGLRectParticle::create();
    sparkle.init(location, 2, 60);
    sparkle.setAnimationStyle(MCParticle::Shrink);
    sparkle.setColor(r, g, b, a);
    sparkle.addToWorld();
    sparkle.setVelocity(velocity() * 0.5f);
}

Car::~Car()
{
}
