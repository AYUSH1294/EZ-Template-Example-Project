#pragma once
#include "main.h"
#include "Ez-Template/api.hpp"
#include "api.h"
#include "pros/misc.h"
#include "pros/rtos.hpp"

// Chassis constructor
ez::Drive chassis(
    // Left Chassis Ports (negative port will reverse it!)
    {-1, 2, -3},
    // Right Chassis Ports (negative port will reverse it!)
    {4, -5, 6},
    10,      // IMU Port
    3.25,    // Wheel Diameter
    600);    // Wheel RPM

// Define motors
inline pros::Motor bottom_intake(-20);
inline pros::Motor top_intake(19);
inline pros::Motor lift(9);
inline pros::Motor score(18);



// Define pistons
inline ez::Piston match_load('A', false);
inline ez::Piston wing('B', false);

// Forward declaration
void default_constants();

// =====================================================================
//  PINNACLE (RECF Achieve 2026-27) AUTONS
//
//  ALL DISTANCES / ANGLES ARE STARTING GUESSES - tune on the real field.
//  Positive turn = right (clockwise). Red = +1, Blue = -1 mirrors turns.
//  Frame assumed: robot starts touching its alliance loader, facing into
//  the field, holding the yellow/red preload pin.
// =====================================================================

// ---------- tuning constants (inches / degrees / ms) ----------
namespace AU {
  // preload
  constexpr double PRELOAD_DRIVE = 18;   // loader -> alliance goal
  constexpr double PRELOAD_TURN  = 45;
  // roller in our zone
  constexpr double ROLLER_TURN   = 90;
  constexpr double ROLLER_DRIVE  = 20;
  // neutral goal run (stay on OUR side of neutral zone in auton!)
  constexpr double NEUTRAL_TURN  = 0;
  constexpr double NEUTRAL_DRIVE = 36;
  // timing
  constexpr uint32_t AUTON_MS  = 15000;  // alliance match auton
  constexpr uint32_t SKILLS_MS = 60000;  // solo coding match
}

// ---------- time guard so we never run past the period ----------
static uint32_t auton_start = 0;
static bool time_ok(uint32_t need_ms, uint32_t limit_ms) {
  return (pros::millis() - auton_start) + need_ms < limit_ms;
}

// ---------- subsystem helpers: FILL IN FOR THE NEW ROBOT ----------
void intake_in()   { bottom_intake.move(-127); top_intake.move(-127); }
void intake_out()  { bottom_intake.move(127);  top_intake.move(127);  }
void intake_stop() { bottom_intake.move(0);    top_intake.move(0);    }

// Place held pin on goal in front of us, ALLIANCE COLOR UP (visible).
void place_pin() { intake_out(); pros::delay(400); intake_stop(); }

// Nest a cup onto a scored pin (1 pt each).
void place_cup() { intake_out(); pros::delay(300); intake_stop(); }

// Rotate the 3-sided roller so OUR color faces the field (0-2 flips).
void roller_to_alliance(int flips) {
  for (int i = 0; i < flips; i++) {
    score.move(127);   // TODO: your roller-turning mechanism
    pros::delay(350);
    score.move(0);
    pros::delay(100);
  }
}

static void auton_reset() {
  chassis.pid_targets_reset();
  chassis.drive_imu_reset();
  chassis.drive_sensor_reset();
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);
  auton_start = pros::millis();
}

