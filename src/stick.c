#include "Phobri64.h"
#include "polyfit.h"

// FIXME - this whole file need to actually document what the units are in the
// signal chain It's impossible to read code like this without knowing what the
// units actually mean. For example, if you have a float, does it go from -1 to
// 1, 0 to 1, -127 to 127? Just the type is not enough. Ideally, the types would
// be separate and you have to manually cast them. A typedef should be
// sufficient

void clean_cal_points(const float raw_cal_points_x[],
                      const float raw_cal_points_y[], float cleaned_points_x[],
                      float cleaned_points_y[]) {
    cleaned_points_x[0] = 0;
    cleaned_points_y[0] = 0;

    for (int i = 0; i < NUM_NOTCHES; i++) {
        // each origin reading is summed together
        cleaned_points_x[0] += raw_cal_points_x[i * 2];
        cleaned_points_y[0] += raw_cal_points_y[i * 2];

        // for the notches, copy point into cleaned list
        cleaned_points_x[i + 1] = raw_cal_points_x[i * 2 + 1];
        cleaned_points_y[i + 1] = raw_cal_points_y[i * 2 + 1];
    }

    // remove the largest and smallest origin values to remove outliers
    // first, find their indices

    int smallestX = 0;
    int largestX = 0;
    int smallestY = 0;
    int largestY = 0;
    for (int i = 0; i < NUM_NOTCHES; i++) {
        if (raw_cal_points_x[i * 2] < raw_cal_points_x[smallestX]) {
            // record the new smallest index
            smallestX = i * 2;
        } else if (raw_cal_points_x[i * 2] > raw_cal_points_x[largestX]) {
            // record the new largest index
            largestX = i * 2;
        }

        if (raw_cal_points_y[i * 2] < raw_cal_points_y[smallestY]) {
            // record the new smallest index
            smallestY = i * 2;
        } else if (raw_cal_points_y[i * 2] > raw_cal_points_y[largestY]) {
            // record the new largest index
            largestY = i * 2;
        }
    }
    // subtract the smallest and largest values
    cleaned_points_x[0] -= raw_cal_points_x[smallestX];
    cleaned_points_x[0] -= raw_cal_points_x[largestX];
    cleaned_points_y[0] -= raw_cal_points_y[smallestY];
    cleaned_points_y[0] -= raw_cal_points_y[largestY];

    // divide by the total number of calibration steps/2 to get the average
    // origin value except it's minus 4 steps since we removed outliers
    cleaned_points_x[0] = cleaned_points_x[0] / ((float)NUM_NOTCHES - 2);
    cleaned_points_y[0] = cleaned_points_y[0] / ((float)NUM_NOTCHES - 2);
}

float linearize(const float point, const float coefficients[]) {
    return (coefficients[0] * (point * point * point) +
            coefficients[1] * (point * point) + coefficients[2] * point +
            coefficients[3]);
}

/*
this method takes the cleaned (i.e. only one neutral cal point) points and
generates the linearization coefficients it then linearizes the provided
calibration points
*/
void linearize_cal(const float cleaned_points_x[],
                   const float cleaned_points_y[], float linearized_points_x[],
                   float linearized_points_y[],
                   calib_results_t *calib_results) {

    // for readability
    const float *in_x = cleaned_points_x;
    const float *in_y = cleaned_points_y;

    float *out_x = linearized_points_x;
    float *out_y = linearized_points_y;

    double fit_points_x[5];
    double fit_points_y[5];

    fit_points_x[0] = (double)in_x[3];                   // left
    fit_points_x[1] = (double)(in_x[6] + in_x[7]) / 2.0; // left diagonal
    fit_points_x[2] = (double)in_x[0];                   // center
    fit_points_x[3] = (double)(in_x[5] + in_x[8]) / 2.0; // right diagonal
    fit_points_x[4] = (double)in_x[1];                   // right

    fit_points_y[0] = (double)in_y[2];                   // up
    fit_points_y[1] = (double)(in_y[5] + in_y[6]) / 2.0; // up diagonal
    fit_points_y[2] = (double)in_y[0];                   // center
    fit_points_y[3] = (double)(in_y[7] + in_y[8]) / 2.0; // down diagonal
    fit_points_y[4] = (double)in_y[4];                   // down

    for (int i = 0; i < 5; i++) {
        debug_print("Fit point %d; (x,y) = (%f, %f)\n", i, fit_points_x[i],
                    fit_points_y[i]);
    }

    double temp_coeffs_x[FIT_ORDER + 1];
    double temp_coeffs_y[FIT_ORDER + 1];

    polyfit(5, fit_points_x, perfect_angles, FIT_ORDER + 1, temp_coeffs_x);
    polyfit(5, fit_points_y, perfect_angles, FIT_ORDER + 1, temp_coeffs_y);

    // write these coefficients to the array that was passed in, this is our
    // first output
    for (int i = 0; i < (FIT_ORDER + 1); i++) {
        calib_results->fit_coeffs_x[i] = (float)temp_coeffs_x[i];
        calib_results->fit_coeffs_y[i] = (float)temp_coeffs_y[i];
    }

    for (int i = 0; i <= NUM_NOTCHES; i++) {
        out_x[i] = linearize(in_x[i], calib_results->fit_coeffs_x);
        out_y[i] = linearize(in_y[i], calib_results->fit_coeffs_y);
    }
}

