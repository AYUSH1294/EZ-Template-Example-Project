#pragma once
#include "main.h"
#include "Ez-Template/api.hpp"
#include "api.h"
#include "pros/misc.h"
#include "pros/rtos.hpp"

<<<<<<< HEAD
=======

/////
// For installation, upgrading, documentations, and tutorials, check out our website!
// https://ez-robotics.github.io/EZ-Template/
/////
// Toggle state variable
bool motorOn = false;

// For edge detection
bool lastButtonState = false;
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
// Chassis constructor
ez::Drive chassis(
    // Left Chassis Ports (negative port will reverse it!)
    {-1, -2, 3},
    // Right Chassis Ports (negative port will reverse it!)
    {4, -5, 6},
    10,      // IMU Port
    3.25,    // Wheel Diameter
    600);    // Wheel RPM

// Define motors
inline pros::Motor bottom_intake(20);
inline pros::Motor top_intake(19);
inline pros::Motor lift(8);
inline pros::Motor score(18);

// Define pistons
inline ez::Piston match_load('A', false);
inline ez::Piston wing('B', false);
<<<<<<< HEAD

// Forward declaration
void default_constants();

// -----------------------------------------
//        AUTONOMOUS FUNCTIONS
// -----------------------------------------

// 1. LEFT SIDE AUTON
void drive_left() {
  chassis.pid_targets_reset();
  chassis.drive_imu_reset();
  chassis.drive_sensor_reset();
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);

  // -- START OF ROUTINE --

  // Step 1: Drive forward to clear the starting zone
  chassis.pid_drive_set(12_in, 110);
  chassis.pid_wait();

  // Step 2: Turn towards the center/neutral game objects (Triballs/Discs)
  chassis.pid_turn_set(90_deg, 90);
  chassis.pid_wait();

  // Step 3: Start intake to pick up the game object
  bottom_intake.move(-127);

  // Step 4: Drive forward to intake the object
  chassis.pid_drive_set(33_in, 45);
  chassis.pid_wait();
  pros::delay(500);

  // Step 5: Back up to return to a safe position
  chassis.pid_drive_set(-30_in, 60);
  chassis.pid_wait();

  // Step 6: Re-orient towards the match load zone (reset heading)
  chassis.pid_turn_set(0_deg, 90);
  chassis.pid_wait();

  // Step 7: Drive towards the match load bar
  chassis.pid_drive_set(23.5_in, 60);
  chassis.pid_wait();

  // Step 8: Turn to align parallel with the match load bar for loading
  chassis.pid_turn_set(-90_deg, 90);
  chassis.pid_wait();

  // Step 9: Deploy match load mechanism (piston)
  match_load.set(true);
  pros::delay(400);

  // Step 10: Drive forward slightly to position for match loading
  chassis.pid_drive_set(28_in, 100);
  chassis.pid_wait();

  // Step 11: "Jiggle" movement to help shake objects loose or ensure intake
  chassis.pid_drive_set(-1.2_in, 60);
  chassis.pid_wait();
  chassis.pid_drive_set(1.2_in, 60);
  chassis.pid_wait();
  chassis.pid_drive_set(-0.6_in, 60);
  chassis.pid_wait();
  pros::delay(100);

  // Step 12: Reverse away from the match load bar towards the goal
  chassis.pid_drive_set(-37_in, 60);
  chassis.pid_wait();

  // Step 13: Eject/Score the game objects into the goal
  bottom_intake.move(100); // Briefly reverse to clear jams or prep
  pros::delay(10);
  bottom_intake.move(0);
  bottom_intake.move(-127); // Intake runs to push objects out
  top_intake.move(-127);
  pros::delay(7000); // Long delay to ensure all match loads are scored
  top_intake.move(0);
  bottom_intake.move(0);

  // Step 14: STOPS HERE
  chassis.pid_drive_set(10_in, 60);
  chassis.pid_wait();

  // Step 15: Retract match load mechanism and execute end-game maneuvering
  match_load.set(false);
