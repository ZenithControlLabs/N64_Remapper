#include "Phobri64.h"
#include "curve_fitting.h"

float linearize(const float point, const float coefficients[]) {
	return (coefficients[0]*(point*point*point) + coefficients[1]*(point*point) + coefficients[2]*point + coefficients[3]);
}

/*
 * calcStickValues computes the stick x/y coordinates from angle.
 * This requires weird trig because the stick moves spherically.
 */
void calc_stick_values(float angle, float* x, float* y) {
	*x = 100*atan2f((sinf(MAX_STICK_ANGLE)*cosf(angle)),cosf(MAX_STICK_ANGLE))/MAX_STICK_ANGLE;
	*y = 100*atan2f((sinf(MAX_STICK_ANGLE)*sinf(angle)),cosf(MAX_STICK_ANGLE))/MAX_STICK_ANGLE;
}

/*
 * Convert the x/y coordinates (actually angles on a sphere) to an azimuth
 * We first convert to a 3D coordinate and then drop to 2D, then arctan it
 * This does the opposite of calcStickValues, ideally.
 */
float angle_on_sphere(const float x, const float y) {
	float xx = sinf(x * MAX_STICK_ANGLE/100) * cosf(y * MAX_STICK_ANGLE/100);
	float yy = cosf(x * MAX_STICK_ANGLE/100) * sinf(y * MAX_STICK_ANGLE/100);
	float angle = atan2f(yy, xx); // WHY IS THIS BACKWARDS
	if(angle < 0){
		angle += 2*M_PI;
	}

    return angle;
}


void clean_cal_points(const float raw_cal_points_x[], const float raw_cal_points_y[], float cleaned_points_x[], float cleaned_points_y[]) {
	cleaned_points_x[0] = 0;
	cleaned_points_y[0] = 0;

	for (int i = 0; i < NUM_NOTCHES; i++) {
		// each origin reading is summed together
		cleaned_points_x[0] += raw_cal_points_x[i*2];
		cleaned_points_y[0] += raw_cal_points_y[i*2];

		// for the notches, copy point into cleaned list
		cleaned_points_x[i+1] = raw_cal_points_x[i*2+1];
		cleaned_points_y[i+1] = raw_cal_points_y[i*2+1];
	}

	// remove the largest and smallest origin values to remove outliers
	// first, find their indices

	int smallestX = 0;
	int largestX = 0;
	int smallestY = 0;
	int largestY = 0;
	for (int i = 0; i < NUM_NOTCHES; i++){
		if (raw_cal_points_x[i*2] < raw_cal_points_x[smallestX]){
			// record the new smallest index
			smallestX = i*2;
		} else if (raw_cal_points_x[i*2] > raw_cal_points_x[largestX]){
			// record the new largest index
			largestX = i*2;
		}

		if (raw_cal_points_y[i*2] < raw_cal_points_y[smallestY]){
			// record the new smallest index
			smallestY = i*2;
		} else if (raw_cal_points_y[i*2] > raw_cal_points_y[largestY]){
			// record the new largest index
			largestY = i*2;
		}
	}
	// subtract the smallest and largest values
	cleaned_points_x[0] -= raw_cal_points_x[smallestX];
	cleaned_points_x[0] -= raw_cal_points_x[largestX];
	cleaned_points_y[0] -= raw_cal_points_y[smallestY];
	cleaned_points_y[0] -= raw_cal_points_y[largestY];

	//divide by the total number of calibration steps/2 to get the average origin value
	//except it's minus 4 steps since we removed outliers
	cleaned_points_x[0] = cleaned_points_x[0]/((float)NUM_NOTCHES-2);
	cleaned_points_y[0] = cleaned_points_y[0]/((float)NUM_NOTCHES-2);
}

/*
this method takes the cleaned (i.e. only one neutral cal point) points and generates the linearization coefficients
it then linearizes the provided calibration points 
*/
void linearize_cal(const float cleaned_points_x[], const float cleaned_points_y[], float linearized_points_x[], float linearized_points_y[], stick_params_t *stick_params) {

	// for readability
	const float* in_x = cleaned_points_x;
	const float* in_y = cleaned_points_y;

	float* out_x = linearized_points_x;
	float* out_y = linearized_points_y;

	float fit_points_x[5];
	float fit_points_y[5];

	fit_points_x[0] = in_x[8+1];                     // right
	fit_points_x[1] = (in_x[6+1] + in_x[10+1])/2.0;  // right diagonal
	fit_points_x[2] = in_x[0];                       // center
	fit_points_x[3] = (in_x[2+1] + in_x[14+1])/2.0;  // left diagonal
	fit_points_x[4] = in_x[0+1];                     // left

	fit_points_y[0] = in_y[12+1];                    // down
	fit_points_y[1] = (in_y[10+1] + in_y[14+1])/2.0; // down diagonal
	fit_points_y[2] = in_y[0];                       // center
	fit_points_y[3] = (in_y[6+1] + in_y[2+1])/2.0;   // up diagonal
	fit_points_y[4] = in_y[4+1];                     // up	

	float* x_output = perfect_angles;
	float* y_output = perfect_angles;

	float temp_coeffs_x[FIT_ORDER + 1];
	float temp_coeffs_y[FIT_ORDER + 1];

	fitCurve(FIT_ORDER, 5, fit_points_x, x_output, FIT_ORDER+1, temp_coeffs_x);
	fitCurve(FIT_ORDER, 5, fit_points_y, x_output, FIT_ORDER+1, temp_coeffs_y);

	//write these coefficients to the array that was passed in, this is our first output
	for(int i = 0; i < (FIT_ORDER+1); i++){
		stick_params->fit_coeffs_x[i] = temp_coeffs_x[i];
		stick_params->fit_coeffs_x[i] = temp_coeffs_y[i];
	}

	for (int i = 0; i <= NUM_NOTCHES; i++) {
		out_x[i] = linearize(in_x[i], stick_params->fit_coeffs_x);
		out_y[i] = linearize(in_y[i], stick_params->fit_coeffs_y);
	}
}

