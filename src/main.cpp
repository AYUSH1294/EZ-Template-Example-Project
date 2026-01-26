#pragma once

#include "main.h"
#include "Ez-Template/api.hpp"
#include "api.h"
#include "pros/misc.h"
#include "pros/rtos.hpp"


/////
// For installation, upgrading, documentations, and tutorials, check out our website!
// https://ez-robotics.github.io/EZ-Template/
/////
// Toggle state variable
bool motorOn = false;

// For edge detection
bool lastButtonState = false;
// Chassis constructor
ez::Drive chassis(
    // These are your drive motors, the first motor is used for sensing!
    {-1, -2, 3},     // Left Chassis Ports (negative port will reverse it!)
    {4, -5, 6},  // Right Chassis Ports (negative port will reverse it!)

    10,      // IMU Port
    3.25,  // Wheel Diameter (Remember, 4" wheels without screw holes are actually 4.125!)
    600);   // Wheel RPM = cartridge * (motor gear / wheel gear)

// Uncomment the trackers you're using here!
// 1. Tell the compiler the chassis exists elsewhere
extern Drive chassis;

// 2. Define your motors (using inline so they can be in the header)
inline pros::Motor bottom_intake(20);
inline pros::Motor top_intake(19);
inline pros::Motor lift(8);
inline pros::Motor score(18);

// 3. Define your piston
inline ez::Piston match_load('A', false);
inline ez::Piston wing('B', false);
  
// - `8` and `9` are smart ports (making these negative will reverse the sensor)
//  - you should get positive values on the encoders going FORWARD and RIGHT
// - `2.75` is the wheel diameter
// - `4.0` is the distance from the center of the wheel to the center of the robot
// ez::tracking_wheel horiz_tracker(8, 2.75, 4.0);  // This tracking wheel is perpendicular to the drive wheels
// ez::tracking_wheel vert_tracker(9, 2.75, 4.0);   // This tracking wheel is parallel to the drive wheels

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
  // Print our branding over your terminal :D
  ez::ez_template_print();
  default_constants();
  pros::delay(500);  // Stop the user from doing anything while legacy ports configure

  // Look at your horizontal tracking wheel and decide if it's in front of the midline of your robot or behind it
  //  - change `back` to `front` if the tracking wheel is in front of the midline
  //  - ignore this if you aren't using a horizontal tracker
  // chassis.odom_tracker_back_set(&horiz_tracker);
  // Look at your vertical tracking wheel and decide if it's to the left or right of the center of the robot
  //  - change `left` to `right` if the tracking wheel is to the right of the centerline
  //  - ignore this if you aren't using a vertical tracker
  // chassis.odom_tracker_left_set(&vert_tracker);

  // Configure your chassis controls
  chassis.opcontrol_curve_buttons_toggle(true);   // Enables modifying the controller curve with buttons on the joysticks
  chassis.opcontrol_drive_activebrake_set(0.0);   // Sets the active brake kP. We recommend ~2.  0 will disable.
  chassis.opcontrol_curve_default_set(0.0, 0.0);  // Defaults for curve. If using tank, only the first parameter is used. (Comment this line out if you have an SD card!)

  // Set the drive to your own constants from autons.cpp!
  default_constants();

  // These are already defaulted to these buttons, but you can change the left/right curve buttons here!
  // chassis.opcontrol_curve_buttons_left_set(pros::E_CONTROLLER_DIGITAL_LEFT, pros::E_CONTROLLER_DIGITAL_RIGHT);  // If using tank, only the left side is used.
  // chassis.opcontrol_curve_buttons_right_set(pros::E_CONTROLLER_DIGITAL_Y, pros::E_CONTROLLER_DIGITAL_A);

  // Autonomous Selector using LLEMU
  ez::as::auton_selector.autons_add({});

  // Initialize chassis and auton selector
  chassis.initialize();
  ez::as::initialize();
  master.rumble(chassis.drive_imu_calibrated() ? "." : "---");
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
  // . . .
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
  // . . .
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
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
}

/**
 * Simplifies printing tracker values to the brain screen
 */