=======
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
  
  // Turn around to face the opposite direction
  chassis.pid_turn_set(180_deg, 90);
  
  // Drive across the field (perhaps to touch the elevation bar or clear zone)
  chassis.pid_drive_set(20_in, 360);
  
  // Final turns and drive to park or final position
  chassis.pid_turn_set(-90_deg, 90);
  chassis.pid_drive_set(10_in, 60);
  chassis.pid_turn_set(180_deg, 90);
  chassis.pid_drive_set(50_in, 200);
}

// 2. RIGHT SIDE AUTON (Mirrored)
void drive_right() {
  chassis.pid_targets_reset();
  chassis.drive_imu_reset();
  chassis.drive_sensor_reset();
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);

  // -- START OF ROUTINE --

  // Step 1: Drive forward to clear the starting zone
  chassis.pid_drive_set(12_in, 110);
  chassis.pid_wait();

  // Step 2: Turn towards center/neutral game objects (Mirrored: Left turn instead of Right)
  chassis.pid_turn_set(-90_deg, 90); 
  chassis.pid_wait();

  // Step 3: Start intake to pick up the game object
  bottom_intake.move(-127);

  // Step 4: Drive forward to intake the object
  chassis.pid_drive_set(33_in, 45);
  chassis.pid_wait();
  pros::delay(500);

  // Step 5: Back up to return to a safe position
  chassis.pid_drive_set(-30_in, 60);
  chassis.pid_wait();

  // Step 6: Re-orient towards the match load zone (0 degrees is straight relative to start)
  chassis.pid_turn_set(0_deg, 90);
  chassis.pid_wait();

  // Step 7: Drive towards the match load bar
  chassis.pid_drive_set(23.5_in, 60);
  chassis.pid_wait();

  // Step 8: Turn to align parallel with match load bar (Mirrored: Right turn instead of Left)
  chassis.pid_turn_set(90_deg, 90); 
  chassis.pid_wait();

  // Step 9: Deploy match load mechanism (piston)
  match_load.set(true);
  pros::delay(400);

  // Step 10: Drive forward slightly to position for match loading
  chassis.pid_drive_set(28_in, 100);
  chassis.pid_wait();

  // Step 11: "Jiggle" movement to help shake objects loose or ensure intake
  chassis.pid_drive_set(-1.2_in, 60);
  chassis.pid_wait();
  chassis.pid_drive_set(1.2_in, 60);
  chassis.pid_wait();
  chassis.pid_drive_set(-0.6_in, 60);
  chassis.pid_wait();
  pros::delay(100);

  // Step 12: Reverse away from the match load bar towards the goal
  chassis.pid_drive_set(-37_in, 60);
  chassis.pid_wait();

  // Step 13: Eject/Score the game objects into the goal
  bottom_intake.move(100); // Briefly reverse
  pros::delay(10);
  bottom_intake.move(0);
  bottom_intake.move(-127); // Intake runs to push objects out
  top_intake.move(-127);
  pros::delay(7000); // Long delay to ensure all match loads are scored
  top_intake.move(0);
  bottom_intake.move(0);

  // Step 14: Drive forward slightly away from the goal
  chassis.pid_drive_set(10_in, 60); 
  chassis.pid_wait();

  // Step 15: Retract match load mechanism and execute end-game maneuvering
  match_load.set(false);
  
  // Turn around to face opposite direction (Mirrored: -180 instead of 180)
  chassis.pid_turn_set(-180_deg, 90); 
  
  // Drive across the field (perhaps to touch the elevation bar or clear zone)
  chassis.pid_drive_set(20_in, 360); 
  
  // Final turns and drive to park or final position (Mirrored turns)
  chassis.pid_turn_set(90_deg, 90);   
  chassis.pid_drive_set(10_in, 60);
  chassis.pid_turn_set(-180_deg, 90); 
  chassis.pid_drive_set(50_in, 200);
}

// 3. DO NOTHING
void drive_nothing() {
  // Do nothing
}

// -----------------------------------------
//           INITIALIZE
// -----------------------------------------

