#pragma once
#include "includes.h"

namespace  listCommands {
	// please add extra space at the end
	// HEADER meanings
	// D = DWORD (value within 0 - 4,294,967,295)
	// B = BOOLEAN (0 - 1)
	std::string d_CameraDistance = "dota_camera_distance "; // Cameradistance from ground
	std::string b_Fog = "fog_enable "; // Enable or disable fog
	std::string b_ParticleHasLimit = "dota_use_particle_fow "; //Show hidden spells (particles) and teleports in map's fog = 0, 1 
	std::string b_hitboxes = "dota_unit_show_selection_boxes ";//Draws selection hitboxes. 0 = off, 1 = non-trees, 2 = trees, 3 = all entities
	std::string b_rfarz = "r_farz ";//default -1 multiply it always 2x from distance
	std::string d_range_display = "dota_range_display ";//any number Displays a ring around the hero at the specified radius
	std::string d_cl_weather = "cl_weather "; //	"Default = 0" 		"Snow" 		"Rain" 		"Moonbeam"		"Pestilence"		"Harvest"		"Sirocco"		"Spring"		"Ash"		"Aurora"
	std::string b_fowaa = "fow_client_nofiltering "; //1 - Remove anti-aliasing of fog


}