#pragma once
#include "main.h"
#include "Ez-Template/api.hpp"
#include "api.h"
#include "pros/misc.h"
#include "pros/rtos.hpp"
#include <atomic>
#include <cmath>


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
inline pros::Motor lift1(-9);
// lift2 must spin the SAME way as lift1 on the lift. If the lift won't move
// (motors fighting each other), flip this between 11 and -11.
inline pros::Motor lift2(11);
inline pros::Motor score(18);
inline pros::Motor claw(8); 






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
  constexpr double PRELOAD_DRIVE = -18;   // loader -> alliance goal
  constexpr double PRELOAD_TURN  = -45;
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
void calw_stop()  { claw.move(0);}

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


// =====================================================================
//  CLAW + LIFT AUTONS  (rules: RECF Achieve Pinnacle manual v3.0)
//
//  HOW A RUN STARTS / STOPS
//   From Nov 17 2026 there is NO field control. Start auton by holding
//   B + DOWN, then put the controller down. The code stops the robot by
//   itself before the 15 s alliance auton / 60 s solo coding match ends
//   (moving after the period = foul + no auton ranking point).
//
//  SETUP BEFORE EVERY RUN
//   - Lift all the way DOWN (the lift encoders are zeroed at the start).
//   - Robot touching our loader, claw pointed at the nearest ALLIANCE goal
//     (or set GOAL_HEADING if it has to turn).
//   - Preload pin in the open claw, ALLIANCE COLOR ON TOP.
//
//  ALL DISTANCES ARE GUESSES - measure on a real field and tune.
// =====================================================================
namespace CL {
  // ---- route (inches / degrees) ----
  constexpr double GOAL_HEADING  = 0;    // heading to the goal (0 = robot aimed at it)
  constexpr double GOAL_APPROACH = 14;   // stop SHORT of the goal while the lift rises
  constexpr double GOAL_CREEP    = 4;    // then creep so the pin is over the goal
  constexpr double BACK_OFF      = 6;    // back away after letting go
  constexpr double LOADER_DRIVE  = 10;   // (solo) after turning around, drive to the loader
  constexpr double LOADER_CREEP  = 3;    // (solo) slow push into the loader to grab the pin

  // ---- speeds (out of 127) ----
  constexpr int DRIVE_SPEED = 110;
  constexpr int SLOW_SPEED  = 50;
  constexpr int TURN_SPEED  = 90;

  // ---- direction: -1 flips every auton forward/back move ----
  // (the claw is on the back of the robot). Set to 1 to undo.
  constexpr int DRIVE_DIR = -1;

  // ---- lift (lift1 + lift2 together) ----
  // How high auton raises the lift, in motor degrees (360 = 1 turn).
  // To find it: in driver, hold UP until the claw is at goal height and read
  // "Lift:" on the controller screen (in turns), then put turns * 360 here.
  constexpr double LIFT_UP_DEG   = 5.0 * 360.0;   // was 10 turns (too high)
  // Lift direction in auton, per motor. -1 = flipped, 1 = normal.
  constexpr int LIFT1_AUTON_DIR = -1;  // port 9
  constexpr int LIFT2_AUTON_DIR = -1;  // port 11
  constexpr double LIFT_DOWN_DEG = 0;
  constexpr int    LIFT_RPM      = 200;  // driver uses 100; lower if the lift skips/stalls
  constexpr double LIFT_TOL_DEG  = 60;
  // Max time to wait for the lift. If it isn't there by then, auton warns
  // and KEEPS GOING (lift keeps trying in the background).
  constexpr uint32_t LIFT_TIMEOUT_MS = 4000;

  // ---- claw (driver: R2 = -200, L2 = +200) ----
  // Flipped: auton was opening when it should grab and not dropping the pin.
  // If it's wrong again, swap all three signs back.
  constexpr int CLAW_CLOSE = 127;
  constexpr int CLAW_HOLD  = 40;    // gentle squeeze while carrying
  constexpr int CLAW_OPEN  = -127;
  constexpr uint32_t CLAW_MS = 500; // longer so the pin fully drops

  // ---- time ----
  constexpr uint32_t SAFETY_MS = 300;    // stop this early so the robot is still at the buzzer
  constexpr uint32_t CYCLE_MS  = 16000;  // (solo) time one loader -> goal trip takes. Measure it!
}

// ---------- period clock + hard stop ----------
static uint32_t period_start_ms = 0;
static std::atomic<uint32_t> period_limit_ms{AU::AUTON_MS};
static std::atomic<bool> period_active{false};
static std::atomic<bool> end_requested{false};  // set when Y is pressed during auton