void screen_print_tracker(ez::tracking_wheel *tracker, std::string name, int line) {
  std::string tracker_value = "", tracker_width = "";
  // Check if the tracker exists
  if (tracker != nullptr) {
    tracker_value = name + " tracker: " + util::to_string_with_precision(tracker->get());             // Make text for the tracker value
    tracker_width = "  width: " + util::to_string_with_precision(tracker->distance_to_center_get());  // Make text for the distance to center
  }
  ez::screen_print(tracker_value + tracker_width, line);  // Print final tracker text
}

/**
 * Ez screen task
 * Adding new pages here will let you view them during user control or autonomous
 * and will help you debug problems you're having
 */
void ez_screen_task() {
  while (true) {
    // Only run this when not connected to a competition switch
    if (!pros::competition::is_connected()) {
      // Blank page for odom debugging
      if (chassis.odom_enabled() && !chassis.pid_tuner_enabled()) {
        // If we're on the first blank page...
        if (ez::as::page_blank_is_on(0)) {
          // Display X, Y, and Theta
          ez::screen_print("x: " + util::to_string_with_precision(chassis.odom_x_get()) +
                               "\ny: " + util::to_string_with_precision(chassis.odom_y_get()) +
                               "\na: " + util::to_string_with_precision(chassis.odom_theta_get()),
                           1);  // Don't override the top Page line

          // Display all trackers that are being used
          screen_print_tracker(chassis.odom_tracker_left, "l", 4);
          screen_print_tracker(chassis.odom_tracker_right, "r", 5);
          screen_print_tracker(chassis.odom_tracker_back, "b", 6);
          screen_print_tracker(chassis.odom_tracker_front, "f", 7);
        }
      }
    }

    // Remove all blank pages when connected to a comp switch
    else {
      if (ez::as::page_blank_amount() > 0)
        ez::as::page_blank_remove_all();
    }

    pros::delay(ez::util::DELAY_TIME);
  }
}
pros::Task ezScreenTask(ez_screen_task);

/**
 * Gives you some extras to run in your opcontrol:
 * - run your autonomous routine in opcontrol by pressing DOWN and B
 *   - to prevent this from accidentally happening at a competition, this
 *     is only enabled when you're not connected to competition control.
 * - gives you a GUI to change your PID values live by pressing X
 */
void ez_template_extras() {
  // Only run this when not connected to a competition switch
  if (!pros::competition::is_connected()) {
    // PID Tuner
    // - after you find values that you're happy with, you'll have to set them in auton.cpp

    // Enable / Disable PID Tuner
    //  When enabled:
    //  * use A and Y to increment / decrement the constants
    //  * use the arrow keys to navigate the constants
    if (master.get_digital_new_press(DIGITAL_X))
      chassis.pid_tuner_toggle();

    // Trigger the selected autonomous routine
    if (master.get_digital(DIGITAL_B) && master.get_digital(DIGITAL_DOWN)) {
      pros::motor_brake_mode_e_t preference = chassis.drive_brake_get();
      autonomous();
      chassis.drive_brake_set(preference);
    }

    // Allow PID Tuner to iterate
    chassis.pid_tuner_iterate();
  }

  // Disable PID Tuner when connected to a comp switch
  else {
    if (chassis.pid_tuner_enabled())
      chassis.pid_tuner_disable();
  }
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
  bool intake_running = false;        // R2 toggle
  bool top_intake_running = false;    // L1 toggle

  while (true) {
    ez_template_extras();
    chassis.opcontrol_arcade_standard(ez::SINGLE);

    // R2 toggle
    if (master.get_digital_new_press(DIGITAL_R2)) {
      intake_running = !intake_running;
    }

    // L1 toggle
    if (master.get_digital_new_press(DIGITAL_L1)) {
      top_intake_running = !top_intake_running;
    }

    // X button = FORCE REVERSE (momentary)
    if (master.get_digital(DIGITAL_X)) {
      bottom_intake.move(100);   // reverse
      top_intake.move(100);      // reverse
      pros::delay(20);
      continue;                  // skip the rest so nothing overrides it
    }

    // --- NORMAL LOGIC BELOW ---

    // Bottom intake runs if either toggle is active
    if (intake_running || top_intake_running) {
      bottom_intake.move(-200);
    } else {
      bottom_intake.move(0);
    }

    // Top intake runs only when R2 toggle is active
    if (intake_running) {
      top_intake.move(-200);
    } else {
      top_intake.move(0);
    }

    pros::delay(20);

    if (master.get_digital(DIGITAL_A)) {
      match_load.set(true);
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