// -----------------------------------------
//  ALLIANCE AUTON (15 s)
//  Goal: Auton Ranking Point = >=3 cups AND >=4 halfpins across >=2 goals
//  AND our zone's roller showing our color. Never touch the opponent's side
//  of the neutral zone or their alliance goals (any foul forfeits the RP).
// -----------------------------------------
static void pinnacle_alliance(bool red) {
  const int s = red ? 1 : -1;
  auton_reset();

  // 1) Score preload (yellow/red pin) on our alliance goal
  chassis.pid_drive_set(AU::PRELOAD_DRIVE * 1_in, 110);
  chassis.pid_wait();
  chassis.pid_turn_set(s * AU::PRELOAD_TURN * 1_deg, 90);
  chassis.pid_wait();
  place_pin();

  // 2) Turn our zone's roller to our color
  chassis.pid_turn_set(s * AU::ROLLER_TURN * 1_deg, 90);
  chassis.pid_wait();
  chassis.pid_drive_set(AU::ROLLER_DRIVE * 1_in, 90);
  chassis.pid_wait();
  roller_to_alliance(1);
  chassis.pid_drive_set(-6_in, 90);
  chassis.pid_wait();

  // 3) Collect pins/cups (drive team may also feed the loader during auton)
  if (time_ok(5000, AU::AUTON_MS)) {
    intake_in();
    chassis.pid_turn_set(s * AU::NEUTRAL_TURN * 1_deg, 90);
    chassis.pid_wait();
    chassis.pid_drive_set(AU::NEUTRAL_DRIVE * 1_in, 70);
    chassis.pid_wait();
    pros::delay(300);
    intake_stop();
  }

  // 4) Score on a SECOND goal (neutral goal on our side)
  if (time_ok(3000, AU::AUTON_MS)) {
    place_pin();
    place_cup();
  }

  // 5) Return toward our loader
  if (time_ok(2000, AU::AUTON_MS)) {
    chassis.pid_drive_set(-AU::NEUTRAL_DRIVE * 1_in, 110);
    chassis.pid_wait();
  }
}

// -----------------------------------------
//  SOLO CODING MATCH (60 s, no driver input)
// -----------------------------------------
static void pinnacle_skills() {
  pinnacle_alliance(true);
  auton_start = pros::millis() - 15000;  // opener already used ~15 s

  for (int cycle = 0; cycle < 3; cycle++) {
    if (!time_ok(12000, AU::SKILLS_MS)) break;
    intake_in();
    chassis.pid_drive_set(-24_in, 110);
    chassis.pid_wait();
    pros::delay(600);   // time for drive team to feed the loader
    chassis.pid_drive_set(24_in, 110);
    chassis.pid_wait();
    place_pin();
  }

  // Finish touching our loader
  chassis.pid_drive_set(-18_in, 110);
  chassis.pid_wait();
}

// 1. RED ALLIANCE
void drive_left()   { pinnacle_alliance(true); }
// 2. BLUE ALLIANCE (mirrored)
void drive_right()  { pinnacle_alliance(false); }
// 3. SOLO CODING / SKILLS
void drive_skills() { pinnacle_skills(); }
// 4. DO NOTHING
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

  ez::as::auton_selector.autons_add({
      ez::Auton("Red Alliance Auton", drive_left),
      ez::Auton("Blue Alliance Auton", drive_right),
      ez::Auton("Solo Coding / Skills", drive_skills),
      ez::Auton("Do Nothing", drive_nothing),
  });

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
  chassis.pid_targets_reset();
  chassis.drive_imu_reset();
  chassis.drive_sensor_reset();
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);

  ez::as::auton_selector.selected_auton_call();
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
  bool intake_running = false;
  bool top_intake_running = false;

  while (true) {
    ez_template_extras();
    chassis.opcontrol_arcade_standard(ez::SINGLE);

    if (master.get_digital_new_press(DIGITAL_UP)) {
      lift.set_voltage_limit(12000);
      lift.move_relative(20.0 * 360.0, 100);
    }

    if (master.get_digital_new_press(DIGITAL_LEFT)) {
      lift.set_voltage_limit(12000);
      lift.move_relative(-20.0 * 360.0, 100);
    }

    if (master.get_digital_new_press(DIGITAL_R2)) {
      intake_running = !intake_running;
    }

    if (master.get_digital_new_press(DIGITAL_L1)) {
      top_intake_running = !top_intake_running;
    }

    if (master.get_digital(DIGITAL_X)) {
      bottom_intake.move(100);
      top_intake.move(100);
      pros::delay(20);
      continue;
    }

    if (intake_running || top_intake_running) {
      bottom_intake.move(-200);
    } else {
      bottom_intake.move(0);
    }

    if (intake_running) {
      top_intake.move(-200);
    } else {
      top_intake.move(0);
    }

    pros::delay(20);

    if (master.get_digital(DIGITAL_A)) {
      match_load.set(true);
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