static uint32_t period_elapsed() { return pros::millis() - period_start_ms; }
static bool period_over() {
  return end_requested || period_elapsed() + CL::SAFETY_MS >= period_limit_ms;
}

// Show which auton step is running on the controller screen (and terminal).
static void step(const char* msg) {
  printf("AUTON %5.1fs: %s\n", period_elapsed() / 1000.0, msg);
  master.print(0, 0, "%-18s", msg);
}
static bool period_has(uint32_t need_ms) { return period_elapsed() + need_ms + CL::SAFETY_MS < period_limit_ms; }

static void stop_everything() {
  chassis.drive_mode_set(ez::DISABLE, true);
  chassis.drive_set(0, 0);
  bottom_intake.brake();
  top_intake.brake();
  lift1.brake();
  lift2.brake();
  score.brake();
  claw.brake();
}

// ---------- helpers (do nothing once the period is over) ----------
static void drive(double inches, int speed) {
  if (period_over()) return;
  chassis.pid_drive_set(CL::DRIVE_DIR * inches * 1_in, speed, std::fabs(inches) > 8);
  chassis.pid_wait();
}

static void turn_to(double heading_deg) {
  if (period_over()) return;
  chassis.pid_turn_set(heading_deg * 1_deg, CL::TURN_SPEED);
  chassis.pid_wait();
}

static void lift_to(double deg) {  // starts the lift, doesn't wait
  if (period_over()) return;
  lift1.move_absolute(CL::LIFT1_AUTON_DIR * deg, CL::LIFT_RPM);  // port 9
  lift2.move_absolute(CL::LIFT2_AUTON_DIR * deg, CL::LIFT_RPM);  // port 11
}

static bool lift_wait() {
  const uint32_t t0 = pros::millis();
  while (!period_over() && pros::millis() - t0 < CL::LIFT_TIMEOUT_MS) {
    const bool lift1_at_target =
        std::fabs(lift1.get_target_position() - lift1.get_position()) <= CL::LIFT_TOL_DEG;
    const bool lift2_at_target =
        std::fabs(lift2.get_target_position() - lift2.get_position()) <= CL::LIFT_TOL_DEG;
    if (lift1_at_target && lift2_at_target) return true;
    pros::delay(10);
  }
  // Lift didn't make it: don't stop. Leave it trying and carry on.
  if (!period_over()) {
    step("LIFT SLOW-going on");
    master.rumble("-");
  }
  return false;
}

static void claw_grab() {
  if (period_over()) return;
  claw.move(CL::CLAW_CLOSE);
  pros::delay(CL::CLAW_MS);
  claw.move(CL::CLAW_HOLD);
}

static void claw_release() {
  if (period_over()) return;
  claw.move(CL::CLAW_OPEN);
  pros::delay(CL::CLAW_MS);
  claw.move(0);
}

// Carry the pin in the claw to the goal in front and drop it on.
static bool claw_score_pin(int s) {
  if (period_over()) return false;
  step("2 lift up+turn");
  lift_to(CL::LIFT_UP_DEG);                  // raise while driving
  turn_to(s * CL::GOAL_HEADING);
  step("3 drive to goal");
  drive(CL::GOAL_APPROACH, CL::DRIVE_SPEED);  // stop short so the pin can't hit the goal
  step("4 wait for lift");
  lift_wait();                                // keeps going even if the lift is stuck
  step("5 creep + drop");
  drive(CL::GOAL_CREEP, CL::SLOW_SPEED);
  claw_release();                             // pin drops onto the goal
  step("6 back off");
  drive(-CL::BACK_OFF, CL::SLOW_SPEED);
  lift_to(CL::LIFT_DOWN_DEG);                 // lower on the way out
  return !period_over();
}

// ALLIANCE (15 s): preload -> nearest alliance goal. Stays in our zone.
static void claw_alliance(bool red) {
  period_limit_ms = AU::AUTON_MS;
  step("1 claw grab");
  claw_grab();
  claw_score_pin(red ? 1 : -1);
}

// SOLO CODING (60 s): preload, then loader -> goal cycles.
// Teammate loads a pin into the loader, ALLIANCE COLOR ON TOP.
static void claw_skills() {
  period_limit_ms = AU::SKILLS_MS;
  step("1 claw grab");
  claw_grab();
  if (!claw_score_pin(1)) return;

  while (period_has(CL::CYCLE_MS)) {
    step("7 go to loader");
    turn_to(CL::GOAL_HEADING + 180);        // face the loader (lift lowers meanwhile)
    drive(CL::LOADER_DRIVE, CL::DRIVE_SPEED);
    lift_wait();                            // claw should be down to grab; keeps going if stuck
    drive(CL::LOADER_CREEP, CL::SLOW_SPEED);
    claw_grab();
    drive(-CL::LOADER_CREEP, CL::SLOW_SPEED);
    if (!claw_score_pin(1)) return;
  }
}