void initialize() {
  ez::ez_template_print();
  pros::delay(500);

  chassis.opcontrol_curve_buttons_toggle(true);
  chassis.opcontrol_drive_activebrake_set(0.0);
  chassis.opcontrol_curve_default_set(0.0, 0.0);

  default_constants();

<<<<<<< HEAD
  ez::as::auton_selector.autons_add({
      ez::Auton("Left Side Auton", drive_left),
      ez::Auton("Right Side Auton", drive_right),
      ez::Auton("Do Nothing", drive_nothing),
  });
=======
  // These are already defaulted to these buttons, but you can change the left/right curve buttons here!
  // chassis.opcontrol_curve_buttons_left_set(pros::E_CONTROLLER_DIGITAL_LEFT, pros::E_CONTROLLER_DIGITAL_RIGHT);  // If using tank, only the left side is used.
  // chassis.opcontrol_curve_buttons_right_set(pros::E_CONTROLLER_DIGITAL_Y, pros::E_CONTROLLER_DIGITAL_A);

  // Autonomous Selector using LLEMU
  ez::as::auton_selector.autons_add({});
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a

  chassis.initialize();
  ez::as::initialize();
  master.rumble(chassis.drive_imu_calibrated() ? "." : "---");
}

void disabled() {}
void competition_initialize() {}

// -----------------------------------------
//           AUTONOMOUS TASK
// -----------------------------------------

void autonomous() {
<<<<<<< HEAD
  chassis.pid_targets_reset();
  chassis.drive_imu_reset();
  chassis.drive_sensor_reset();
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);

  // CORRECTED LINE:
  ez::as::auton_selector.selected_auton_call();
=======
  chassis.pid_targets_reset();                // Resets PID targets to 0
  chassis.drive_imu_reset();                  // Reset gyro position to 0
  chassis.drive_sensor_reset();               // Reset drive sensors to 0
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);    // Set the current position, you can start at a specific position with this
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);  

    // If the LLEMU/auton selector is enabled, call the selected auton
  // This lets you pick autons from the LCD/selector instead of hardcoding here


  // Set motors to hold.  This helps autonomous consistency
  // Set motors to hold.  This helps autonomous consistency


  chassis.pid_drive_set(12_in,110);
  chassis.pid_wait();

  chassis.pid_turn_set(90_deg,90);
  chassis.pid_wait();

  bottom_intake.move(-127);

  chassis.pid_drive_set(33_in,45);
  chassis.pid_wait();
  pros::delay(500);

  chassis.pid_drive_set(-30_in,60);
  chassis.pid_wait();

  chassis.pid_turn_set(0_deg,90);
  chassis.pid_wait();

  chassis.pid_drive_set(23.5_in,60);
  chassis.pid_wait();

  chassis.pid_turn_set(-90_deg,90);
  chassis.pid_wait();

  match_load.set(true);
  pros::delay(500);

  chassis.pid_drive_set(29_in,45);
  chassis.pid_wait();

  chassis.pid_drive_set(-1.2_in,45);
  chassis.pid_wait();

  chassis.pid_drive_set(1.2_in,45);
  chassis.pid_wait();

  pros::delay(800);

  chassis.pid_drive_set(-39_in,60);
  chassis.pid_wait();

  top_intake.move(-127);
  pros::delay(5000);
  top_intake.move(0);
  bottom_intake.move(0);

  chassis.pid_drive_set(20_in,60);
  chassis.pid_wait();

  
  

  /*Odometry and Pure Pursuit are not magic

  It is possible to get perfectly consistent results without tracking wheels,
  but it is also possible to have extremely inconsistent results without tracking wheels.
  When you don't use tracking wheels, you need to:
   - avoid wheel slip
   - avoid wheelies
   - avoid throwing momentum around (super harsh turns, like in the example below)
  You can do cool curved motions, but you have to give your robot the best chance
  to be consistent
  */

   // Calls selected auton from autonomous selector
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
}

// -----------------------------------------
//           SCREEN TASKS
// -----------------------------------------

void screen_print_tracker(ez::tracking_wheel *tracker, std::string name, int line) {
  std::string tracker_value = "", tracker_width = "";
  if (tracker != nullptr) {
    tracker_value = name + " tracker: " + util::to_string_with_precision(tracker->get());
    tracker_width = "  width: " + util::to_string_with_precision(tracker->distance_to_center_get());
  }
  ez::screen_print(tracker_value + tracker_width, line);
}

