// Mars lander simulator
// Version 1.11
// Mechanical simulation functions
// Gabor Csanyi and Andrew Gee, August 2019

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation, to make use of it
// for non-commercial purposes, provided that (a) its original authorship
// is acknowledged and (b) no modified versions of the source code are
// published. Restriction (b) is designed to protect the integrity of the
// exercise for future generations of students. The authors would be happy
// to receive any suggested modifications by private correspondence to
// ahg@eng.cam.ac.uk and gc121@eng.cam.ac.uk.

#include "lander.h"

void autopilot (void)
  // Autopilot to adjust the engine throttle, parachute and attitude control
{
  // INSERT YOUR CODE HERE
    double K_h, K_p, delta, h, P_out, target_altitude = 500;
    throttle = 747.1 / MAX_THRUST;
    bool safe = safe_to_deploy_parachute();
    K_h = 0.017;
    K_p = 10;
    delta = 0.001;
    h = position.abs() - MARS_RADIUS;
    P_out = -K_p * (0.5 + K_h * h + velocity * position.norm());

    //if (parachute_status == NOT_DEPLOYED && safe == TRUE && position.abs() < MARS_RADIUS + 0.5 * EXOSPHERE) parachute_status = DEPLOYED;

    if (P_out <= -delta) throttle = 0;

    else if (P_out > -delta && P_out < 1 - delta) throttle = delta + P_out; 

    else if (P_out >= 1 - delta) throttle = 1; 
    
    //// Write h and descent rate to file
    //ofstream fout;
    //fout.open("autopilot.txt", ios_base::app);
    //if (fout) { // file opened successfully
    //    fout << h << ' ' << velocity * position.norm() << endl;
    //}

    //else { // file did not open successfully
    //    cout << "Could not open trajectory file for writing" << endl;
    //}

}

void numerical_dynamics (void)
  // This is the function that performs the numerical integration to update the
  // lander's pose. The time step is delta_t (global variable).
{
  // INSERT YOUR CODE HERE
    // Euler integrator
    /*double lander_mass = UNLOADED_LANDER_MASS + fuel * FUEL_CAPACITY * FUEL_DENSITY, d = atmospheric_density(position);
    vector3d gravity, drag_lander, drag_parachute, thrust, acceleration;
    gravity = -(GRAVITY * MARS_MASS * lander_mass / position.abs2()) * position.norm();
    drag_lander = -0.5 * d * DRAG_COEF_LANDER * M_PI * LANDER_SIZE * LANDER_SIZE * velocity.abs2() * velocity.norm();
    drag_parachute = -0.5 * d * DRAG_COEF_CHUTE * 5.0 * 4.0 * LANDER_SIZE * LANDER_SIZE * velocity.abs2() * velocity.norm();
    thrust = thrust_wrt_world();

    if (parachute_status == DEPLOYED) {
        acceleration = (gravity + drag_lander + drag_parachute + thrust) / lander_mass;
    }

    else if (parachute_status == NOT_DEPLOYED || LOST) {
        acceleration = (gravity + drag_lander + thrust) / lander_mass;
    }

    position = position + delta_t * velocity;
    velocity = velocity + delta_t * acceleration;*/

    // Verlet integrator
    double lander_mass = UNLOADED_LANDER_MASS + fuel * FUEL_CAPACITY * FUEL_DENSITY, d = atmospheric_density(position);
    vector3d gravity, drag_lander, drag_parachute, thrust, acceleration, new_position;
    static vector3d previous_position;
    gravity = -(GRAVITY * MARS_MASS * lander_mass / position.abs2()) * position.norm();
    drag_lander = -0.5 * d * DRAG_COEF_LANDER * M_PI * LANDER_SIZE * LANDER_SIZE * velocity.abs2() * velocity.norm();
    drag_parachute = -0.5 * d * DRAG_COEF_CHUTE * 5.0 * 4.0 * LANDER_SIZE * LANDER_SIZE * velocity.abs2() * velocity.norm();
    thrust = thrust_wrt_world();

        if (simulation_time == 0.0) {
            if (parachute_status == DEPLOYED) {
                acceleration = (gravity + drag_lander + drag_parachute + thrust) / lander_mass;
            }

            else if (parachute_status == NOT_DEPLOYED || LOST) {
                acceleration = (gravity + drag_lander + thrust) / lander_mass;
            }
            
            new_position = position + delta_t * velocity;
            velocity = velocity + delta_t * acceleration;
        }

        else {
            if (parachute_status == DEPLOYED) {
                acceleration = (gravity + drag_lander + drag_parachute + thrust) / lander_mass;
            }

            else if (parachute_status == NOT_DEPLOYED || LOST) {
                acceleration = (gravity + drag_lander + thrust) / lander_mass;
            }
            new_position = 2 * position - previous_position + delta_t * delta_t * acceleration;
            velocity = (new_position - position) / delta_t;
        }

        previous_position = position;
        position = new_position;

  // Here we can apply an autopilot to adjust the thrust, parachute and attitude
  if (autopilot_enabled) autopilot();

  // Here we can apply 3-axis stabilization to ensure the base is always pointing downwards
  if (stabilized_attitude) attitude_stabilization();
}

