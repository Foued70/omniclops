/*
    Stereo correspondence for stacked omnidirectional vision
    Copyright (C) 2010 Bob Mottram
    fuzzgun@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stackedstereo.h"

/*!
 * \brief returns a set of features along a single vertical column (x coordinate)
 * \param unwarped_img unwarped colour image
 * \param img_width width of the image
 * \param img_height height of the image
 * \param x x coordinate for the column
 * \param magnitude array storing refelectance values
 * \param edgemagnitude array used for non-maximal suppression
 * \param features array used to store detected edge y coordinates
 */
int stackedstereo::get_features_from_unwarped(
	unsigned char* unwarped_img,
	int img_width,
	int img_height,
	int x,
	short* magnitude,
	short* edge_magnitude,
	short* features)
{
	int no_of_features = 0;
	int suppression_radius = 10;

	for (int y = 3; y < img_height-3; y++) {
		int n = (y*img_width + x)*3;
		magnitude[y] = short(unwarped_img[n] + unwarped_img[n+1] + unwarped_img[n+2]);
	}
	for (int y = 3; y < img_height-3; y++) {
		edge_magnitude[y] =
			(short)((abs(magnitude[y-1] - magnitude[y+1]) * 4) +
			abs(magnitude[y-2] - magnitude[y+2]) +
			abs(magnitude[y-3] - magnitude[y+3]));
	}

	// non-maximal suppression
	for (int y = 3; y < img_height-suppression_radius; y++) {
		short mag = edge_magnitude[y];
		if (mag > 0) {
			for (int y2 = y+1; y2 < y + suppression_radius; y2++) {
				if (edge_magnitude[y2] <= mag) {
					edge_magnitude[y2] = 0;
				}
				else {
					edge_magnitude[y] = 0;
					break;
				}
			}
			if (edge_magnitude[y] > (short)0) {
				if (no_of_features < MAX_STACKED_STEREO_FEATURES) {
					features[no_of_features] = y;
					no_of_features++;
				}
				else {
					printf("Maximum number of stereo features per radial line reached\n");
				}
			}
		}
	}
	return(no_of_features);
}

/*!
 * \brief computes the sum of squared differences for a pair of image patches
 * \param img colour image
 * \param img_width width of the image
 * \param img_height height of the image
 * \param x0 x centre coordinate of the first patch
 * \param y0 y centre coordinate of the first patch
 * \param x1 x centre coordinate of the second patch
 * \param y1 y centre coordinate of the second patch
 * \param patch_width_pixels patch width
 * \param patch_height_pixels patch *height
 * \param sampling_step sub-sampling step size
 */
unsigned int stackedstereo::SSD(
	unsigned char *img,
	int img_width,
	int img_height,
	int x0, int y0,
	int x1, int y1,
	int patch_width_pixels,
	int patch_height_pixels,
	int sampling_step)
{
	unsigned int ssd = 0;

	for (int dx = -patch_width_pixels; dx <= patch_width_pixels; dx += sampling_step) {
		int xx0 = x0 + dx;
		int xx1 = x1 + dx;
		if ((xx0 > -1) && (xx1 > -1) && (xx0 < img_width) && (xx1 < img_width)) {
			for (int dy = -patch_height_pixels; dy <= patch_height_pixels; dy += sampling_step) {
				int yy0 = y0 + dy;
				int yy1 = y1 + dy;
				if ((yy0 > -1) && (yy1 > -1) && (yy0 < img_height) && (yy1 < img_height)) {
					int n0 = (yy0*img_width + xx0)*3;
					int n1 = (yy1*img_width + xx1)*3;
					int b = img[n0] - img[n1];
					int g = img[n0+1] - img[n1+1];
					int r = img[n0+2] - img[n1+2];
					ssd += (unsigned int)(r*r + g*g + b*b);
				}
			}
		}
	}
	return(ssd);
}

/*!
 * \brief compares two image patches and returns a value indicating how similar they are
 * \param img colour image
 * \param img_width width of the image
 * \param img_height height of the image
 * \param x0 x coordinate of the centre of the first patch
 * \param y0 y coordinate of the centre of the first patch
 * \param x1 x coordinate of the centre of the second patch
 * \param y1 y coordinate of the centre of the second patch
 * \param patch_width_pixels width of the patch
 * \param patch_height_pixels height of the patch
 * \param sampling_step step size used to sample the patch
 * \return correlation value
 */
