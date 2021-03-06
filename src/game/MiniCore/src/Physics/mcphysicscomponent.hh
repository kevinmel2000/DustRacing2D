// This file belongs to the "MiniCore" game engine.
// Copyright (C) 2015 Jussi Lind <jussi.lind@iki.fi>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//

#ifndef MCPHYSICSCOMPONENT_HH
#define MCPHYSICSCOMPONENT_HH

#include "mcobjectcomponent.hh"
#include "mcvector3d.hh"

/** Implements physics integrations of an MCObject.
 *  The physics component is attached to an object and it operates
 *  through the public interface. */
class MCPhysicsComponent : public MCObjectComponent
{
public:

    //! Constructor.
    MCPhysicsComponent();

    //! Destructor.
    virtual ~MCPhysicsComponent();

    /*! \brief Add an impulse.
     *  \param impulse The velocity component of the impulse. */
    virtual void addImpulse(const MCVector3dF & impulse, bool isCollision = false);

    /*! \brief Add an impulse.
     *  Causes angular impulse depending on object's shape and position of the impulse vector.
     *  \param impulse The velocity component of the impulse.
     *  \param pos     Position of the impulse. */
    virtual void addImpulse(const MCVector3dF & impulse, const MCVector3dF & pos, bool isCollision = false);

    //! Add rotational impulse in rad/s.
    virtual void addAngularImpulse(float impulse, bool isCollision = false);

    /*! Set current velocity to the given value.
     *  \param newVelocity The new velocity. */
    void setVelocity(const MCVector3dF & newVelocity);

    //! Return current velocity.
    const MCVector3dF & velocity() const;

    //! Return current speed.
    float speed() const;

    //! Set maximum speed.
    void setMaxSpeed(float maxSpeed);

    /*! Set current angular velocity to the given value.
     *  Unit: rad/s.
     *  \param newVelocity The new velocity. */
    void setAngularVelocity(float newVelocity);

    /*! Return current angular velocity.
     *  Unit: rad/s. */
    float angularVelocity() const;

    /*! Set constant acceleration (e.g. gravity). Use addForce()
     *  to cause acceleration that varies from frame to frame.
     *  \param newAcceleration The new acceleration. */
    void setAcceleration(const MCVector3dF & newAcceleration);

    //! Return constant acceleration.
    const MCVector3dF & acceleration() const;

    /*! Add a force (N) vector to the object for a single frame.
     *  \param force Force vector to be added. */
    void addForce(const MCVector3dF & force);

    /*! Add a force (N) vector to the object for a single frame.
     *  \param force Force vector to be added.
     *  \param pos Position of the force (in world coordinates). */
    void addForce(const MCVector3dF & force, const MCVector3dF & pos);

    /*! Add torque (Nm) to the object for a single frame.
     *  \param torque Value to be added. */
    void addTorque(float torque);

    //! Clear accumulated forces.
    void clearForces();

    /*! Set mass.
     *  \param newMass The new mass.
     *  \param stationary Sets inverse mass to zero if true.
     *         This results in "infinite" mass for stationary objects. */
    void setMass(float newMass, bool stationary = false);

    //! Get inverse mass.
    float invMass() const;

    //! Get mass.
    float mass() const;

    //! Set moment of inertia.
    void setMomentOfInertia(float momentOfInertia);

    //! Get moment of inertia.
    float momentOfInertia() const;

    //! Get inverse moment of inertia.
    float invMomentOfInertia() const;

    /*! \brief Set restitution.
     *  \param newRestitution The new restitution [0.0..1.0]
     *  (0.0 means a totally "soft" object, 1.0 means hard).
     *  Default is 0.5. */
    void setRestitution(float newRestitution);

    //! Get restitution.
    float restitution() const;

    /*! Set global friction. A friction coeff > 0 results in a
     *  implicit creation of a MCFrictionGenarator when the
     *  object is being added to the world.
     *  Use a custom friction generator if a more controlled
     *  friction is wanted.
     *  DEPRECATED
     *  \see setLinearDamping();
     *  \see setAngularDamping(); */
    void setXYFriction(float friction);

    //! Get global friction.
    float xyFriction() const;

    //! Return true, if the object is sleeping.
    bool isSleeping() const;

    /*! Object goes to sleep if linear and angular velocities drops below
     *  these values. The defaults are 0.01. */
    void setSleepLimits(float linearSleepLimit, float angularSleepLimit);

    //! The object won't sleep if enabled.
    void preventSleeping(bool flag);

    void toggleSleep(bool state);

    //! \return true if object is stationary.
    bool isStationary() const;

    //! \return true if integration is in progress.
    bool isIntegrating() const;

    //! Reset Z-component.
    void resetZ();

    /*! Set an optional "tag" used in collision detection. This can be used to
     *  efficiently filter collisions before sending collision events to objects.
     *  \see setNeverCollideWithTag() */
    void setCollisionTag(int tag);

    //! \return Collision tag.
    int collisionTag() const;

    /*! Collisions with objects having the given tag are never processed.
     *  \see setCollisionTag() */
    void setNeverCollideWithTag(int tag);

    //! \return tag with which collision are filtered out.
    int neverCollideWithTag() const;

    //! Set damping factor for angular motion. Default is 0.999.
    void setAngularDamping(float angularDamping);

    //! Set damping factor for linear motion. Default is 0.999.
    void setLinearDamping(float linearDamping);

    //! \reimp
    virtual void stepTime(int step) override;

    //! \reimp
    virtual void reset() override;

private:

    void integrate(float step);

    void integrateLinear(float step);

    float integrateAngular(float step);

    float m_damping;

    MCVector3dF m_acceleration;

    MCVector3dF m_velocity;

    float m_maxSpeed;

    float m_linearDamping;

    MCVector3dF m_linearImpulse;

    MCVector3dF m_forces;

    float m_angularAcceleration; // Radians / s^2

    float m_angularVelocity; // Radians / s

    float m_angularDamping;

    float m_angularImpulse;

    float m_torque;

    float m_invMass;

    float m_mass;

    float m_invMomentOfInertia;

    float m_momentOfInertia;

    float m_restitution;

    float m_xyFriction;

    bool m_isSleeping;

    bool m_isSleepingPrevented;

    bool m_isStationary;

    bool m_isIntegrating;

    float m_linearSleepLimit;

    float m_angularSleepLimit;

    int m_sleepCount;

    int m_collisionTag;

    int m_neverCollideWithTag;
};

#endif // MCPHYSICSCOMPONENT_HH