// Invert a 3x3 matrix
void inverse(const float in[3][3], float out[3][3]) {
    float det = in[0][0] * (in[1][1] * in[2][2] - in[2][1] * in[1][2]) -
                in[0][1] * (in[1][0] * in[2][2] - in[1][2] * in[2][0]) +
                in[0][2] * (in[1][0] * in[2][1] - in[1][1] * in[2][0]);
    float invdet = 1 / det;

    out[0][0] = (in[1][1] * in[2][2] - in[2][1] * in[1][2]) * invdet;
    out[0][1] = (in[0][2] * in[2][1] - in[0][1] * in[2][2]) * invdet;
    out[0][2] = (in[0][1] * in[1][2] - in[0][2] * in[1][1]) * invdet;
    out[1][0] = (in[1][2] * in[2][0] - in[1][0] * in[2][2]) * invdet;
    out[1][1] = (in[0][0] * in[2][2] - in[0][2] * in[2][0]) * invdet;
    out[1][2] = (in[1][0] * in[0][2] - in[0][0] * in[1][2]) * invdet;
    out[2][0] = (in[1][0] * in[2][1] - in[2][0] * in[1][1]) * invdet;
    out[2][1] = (in[2][0] * in[0][1] - in[0][0] * in[2][1]) * invdet;
    out[2][2] = (in[0][0] * in[1][1] - in[1][0] * in[0][1]) * invdet;
}

void notch_remap(const float x_in, const float y_in, float *x_out, float *y_out,
                 const calib_results_t *calib_results) {
    // determine the angle between the x unit vector and the current position
    // vector
    float angle = atan2f(x_in, y_in);

    // unwrap the angle based on the first region boundary
    if (angle < calib_results->boundary_angles[0]) {
        angle += M_PI * 2;
    }

    // go through the region boundaries from lowest angle to highest, checking
    // if the current position vector is in that region if the region is not
    // found then it must be between the first and the last boundary, ie the
    // last region we check GATE_REGIONS*2 because each notch has its own very
    // small region we use to make notch values more consistent
    int region = NUM_NOTCHES - 1;
    for (int i = 1; i < NUM_NOTCHES; i++) {
        if (angle < calib_results->boundary_angles[i]) {
            region = i - 1;
            break;
        }
    }

    // Apply the affine transformation using the coefficients found during
    // calibration
    *x_out = calib_results->affine_coeffs[region][0] * x_in +
             calib_results->affine_coeffs[region][1] * y_in;
    *y_out = calib_results->affine_coeffs[region][2] * x_in +
             calib_results->affine_coeffs[region][3] * y_in;
}

// Multiply two 3x3 matrices
void matrix_matrix_mult(const float left[3][3], const float right[3][3],
                        float output[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            output[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                output[i][j] += left[i][k] * right[k][j];
            }
        }
    }
}

void print_mtx(const float matrix[3][3]) {
    int i, j, nrow, ncol;
    nrow = 3;
    ncol = 3;
    debug_print("\n");
    for (i = 0; i < nrow; i++) {
        for (j = 0; j < ncol; j++) {
            debug_print("%.6f, ", matrix[i][j]); // print 6 decimal places
        }
        debug_print("\n");
    }
    debug_print("\n");
}

