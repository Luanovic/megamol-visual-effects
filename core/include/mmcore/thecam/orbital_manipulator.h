/*
 * orbital_manipulator.h
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VISUS).
 * All rights reserved.
 */

#ifndef ORBITAL_MANIPULATOR_H_INCLUDED
#define ORBITAL_MANIPULATOR_H_INCLUDED


#include "mmcore/thecam/utility/config.h"

#include "mmcore/thecam/manipulator_base.h"


namespace megamol {
namespace core {
namespace thecam {

/**
 * Implements an orbtial camera maniupulator.
 *
 * @tparam T The type of the camera to be manipulated.
 */
template <class T> class OrbitalManipulator : public manipulator_base<T> {

public:
    /** The type of the camera to be manipulated by the manipulator. */
    typedef typename manipulator_base<T>::camera_type camera_type;

    /** The mathematical traits of the camera. */
    typedef typename manipulator_base<T>::maths_type maths_type;

    // Typedef all mathematical types we need in the manipulator.
    typedef typename maths_type::point_type point_type;
    typedef typename maths_type::quaternion_type quaternion_type;
    typedef typename maths_type::screen_type screen_type;
    typedef typename maths_type::vector_type vector_type;
    typedef typename maths_type::world_type world_type;

    OrbitalManipulator(const point_type& rotCentre = point_type())
        : m_rot_cntr(rotCentre), m_orbit(1.0) {}

    /**
     * Finalises the instance.
     */
    ~OrbitalManipulator() = default;

    /**
     * Report that the mouse pointer has been dragged (moved while the
     * designated button was down) to the specified screen coordinates.
     *
     * @param x
     * @param y
     */
    void on_drag_rotate(const screen_type x, const screen_type y) {
        if (this->manipulating() && this->enabled()) {
            auto cam = this->camera();
            THE_ASSERT(cam != nullptr);

            if (this->m_last_sx != x || this->m_last_sy != y) {

                screen_type dx = x - m_last_sx;
                screen_type dy = y - m_last_sy;

                quaternion_type rot_lat;
                quaternion_type rot_lon;

                auto intial_pos = cam->eye_position();
                auto initial_orientation = cam->orientation();

                auto shifted_pos = intial_pos - m_rot_cntr;
                quaternion_type pos_rot(shifted_pos.x(), shifted_pos.y(), shifted_pos.z(), 0.0f);


                thecam::math::set_from_angle_axis(
                    rot_lon, dx * (3.14159265f / 180.0f), vector_type(0.0, 1.0, 0.0, 0.0));
                auto rot_lon_conj = thecam::math::conjugate(rot_lon);

                cam->orientation(rot_lon * initial_orientation);
                initial_orientation = cam->orientation();


                auto cam_right = cam->right_vector();
                thecam::math::set_from_angle_axis(rot_lat, dy * (3.14159265f / 180.0f), -cam_right);
                auto rot_lat_conj = thecam::math::conjugate(rot_lat);

                cam->orientation(rot_lat * initial_orientation);


                pos_rot = rot_lon * pos_rot * rot_lon_conj;
                pos_rot = rot_lat * pos_rot * rot_lat_conj;


                cam->position(point_type(
                    pos_rot.x() + m_rot_cntr.x(), pos_rot.y() + m_rot_cntr.y(), pos_rot.z() + m_rot_cntr.z(), 1.0f));

                this->m_last_sx = x;
                this->m_last_sy = y;
            }
        }
    }

    void on_drag_change_orbit(const screen_type x, const screen_type y)
    {
        if (this->manipulating() && this->enabled()) {
            auto cam = this->camera();
            THE_ASSERT(cam != nullptr);

            if (this->m_last_sy != y) {

                screen_type dy = y - m_last_sy;

                auto cam_pos = cam->eye_position();

                auto v = thecam::math::normalise(m_rot_cntr - cam_pos);

                 cam->position(cam_pos - (v * dy * (this->m_orbit/500.0f)));

                this->m_orbit = std::abs(thecam::math::length(m_rot_cntr - cam->eye_position()));
            }

            this->m_last_sx = x;
            this->m_last_sy = y;
        }
    }

    /**
     * Report that dragging begun (mouse for dragging button is down)
     * at the specified screen coordinates.
     *
     * @param x
     * @param y
     */
    void on_drag_start(const screen_type x, const screen_type y) {
        if (!this->manipulating() && this->enabled()) {
            this->begin_manipulation();
            this->m_last_sx = x;
            this->m_last_sy = y;
        }
    }

    /**
     * Report that dragging ended (mouse button was released).
     */
    inline void on_drag_stop(void) { this->end_manipulation(); }

    /**
     * Gets the centre of rotation (usually the centre of the object to
     * rotate the camera around).
     *
     * @returns The centre of rotation.
     */
    inline const point_type& rotation_centre(void) const { return this->rotCentre; }

    /**
     * Changes the centre of rotation.
     *
     * This method must not be called while the cards are turned is begin dragged.
     *
     * @param rotCentre The new centre of rotation.
     */
    inline void set_rotation_centre(const point_type& rotCentre) {
        THE_ASSERT(!this->manipulating());
        this->m_rot_cntr = rotCentre;
    }

    inline void set_orbit(world_type orbit) { m_orbit = orbit; }

    inline world_type get_orbit() { return m_orbit; }

private:
    /** The centre of rotation (target point of the camera). */
    point_type m_rot_cntr;

    /** The distance between rotation center and camera */
    world_type m_orbit;

    /** The x-coordinate of the last clicked screen position */
    screen_type m_last_sx;

    /** The y-coordinate of the last clicked screen position */
    screen_type m_last_sy;
};

} /* end namespace thecam */
} /* end namespace core */
} /* end namespace megamol */


#endif // !TURNTABLE_MANIPULATOR_H_INCLUDED