void ez_screen_task() {
  while (true) {
    if (!pros::competition::is_connected()) {
      if (chassis.odom_enabled() && !chassis.pid_tuner_enabled()) {
        if (ez::as::page_blank_is_on(0)) {
          ez::screen_print("x: " + util::to_string_with_precision(chassis.odom_x_get()) +
                               "\ny: " + util::to_string_with_precision(chassis.odom_y_get()) +
                               "\na: " + util::to_string_with_precision(chassis.odom_theta_get()),
                           1);
          screen_print_tracker(chassis.odom_tracker_left, "l", 4);
          screen_print_tracker(chassis.odom_tracker_right, "r", 5);
          screen_print_tracker(chassis.odom_tracker_back, "b", 6);
          screen_print_tracker(chassis.odom_tracker_front, "f", 7);
        }
      }
    } else {
      if (ez::as::page_blank_amount() > 0)
        ez::as::page_blank_remove_all();
    }
    pros::delay(ez::util::DELAY_TIME);
  }
}
pros::Task ezScreenTask(ez_screen_task);

// -----------------------------------------
//           OPCONTROL
// -----------------------------------------

void ez_template_extras() {
  if (!pros::competition::is_connected()) {
    if (master.get_digital_new_press(DIGITAL_X))
      chassis.pid_tuner_toggle();
    if (master.get_digital(DIGITAL_B) && master.get_digital(DIGITAL_DOWN)) {
      pros::motor_brake_mode_e_t preference = chassis.drive_brake_get();
      autonomous();
      chassis.drive_brake_set(preference);
    }
    chassis.pid_tuner_iterate();
  } else {
    if (chassis.pid_tuner_enabled())
      chassis.pid_tuner_disable();
  }
}

void opcontrol() {
<<<<<<< HEAD
  bool intake_running = false;
  bool top_intake_running = false;
=======
  bool intake_running = false;        // R2 toggle
  bool top_intake_running = false;    // L1 toggle
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a

  while (true) {
    ez_template_extras();
    chassis.opcontrol_arcade_standard(ez::SINGLE);

<<<<<<< HEAD
    if (master.get_digital_new_press(DIGITAL_R2)) {
      intake_running = !intake_running;
    }
=======
    // R2 toggle
    if (master.get_digital_new_press(DIGITAL_R2)) {
      intake_running = !intake_running;
    }

    // L1 toggle
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
    if (master.get_digital_new_press(DIGITAL_L1)) {
      top_intake_running = !top_intake_running;
    }

<<<<<<< HEAD
    if (master.get_digital(DIGITAL_X)) {
      bottom_intake.move(100);
      top_intake.move(100);
      pros::delay(20);
      continue;
    }

=======
    // X button = FORCE REVERSE (momentary)
    if (master.get_digital(DIGITAL_X)) {
      bottom_intake.move(100);   // reverse
      top_intake.move(100);      // reverse
      pros::delay(20);
      continue;                  // skip the rest so nothing overrides it
    }

    // --- NORMAL LOGIC BELOW ---

    // Bottom intake runs if either toggle is active
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
    if (intake_running || top_intake_running) {
      bottom_intake.move(-200);
    } else {
      bottom_intake.move(0);
    }

<<<<<<< HEAD
=======
    // Top intake runs only when R2 toggle is active
>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
    if (intake_running) {
      top_intake.move(-200);
    } else {
      top_intake.move(0);
    }

    pros::delay(20);

    if (master.get_digital(DIGITAL_A)) {
      match_load.set(true);
<<<<<<< HEAD
    } else if (master.get_digital(DIGITAL_B)) {
      match_load.set(false);
    }

    if (master.get_digital(DIGITAL_R1)) {
      wing.set(true);
    } else if (master.get_digital(DIGITAL_L2)) {
      wing.set(false);
    }
    pros::delay(20);
  }
}
=======
    } 
    else if (master.get_digital(DIGITAL_B)) {
      match_load.set(false);
    } 

    if (master.get_digital(DIGITAL_R1)) {
      wing.set(true);
    } 
    else if (master.get_digital(DIGITAL_L2)) {
      wing.set(false);}
    pros::delay(20); 
  } 
  
}



>>>>>>> 542ad6c084b81670a3e628550a58fad30667713a