int stackedstereo::Correlation(
	unsigned char *img,
	int img_width,
	int img_height,
	int x0, int y0,
	int x1, int y1,
	int patch_width_pixels,
	int patch_height_pixels,
	int sampling_step)
{
	int correlation = 0;
	int anticorrelation = 0;
	int average = 0;
	int hits = 0;

	// compute correlation
	for (int dx = -patch_width_pixels; dx <= patch_width_pixels; dx += sampling_step) {
		int xx0 = x0 + dx;
		int xx1 = x1 + dx;
		if ((xx0 > -1) && (xx1 > -1) && (xx0 < img_width) && (xx1 < img_width)) {
			for (int dy = -patch_height_pixels; dy <= patch_height_pixels; dy += sampling_step) {
				int yy0 = y0 + dy;
				int yy1 = y1 + dy;
				if ((yy0 > -1) && (yy1 > -1) && (yy0 < img_height) && (yy1 < img_height)) {
					int n0 = (yy0*img_width + xx0)*3;
					int n1 = (yy1*img_width + xx1)*3;
					int b = 255 - (img[n0] - img[n1]);
					int g = 255 - (img[n0+1] - img[n1+1]);
					int r = 255 - (img[n0+2] - img[n1+2]);
					average += img[n0] + img[n0+1] + img[n0+2];
					hits += 3;
					correlation += r*r + g*g + b*b;
				}
			}
		}
	}
	if (hits > 0) average /= hits;

	// compute anticorrelation
	for (int dx = -patch_width_pixels; dx <= patch_width_pixels; dx += sampling_step) {
		int xx0 = x0 + dx;
		int xx1 = x1 + dx;
		if ((xx0 > -1) && (xx1 > -1) && (xx0 < img_width) && (xx1 < img_width)) {
			for (int dy = -patch_height_pixels; dy <= patch_height_pixels; dy += sampling_step) {
				int yy0 = y0 + dy;
				int yy1 = y1 + dy;
				if ((yy0 > -1) && (yy1 > -1) && (yy0 < img_height) && (yy1 < img_height)) {
					int n0 = (yy0*img_width + xx0)*3;
					int n1 = (yy1*img_width + xx1)*3;
					int b = 255 - ((average - (img[n0] - average)) - img[n1]);
					int g = 255 - ((average - (img[n0+1] - average)) - img[n1+1]);
					int r = 255 - ((average - (img[n0+2] - average)) - img[n1+2]);
					anticorrelation += r*r + g*g + b*b;
				}
			}
		}
	}

	return(correlation - anticorrelation);
}

/*!
 * \brief creates an anaglyph by overlaying the lower mirror image onto the upper mirror one
 * \param img colour image
 * \param img_width width of the image
 * \param img_height height of the image
 * \param offset_y offset used to vertically align the two images
 */
void stackedstereo::anaglyph(
	unsigned char* img,
	int img_width,
	int img_height,
	int offset_y)
{
	int h = img_height/2;
	for (int y = h; y < img_height; y++) {
		int y2 = y - h + offset_y;
		if (y2 > -1) {
			for (int x = 0; x < img_width; x++) {
				int n_lower = (y*img_width + x)*3;
				if (!((img[n_lower] == 0) && (img[n_lower+1] == 0) && (img[n_lower+2] == 0))) {
					int n_upper = (y2*img_width + x)*3;
					img[n_upper] = img[n_lower];
					img[n_upper+1] = 0;
					img[n_lower] = 0;
					img[n_lower+1] = 0;
					img[n_lower+2] = 0;
				}
			}
		}
	}

	int n = 0;
	for (int y = 0; y < img_height; y++) {
		for (int x = 0; x < img_width; x++, n += 3) {
			if (img[n+1] != 0) {
				img[n] = 0;
				img[n+1] = 0;
				img[n+2] = 0;
			}
		}
	}
}

/*!
 * \brief computes calibration offsets
 * \param img colour image
 * \param img_width width of the image
 * \param img_height height of the image
 * \param offset_y returned Y offset in pixels used to compare the lower mirror image to the upper one
 */
