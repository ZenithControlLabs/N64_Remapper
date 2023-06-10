#ifndef _STICK_H
#define _STICK_H

#include "Phobri64.h"

#define NUM_NOTCHES 8
#define FIT_ORDER 3

typedef struct {
    float fit_coeffs_x[FIT_ORDER + 1];
    float fit_coeffs_y[FIT_ORDER + 1];

    float affine_coeffs[NUM_NOTCHES][4];
    float boundary_angles[NUM_NOTCHES];
} calib_results_t;

typedef struct {
    int8_t notch_points_x[NUM_NOTCHES + 1];
    int8_t notch_points_y[NUM_NOTCHES + 1];
} stick_config_t;

// FIXME better name
typedef struct {
    int8_t x;
    int8_t y;
} processed_stick_t;

// This array represents the values you would sweep
// in the notches on a perfect Hori.
// Since a perfect hori would be symmetrical, this applies for X and Y.
static const double perfect_angles[] = {-100, -75, 0, 75, 100};

void fold_center_points(const float raw_cal_points_x[],
                      const float raw_cal_points_y[], float cleaned_points_x[],
                      float cleaned_points_y[]);

float linearize(const float point, const float coefficients[]);

void linearize_cal(const float cleaned_points_x[],
                   const float cleaned_points_y[], float linearized_points_x[],
                   float linearized_points_y[], calib_results_t *calib_results);

void notch_remap(const float x_in, const float y_in, float *x_out, float *y_out,
                 const calib_results_t *calib_results);

void notch_calibrate(const float in_points_x[], const float in_points_y[],
                     float notch_points_x[], float notch_points_y[],
                     calib_results_t *calib_results);

void process_stick(const raw_report_t *raw_report,
                   calib_results_t *calib_results,
                   processed_stick_t *stick_out);

#endif /* _STICK_H */