void initialize_simulation (void)
  // Lander pose initialization - selects one of 10 possible scenarios
{
  // The parameters to set are:
  // position - in Cartesian planetary coordinate system (m)
  // velocity - in Cartesian planetary coordinate system (m/s)
  // orientation - in lander coordinate system (xyz Euler angles, degrees)
  // delta_t - the simulation time step
  // boolean state variables - parachute_status, stabilized_attitude, autopilot_enabled
  // scenario_description - a descriptive string for the help screen

  scenario_description[0] = "circular orbit";
  scenario_description[1] = "descent from 10km";
  scenario_description[2] = "elliptical orbit, thrust changes orbital plane";
  scenario_description[3] = "polar launch at escape velocity (but drag prevents escape)";
  scenario_description[4] = "elliptical orbit that clips the atmosphere and decays";
  scenario_description[5] = "descent from 200km";
  scenario_description[6] = "areostationary orbit";
  scenario_description[7] = "";
  scenario_description[8] = "";
  scenario_description[9] = "";

  switch (scenario) {

  case 0:
    // a circular equatorial orbit
    position = vector3d(1.2*MARS_RADIUS, 0.0, 0.0);
    velocity = vector3d(0.0, -3247.087385863725, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 1:
    // a descent from rest at 10km altitude
    position = vector3d(0.0, -(MARS_RADIUS + 10000.0), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = false;
    break;

  case 2:
    // an elliptical polar orbit
    position = vector3d(0.0, 0.0, 1.2*MARS_RADIUS);
    velocity = vector3d(3500.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 3:
    // polar surface launch at escape velocity (but drag prevents escape)
    position = vector3d(0.0, 0.0, MARS_RADIUS + LANDER_SIZE/2.0);
    velocity = vector3d(0.0, 0.0, 5027.0);
    orientation = vector3d(0.0, 0.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 4:
    // an elliptical orbit that clips the atmosphere each time round, losing energy
    position = vector3d(0.0, 0.0, MARS_RADIUS + 100000.0);
    velocity = vector3d(4000.0, 0.0, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 5:
    // a descent from rest at the edge of the exosphere
    position = vector3d(0.0, -(MARS_RADIUS + EXOSPHERE), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = false;
    break;

  case 6:
    // an areostationary orbit with a ground speed of 0 metres per second
    position = vector3d(cbrt(GRAVITY * MARS_MASS * MARS_DAY * MARS_DAY / (4 * M_PI * M_PI)), 0.0, 0.0);
    velocity = vector3d(0.0, cbrt(GRAVITY * MARS_MASS * MARS_DAY * MARS_DAY / (4 * M_PI * M_PI)) * 2 * M_PI / MARS_DAY, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 7:
    position = vector3d(0.0, MARS_RADIUS + 700, 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.01;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = true;
    break;

  case 8:
    break;

  case 9:
    break;

  }
}