// 1. RED ALLIANCE (claw + lift)
void drive_left()   { claw_alliance(true); }
// 2. BLUE ALLIANCE (claw + lift, mirrored)
void drive_right()  { claw_alliance(false); }
// 3. SOLO CODING / SKILLS (claw + lift)
void drive_skills() { claw_skills(); }
// Your original routines, still selectable
void old_red()    { pinnacle_alliance(true); }
void old_blue()   { pinnacle_alliance(false); }
void old_skills() { period_limit_ms = AU::SKILLS_MS; pinnacle_skills(); }
// TEST: only drives and turns (no claw, no lift). Use this to check
// that auton driving works at all.
void drive_test() {
  step("TEST fwd 12in");
  drive(12, 80);
  step("TEST turn 90");
  turn_to(90);
  step("TEST turn 0");
  turn_to(0);
  step("TEST back 12in");
  drive(-12, 80);
  step("TEST done");
}
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

  lift1.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  lift2.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  claw.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);


  ez::as::auton_selector.autons_add({
      ez::Auton("Drive Test (no claw/lift)", drive_test),
      ez::Auton("Red Alliance Auton", drive_left),
      ez::Auton("Blue Alliance Auton", drive_right),
      ez::Auton("Solo Coding / Skills", drive_skills),
      ez::Auton("Do Nothing", drive_nothing),
      ez::Auton("Old Red Auton", old_red),
      ez::Auton("Old Blue Auton", old_blue),
      ez::Auton("Old Skills", old_skills),
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
  lift1.tare_position();  // lift must start all the way down
  lift2.tare_position();

  period_start_ms = pros::millis();
  period_limit_ms = AU::AUTON_MS;  // skills routines raise this to 60 s
  end_requested = false;
  period_active = true;

  if (!chassis.drive_imu_calibrated()) {
    step("IMU FAIL-restart!");
    pros::delay(60);
  } else {
    step("AUTON START");
    pros::delay(60);
  }

  // Watchdog: freezes the robot at the deadline, or right away if Y is
  // pressed (rule 4.1.1 lets the team press a button to END the routine).
  pros::Task watchdog([] {
    while (period_active && !period_over()) {
      if (master.get_digital(DIGITAL_Y)) end_requested = true;
      pros::delay(5);
    }
    while (period_active) {
      stop_everything();
      pros::delay(10);
    }
  });

  ez::as::auton_selector.selected_auton_call();

  // Finished early: let the lift finish lowering, stop at the deadline.
  // (Press Y to skip the wait and go straight to driving.)
  if (!period_over()) step("done - Y to drive");
  while (!period_over()) pros::delay(10);
  period_active = false;
  stop_everything();
  step(end_requested ? "ENDED (Y)" : "AUTON OVER");
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
  // Claw: 0 = stopped, 1 = R2 direction, -1 = L2 direction
  // (a bool can't be "nullptr", so this uses a number instead)
  int claw_running = 0;
  bool intake_running = false;
  bool top_intake_running = false;
  int screen_count = 0;

  while (true) {
    ez_template_extras();
    chassis.opcontrol_arcade_standard(ez::SINGLE);

    // Lift: moves ONLY while you hold the button, stops (and holds) on release.
    if (master.get_digital(DIGITAL_UP)) {
      lift1.move_velocity(100);
      lift2.move_velocity(100);
    } else if (master.get_digital(DIGITAL_LEFT)) {
      lift1.move_velocity(-100);
      lift2.move_velocity(-100);
    } else {
      lift1.brake();
      lift2.brake();
    }

    // Show lift height (turns) on the controller, to tune LIFT_UP_DEG
    if (++screen_count >= 10) {
      screen_count = 0;
      master.print(1, 0, "Lift: %.1f   ", lift1.get_position() / 360.0);
    }


    if (master.get_digital_new_press(DIGITAL_R2)) {
      if (claw_running == 1) claw_running = 0;
      else claw_running = 1;
    }


    if (master.get_digital_new_press(DIGITAL_L2)) {
      if (claw_running == -1) claw_running = 0;
      else claw_running = -1;
    }


    if (master.get_digital(DIGITAL_X)) {
      bottom_intake.move(100);
      top_intake.move(100);
      pros::delay(20);
      continue;
    }


    if (claw_running == 0) {
      claw.move(0);
    } else if (claw_running == 1) {
      claw.move(-200);
    } else {
      claw.move(200);
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