void notch_calibrate(const float in_points_x[], const float in_points_y[],
                     float notch_points_x[], float notch_points_y[],
                     calib_results_t *calib_results) {

    for (int i = 1; i <= NUM_NOTCHES; i++) {
        //("calibration region %d\n", i);

        float pointsIn[3][3];
        float pointsOut[3][3];

        if (i == (NUM_NOTCHES)) {
            // debug_print("final region\n");
            pointsIn[0][0] = in_points_x[0];
            pointsIn[0][1] = in_points_x[i];
            pointsIn[0][2] = in_points_x[1];
            pointsIn[1][0] = in_points_y[0];
            pointsIn[1][1] = in_points_y[i];
            pointsIn[1][2] = in_points_y[1];
            pointsIn[2][0] = 1;
            pointsIn[2][1] = 1;
            pointsIn[2][2] = 1;
            pointsOut[0][0] = notch_points_x[0];
            pointsOut[0][1] = notch_points_x[i];
            pointsOut[0][2] = notch_points_x[1];
            pointsOut[1][0] = notch_points_y[0];
            pointsOut[1][1] = notch_points_y[i];
            pointsOut[1][2] = notch_points_y[1];
            pointsOut[2][0] = 1;
            pointsOut[2][1] = 1;
            pointsOut[2][2] = 1;
        } else {
            pointsIn[0][0] = in_points_x[0];
            pointsIn[0][1] = in_points_x[i];
            pointsIn[0][2] = in_points_x[i + 1];
            pointsIn[1][0] = in_points_y[0];
            pointsIn[1][1] = in_points_y[i];
            pointsIn[1][2] = in_points_y[i + 1];
            pointsIn[2][0] = 1;
            pointsIn[2][1] = 1;
            pointsIn[2][2] = 1;
            pointsOut[0][0] = notch_points_x[0];
            pointsOut[0][1] = notch_points_x[i];
            pointsOut[0][2] = notch_points_x[i + 1];
            pointsOut[1][0] = notch_points_y[0];
            pointsOut[1][1] = notch_points_y[i];
            pointsOut[1][2] = notch_points_y[i + 1];
            pointsOut[2][0] = 1;
            pointsOut[2][1] = 1;
            pointsOut[2][2] = 1;
        }

        // debug_print("In points:\n");
        // print_mtx(pointsIn);
        // debug_print("Out points:\n");
        // print_mtx(pointsOut);

        float temp[3][3];
        inverse(pointsIn, temp);

        float A[3][3];
        matrix_matrix_mult(pointsOut, temp, A);

        // debug_print("The transform matrix is:\n");
        // print_mtx(A);

        // debug_print("The affine transform coefficients for this region
        // are:\n");

        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                calib_results->affine_coeffs[i - 1][j * 2 + k] = A[j][k];
                // debug_print(calib_results->affine_coeffs[i-1][j*2+k]);
                // debug_print(",");
            }
        }

        // debug_print("\nThe angle defining this  regions is:\n");
        calib_results->boundary_angles[i - 1] =
            atan2f((in_points_y[i] - in_points_y[0]),
                   (in_points_x[i] - in_points_x[0]));
        // unwrap the angles so that the first has the smallest value
        if (calib_results->boundary_angles[i - 1] <
            calib_results->boundary_angles[0]) {
            calib_results->boundary_angles[i - 1] += M_PI * 2;
        }
        // debug_print("%d\n", calib_results->boundary_angles[i-1]);
    }
}

/*
This method is SUPER important because it captures the signal chain of the stick
reports. Look here to find the sauce
*/
void process_stick(const raw_report_t *raw_report,
                   calib_results_t *calib_results,
                   processed_stick_t *stick_out) {

    float linearized_x =
        linearize(raw_report->stick_x, calib_results->fit_coeffs_x);
    float linearized_y =
        linearize(raw_report->stick_y, calib_results->fit_coeffs_y);

    float remapped_x, remapped_y;
    notch_remap(linearized_x, linearized_y, &remapped_x, &remapped_y,
                calib_results);

    float clamped_x = fmin(127, fmax(-128, remapped_x));
    float clamped_y = fmin(127, fmax(-128, remapped_y));

    stick_out->x = (int8_t)(clamped_x);
    stick_out->y = (int8_t)(clamped_y);

#ifdef DEBUG
    _dbg_report.stick_x_raw = raw_report->stick_x;
    _dbg_report.stick_y_raw = raw_report->stick_y;
    _dbg_report.stick_x_lin = linearized_x;
    _dbg_report.stick_y_lin = linearized_y;
#endif
}