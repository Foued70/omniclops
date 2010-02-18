/*
    used to detect vertical features, such as walls or doorways
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

#include "detectverticals.h"

/*!
 * \brief returns edge features which are above the floor level
 * \param features list of edge features for the entire image
 * \param floor_features list of features belonging to the ground plane
 * \param no_of_mirrors number of mirrors
 * \param ray_map_width image width
 * \param ray_map_height image height
 * \param mirror_map lookup table containing mirror indexes
 * \param vertical_features returned edge features above the ground plane
 */
void detectverticals::get_vertical_features(
	vector<int> &features,
	vector<int> &floor_features,
	int no_of_mirrors,
	int ray_map_width,
	int ray_map_height,
	unsigned char* mirror_map,
    vector<int> &vertical_features)
{
	// find features which are above the ground plane
	vertical_features.clear();
	for (int i = (int)features.size()-2; i >= 0; i -= 2) {
		int x0 = features[i];
		int y0 = features[i+1];
		int n0 = y0 * ray_map_width + x0;
		if (mirror_map[n0] == no_of_mirrors) {

			// does this feature belong on the ground plane?
			bool on_ground_plane = false;
			for (int j = (int)floor_features.size()-2; j >= 0; j -= 2) {
				int x1 = floor_features[j];
				int y1 = floor_features[j+1];
				if ((x1 == x0) && (y1 == y0)) {
					on_ground_plane = true;
					break;
				}
			}
			if (!on_ground_plane) {
				vertical_features.push_back(x0);
				vertical_features.push_back(y0);
			}
		}
	}
}

void detectverticals::get_possible_verticals(
	vector<int> &features,
	int* mirror_centre_pixels,
	int min_radius_percent,
	int max_radius_percent,
	int no_of_mirrors,
	int ray_map_width,
	unsigned char* mirror_map,
    vector<int> &possible_verticals)
{
	possible_verticals.clear();

	int min_radius_pixels = min_radius_percent * ray_map_width / 100;
    int max_radius_pixels = max_radius_percent * ray_map_width / 100;

    min_radius_pixels *= min_radius_pixels;
    max_radius_pixels *= max_radius_pixels;

    for (int i = (int)features.size()-2; i >= 0; i -= 2) {
    	int x = features[i];
    	int y = features[i+1];
    	int n = y*ray_map_width + x;
    	if ((mirror_map[n] > 0) && (mirror_map[n] != no_of_mirrors)) {
    		int mirror = mirror_map[n]-1;
    		int dx = x - mirror_centre_pixels[mirror*2];
    		int dy = y - mirror_centre_pixels[mirror*2 + 1];
    		int dist_sqr = dx*dx + dy*dy;
    		if ((dist_sqr > min_radius_pixels) && (dist_sqr < max_radius_pixels)) {
                possible_verticals.push_back(x);
                possible_verticals.push_back(y);
    		}
    	}
    }
}


void detectverticals::detect(
	vector<int> &features,
	vector<int> &floor_features,
	int* mirror_centre_pixels,
	int min_radius_percent,
	int max_radius_percent,
	int no_of_mirrors,
	int ray_map_width,
	int ray_map_height,
	int* ray_map,
	unsigned char* mirror_map,
	int max_range_mm,
	vector<int> &vertical_features,
    vector<int> &points)
{
	points.clear();
	int max_range_mm_sqr = max_range_mm*max_range_mm;

	// find vertical features within the centre mirror
	get_vertical_features(
		features,
		floor_features,
		no_of_mirrors,
		ray_map_width,
		ray_map_height,
		mirror_map,
	    vertical_features);

	// get possible vertical features in mirrors other than the centre one
	vector<int> possible_verticals;
	get_possible_verticals(
		features,
		mirror_centre_pixels,
		min_radius_percent,
		max_radius_percent,
		no_of_mirrors,
		ray_map_width,
		mirror_map,
	    possible_verticals);

    float xi=0, yi=0;

	for (int i = (int)vertical_features.size()-2; i >= 0; i -= 2) {
		int x0 = vertical_features[i];
		int y0 = vertical_features[i+1];
		int n = (y0*ray_map_width + x0)*6;
		int xx0 = ray_map[n];
		int yy0 = ray_map[n+1];
		int xx1 = ray_map[n+3];
		int yy1 = ray_map[n+4];
		int dx = xx1 - xx0;
		int dy = yy1 - yy0;
		xx1 = xx0 + (dx*1000);
		yy1 = yy0 + (dy*1000);

		for (int j = (int)possible_verticals.size()-2; i >= 0; i -= 2) {
			int x1 = possible_verticals[j];
			int y1 = possible_verticals[j+1];
			n = (y1*ray_map_width + x1)*6;
			int xx2 = ray_map[n];
			int yy2 = ray_map[n+1];
			int xx3 = ray_map[n+3];
			int yy3 = ray_map[n+4];
			dx = xx3 - xx2;
			dy = yy3 - yy2;
			xx3 = xx2 + (dx*1000);
			yy3 = yy2 + (dy*1000);

		    if (omni::intersection(
		        xx0,yy0,xx1,yy1,
		        xx2,yy2,xx3,yy3,
		        xi, yi)) {
		    	int range = (int)(xi*xi + yi*yi);
		    	if (range < max_range_mm_sqr) {
		    		points.push_back((int)xi);
		    		points.push_back((int)yi);
		    		points.push_back(i);
		    	}
		    }

		}
	}
}