void stackedstereo::calibrate(
	unsigned char* img,
	int img_width,
	int img_height,
	int &offset_y)
{
	int patch_width_pixels = img_width/2;
	int patch_height_pixels = img_height/4;
	int min_offset_y = -img_height/8;
	int max_offset_y = img_height/8;

	int step = 2;
	unsigned int min_ssd = 0;
	int x = img_width/2;
    for (int offset = min_offset_y; offset <= max_offset_y; offset++) {
    	int y_upper = patch_height_pixels;
    	int y_lower = (img_height/2) + patch_height_pixels + offset;
        unsigned int ssd = SSD(
        	img, img_width, img_height,
        	x, y_upper,
        	x, y_lower,
        	patch_width_pixels,
        	patch_height_pixels,
        	step);
        if ((ssd < min_ssd) || (min_ssd == 0)) {
        	min_ssd = ssd;
        	offset_y = -offset;
        }
    }
}

void stackedstereo::show_matched_features(
	unsigned char* img,
	int img_width,
	int img_height,
	int max_disparity_percent,
	int no_of_matches,
	int* matches)
{
	int half_height = img_height/2;
	int max_disparity_pixels = half_height * max_disparity_percent / 100;

	for (int i = 0; i < no_of_matches; i++) {
		int x = matches[i*4 + 1];
		int y = matches[i*4 + 2];
		int disparity = matches[i*4+3];
		int radius = 1 + (disparity * 10 / max_disparity_pixels);
		drawing::drawCircle(img, img_width, img_height, x,y,radius,255,0,0,0);
	}
}

int stackedstereo::stereo_match(
	unsigned char* img,
	int img_width,
	int img_height,
	int offset_y,
	int step_x,
	int max_disparity_percent,
	int desired_no_of_matches,
	int *matches)
{
	int no_of_matches = 0;
	short magnitude[img_height];
	short edge_magnitude[img_height];
	short features[MAX_STACKED_STEREO_FEATURES];

	const int patch_width = 2;
	const int patch_height = 8;
	int half_height = img_height/2;
	int min_disparity_pixels = -3;
	int max_disparity_pixels = half_height * max_disparity_percent / 100;

	for (int x = 0; x < img_width; x += step_x) {

		// detect edge features
		int no_of_features = get_features_from_unwarped(
		    img,
		    img_width,
		    img_height,
		    x,
		    magnitude,
		    edge_magnitude,
		    features);

		// find the index of the first edge feature in the lower mirror image
	    int first_lower_mirror_feature_index = 0;
	    while (first_lower_mirror_feature_index < no_of_features) {
	    	if (features[first_lower_mirror_feature_index] >= half_height) break;
	    	first_lower_mirror_feature_index++;
	    }

	    // compute possible correspondences
	    for (int i = 0; i < first_lower_mirror_feature_index; i++) {
	    	int y_upper = features[i];
	    	int max_correlation = 0;
	    	int best_disparity = -1;
		    for (int j = first_lower_mirror_feature_index; j < no_of_features; j++) {
		    	int y_lower = features[j] - half_height + offset_y;
		    	int disparity = y_upper - y_lower;
		    	if ((disparity > min_disparity_pixels) &&
		    		(disparity < max_disparity_pixels)) {
		    		if (disparity < 0) disparity = 0;
		    		// compute a correlation value for these two features
                    int correlation = Correlation(img, img_width, img_height, x, y_upper, x, features[j], patch_width, patch_height, 1);
                    if (correlation > max_correlation) {
                    	max_correlation = correlation;
                    	best_disparity = disparity;
                    }
		    	}
		    }
		    if ((best_disparity > -1) &&
		    	(no_of_matches < MAX_STACKED_STEREO_MATCHES)) {
		    	// store possible match
		    	matches[no_of_matches*4] = max_correlation;
		    	matches[no_of_matches*4+1] = x;
		    	matches[no_of_matches*4+2] = y_upper;
		    	matches[no_of_matches*4+3] = best_disparity;
		    	no_of_matches++;
		    }
	    }
	}

	// sort matches in descending order of correlation value
	if (no_of_matches < desired_no_of_matches) desired_no_of_matches = no_of_matches;
    for (int i = 0; i < desired_no_of_matches; i++) {
    	int best_correlation = matches[i*4];
    	int winner = -1;
    	for (int j = i+1; j < no_of_matches; j++) {
    		if (matches[j*4] > best_correlation) {
    			best_correlation = matches[j*4];
    			winner = j;
    		}
    	}
    	if (winner > -1) {
    		for (int j = 0; j < 4; j++) {
    			int temp = matches[i*4 + j];
    			matches[i*4 + j] = matches[winner*4 + j];
    			matches[winner*4 + j] = temp;
    		}
    	}
    }

    return(desired_no_of_matches);
}
