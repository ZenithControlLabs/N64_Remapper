#ifndef _STICK_H
#define _STICK_H

#include "Phobri64.h"

#define MAX_STICK_ANGLE 0.4886921906f
#define NUM_NOTCHES 8
#define FIT_ORDER 3

typedef struct {
    float fit_coeffs_x[FIT_ORDER+1];
    float fit_coeffs_y[FIT_ORDER+1];

    float affine_coeffs[NUM_NOTCHES][4];
    float boundary_angles[NUM_NOTCHES];
} stick_params_t;

// FIXME better name
typedef struct {
    int8_t x;
    int8_t y;
} processed_stick_t; 

// This array represents the values you would sweep
// in the notches on a perfect Hori.
// Since a perfect hori would be symmetrical, this applies for X and Y.
static const float perfect_angles[] = {-100, -75, 0, 75, 100};

float linearize(const float point, const float coefficients[]);

void calc_stick_values(float angle, float* x, float* y);

float angle_on_sphere(const float x, const float y);

void clean_cal_points(const float raw_cal_points_x[], const float raw_cal_points_y[], float cleaned_points_x[], float cleaned_points_y[]);

void linearize_cal(const float cleaned_points_x[], const float cleaned_points_y[], float linearized_points_x[], float linearized_points_y[], stick_params_t *stick_params);

void process_stick(const raw_report_t* raw_report, stick_params_t *stick_params, processed_stick_t* stick_out);

#endif /* _STICK_H */