/*
//Self-explanatory.
void inverse(const float in[3][3], float (&out)[3][3])
{
	float det = in[0][0] * (in[1][1]*in[2][2] - in[2][1]*in[1][2]) -
	            in[0][1] * (in[1][0]*in[2][2] - in[1][2]*in[2][0]) +
	            in[0][2] * (in[1][0]*in[2][1] - in[1][1]*in[2][0]);
	float invdet = 1 / det;

	out[0][0] = (in[1][1]*in[2][2] - in[2][1]*in[1][2]) * invdet;
	out[0][1] = (in[0][2]*in[2][1] - in[0][1]*in[2][2]) * invdet;
	out[0][2] = (in[0][1]*in[1][2] - in[0][2]*in[1][1]) * invdet;
	out[1][0] = (in[1][2]*in[2][0] - in[1][0]*in[2][2]) * invdet;
	out[1][1] = (in[0][0]*in[2][2] - in[0][2]*in[2][0]) * invdet;
	out[1][2] = (in[1][0]*in[0][2] - in[0][0]*in[1][2]) * invdet;
	out[2][0] = (in[1][0]*in[2][1] - in[2][0]*in[1][1]) * invdet;
	out[2][1] = (in[2][0]*in[0][1] - in[0][0]*in[2][1]) * invdet;
	out[2][2] = (in[0][0]*in[1][1] - in[1][0]*in[0][1]) * invdet;
}

//Self-explanatory.
void matrixMatrixMult(const float left[3][3], const float right[3][3], float (&output)[3][3])
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			output[i][j] = 0;
			for (int k = 0; k < 3; k++)
			{
				output[i][j] += left[i][k] * right[k][j];
			}
		}
	}
}

void print_mtx(const float matrix[3][3]){
	int i, j, nrow, ncol;
	nrow = 3;
	ncol = 3;
	printf("\n");
	for (i=0; i<nrow; i++)
	{
		for (j=0; j<ncol; j++)
		{
			printf(matrix[i][j], 6);   // print 6 decimal places
			printf(", ");
		}
		printf("\n");
	}
	printf("\n");
};

void notchCalibrate(const float in_points_x[], const float in_points_y[], float notch_points_x[], float notch_points_y[], stick_params_t *stick_params) {

	for(int i = 1; i <= NUM_NOTCHES; i++){
		printf("calibration region %d\n", i);

		float pointsIn[3][3];
		float pointsOut[3][3];

		if(i == (regions)){
			printf("final region\n");
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
		}
		else{
			pointsIn[0][0] = in_points_x[0];
			pointsIn[0][1] = in_points_x[i];
			pointsIn[0][2] = in_points_x[i+1];
			pointsIn[1][0] = in_points_y[0];
			pointsIn[1][1] = in_points_y[i];
			pointsIn[1][2] = in_points_y[i+1];
			pointsIn[2][0] = 1;
			pointsIn[2][1] = 1;
			pointsIn[2][2] = 1;
			pointsOut[0][0] = notch_points_x[0];
			pointsOut[0][1] = notch_points_x[i];
			pointsOut[0][2] = notch_points_x[i+1];
			pointsOut[1][0] = notch_points_y[0];
			pointsOut[1][1] = notch_points_y[i];
			pointsOut[1][2] = notch_points_y[i+1];
			pointsOut[2][0] = 1;
			pointsOut[2][1] = 1;
			pointsOut[2][2] = 1;
		}

		printf("In points:\n");
		print_mtx(pointsIn);
		debug_println("Out points:");
		print_mtx(pointsOut);

		float temp[3][3];
		inverse(pointsIn, temp);

		float A[3][3];
		matrixMatrixMult(pointsOut, temp, A);

		printf("The transform matrix is:\n");
		print_mtx(A);

		printf("The affine transform coefficients for this region are:\n");

		for(int j = 0; j <2;j++){
			for(int k = 0; k<2;k++){
				stickParams.affineCoeffs[i-1][j*2+k] = A[j][k];
				debug_print(stickParams.affineCoeffs[i-1][j*2+k]);
				debug_print(",");
			}
		}

		printf("\nThe angle defining this  regions is:\n");
		stickParams.boundaryAngles[i-1] = atan2f((yIn[i]-yIn[0]),(xIn[i]-xIn[0]));
		//unwrap the angles so that the first has the smallest value
		if(stickParams.boundaryAngles[i-1] < stickParams.boundaryAngles[0]){
			stickParams.boundaryAngles[i-1] += M_PI*2;
		}
		printf("%d\n", stickParams.boundaryAngles[i-1]);
	}
};
*/