#
#  Dark Oberon - Terrain Scheme
#

name "Plastic World"
author "PeterPP & Crazych"


#
#  Materials
#

<Materials>
  count 3
 
  <Material 0>
    id "gold"
    name "Gold"
    tg_id "material0"
  </Material 0>

  <Material 1>
    id "wood"
    name "Wood"
    tg_id "material1"
  </Material 1>

  <Material 2>
    id "coal"
    name "Coal"
    tg_id "material2"
  </Material 2>
  
</Materials>


#
#  Terrain Segments
#

<Segments>

  <Segment 0>
    name "Underground"
    count 7

    <Terrain type 0>
      name "Sea"
      layer 5
      difficulty 0
    </Terrain type 0>
    
    <Terrain type 1>
      name "Flat"
      layer 7
      difficulty 0
    </Terrain type 1>
    
    <Terrain type 2>
      name "Beach"
      layer 8
      difficulty 0
    </Terrain type 2>

    <Terrain type 3>
      name "Grass"
      layer 10
      difficulty 0
    </Terrain type 3>

    <Terrain type 4>
      name "Rocks"
      layer 30
      difficulty 0
    </Terrain type 4>

    <Terrain type 5>
      name "Unmoveable"
      layer 255
      difficulty 0.255
    </Terrain type 5>

    <Terrain type 6>
      name "Nonexist"
      layer 0
      difficulty 0.255
    </Terrain type 6>
  
    #
    #  Terrain Fragments
    #

    <Fragments>
	    tg_default_id "default"
	    default_terrain_id 10
	    count 29

	    <Fragment 0>
		    size 5
		    tg_id "ug_grass"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 0>
	    
	    <Fragment 1>
		    size 5
		    tg_id "ug_grass_border_e"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 255 255 255 255 255
	    </Fragment 1>
	    
	    <Fragment 2>
		    size 5
		    tg_id "ug_grass_border_n"
		    terrain_id 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255
	    </Fragment 2>
	    
	    <Fragment 3>
		    size 5
		    tg_id "ug_sea"
		    terrain_id 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
	    </Fragment 3>
	    
	    <Fragment 4>
		    size 5
		    tg_id "ug_sea_border_e"
		    terrain_id 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 255 255 255 255 255
	    </Fragment 4>
	    
	    <Fragment 5>
		    size 5
		    tg_id "ug_sea_border_n"
		    terrain_id 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255
	    </Fragment 5>
	    
	    <Fragment 6>
		    size 5
		    tg_id "ug_coast_e"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 8 8 8 8 8 7 7 7 7 7 5 5 5 5 5
	    </Fragment 6>
	    
	    <Fragment 7>
		    size 5
		    tg_id "ug_coast_e_border_n"
		    terrain_id 10 10 10 10 255 10 10 10 10 255 8 8 8 8 255 7 7 7 7 255 5 5 5 5 255
	    </Fragment 7>
	    
	    <Fragment 8>
		    size 5
		    tg_id "ug_coast_en"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 8 8 10 10 8 7 7 10 10 8 7 5
	    </Fragment 8>
	    
	    <Fragment 9>
		    size 5
		    tg_id "ug_coast_es"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 8 8 10 10 10 7 7 8 10 10 5 7 8 10 10
	    </Fragment 9>
	    
	    <Fragment 10>
		    size 5
		    tg_id "ug_coast_n"
		    terrain_id 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5
	    </Fragment 10>
	    
	    <Fragment 11>
		    size 5
		    tg_id "ug_coast_ne"
		    terrain_id 10 10 8 7 5 10 8 8 7 5 8 8 8 7 5 7 7 7 7 5 5 5 5 5 5
	    </Fragment 11>
	    
	    <Fragment 12>
		    size 5
		    tg_id "ug_coast_nw"
		    terrain_id 5 5 5 5 5 7 7 7 7 5 8 8 8 7 5 10 8 8 7 5 10 10 5 7 5
	    </Fragment 12>
	    
	    <Fragment 13>
		    size 5
		    tg_id "ug_coast_s"
		    terrain_id 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10
	    </Fragment 13>
	    
	    <Fragment 14>
		    size 5
		    tg_id "ug_coast_se"
		    terrain_id 5 7 8 10 10 5 7 8 8 10 5 7 8 8 8 5 7 7 7 7 5 5 5 5 5
	    </Fragment 14>
	    
	    <Fragment 15>
		    size 5
		    tg_id "ug_coast_sw"
		    terrain_id 5 5 5 5 5 5 7 7 7 7 5 7 8 8 8 5 7 8 8 10 5 7 8 10 10
	    </Fragment 15>
	    
	    <Fragment 16>
		    size 5
		    tg_id "ug_coast_w"
		    terrain_id 5 5 5 5 5 7 7 7 7 7 8 8 8 8 8 10 10 10 10 10 10 10 10 10 10
	    </Fragment 16>
	    
	    <Fragment 17>
		    size 5
		    tg_id "ug_coast_w_border_n"
		    terrain_id 5 5 5 5 255 7 7 7 7 255 8 8 8 8 255 10 10 10 10 255 10 10 10 10 255
	    </Fragment 17>
	    
	    <Fragment 18>
		    size 5
		    tg_id "ug_coast_wn"
		    terrain_id 10 10 8 7 5 10 10 8 7 7 10 10 10 8 8 10 10 10 10 10 10 10 10 10 10
	    </Fragment 18>
	    
	    <Fragment 19>
		    size 5
		    tg_id "ug_coast_ws"
		    terrain_id 5 7 8 10 10 7 7 8 10 10 8 8 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 19>
	    
	    <Fragment 20>
		    size 5
		    tg_id "ug_grass_border_s"
		    terrain_id 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10
	    </Fragment 20>
	    
	    <Fragment 21>
		    size 5
		    tg_id "ug_grass_border_w"
		    terrain_id 255 255 255 255 255 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 21>
	    
	    <Fragment 22>
		    size 5
		    tg_id "ug_grass_border_sw"
		    terrain_id 255 255 255 255 255 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10
	    </Fragment 22>
	    
	    <Fragment 23>
		    size 5
		    tg_id "ug_sea_border_w"
		    terrain_id 255 255 255 255 255 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
	    </Fragment 23>
	    
	    <Fragment 24>
		    size 5
		    tg_id "ug_coast_n_border_w"
		    terrain_id 255 255 255 255 255 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5
	    </Fragment 24>
	    
	    <Fragment 25>
		    size 5
		    tg_id "ug_coast_s_border_w"
		    terrain_id 255 255 255 255 255 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10
	    </Fragment 25>
	    
	    <Fragment 26>
		    size 5
		    tg_id "ug_grass_border_ne"
		    terrain_id 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 255 255 255 255 255
	    </Fragment 26>
	    
	    <Fragment 27>
		    size 5
		    tg_id "ug_grass_border_nw"
		    terrain_id 255 255 255 255 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255
	    </Fragment 27>
	    
	    <Fragment 28>
		    size 5
		    tg_id "ug_grass_border_se"
		    terrain_id 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 255 255 255 255
	    </Fragment 28>
	    
	  </Fragments>

    #
    #  Terrain Layers
    #


    <Layers>
      count 0
    </Layers>

    #
    #  Terrain Objects
    #

    <Objects>
      count 0
    </Objects>

  </Segment 0>

  <Segment 1>
    count 9
    name "Earth"
    
    <Terrain type 0>
      name "Sea"
      layer 5
      difficulty 0
    </Terrain type 0>
    
    <Terrain type 1>
      name "Flat"
      layer 7
      difficulty 0
    </Terrain type 1>
    
    <Terrain type 2>
      name "Beach"
      layer 8
      difficulty 0
    </Terrain type 2>

    <Terrain type 3>
      name "Grass"
      layer 10
      difficulty 0
    </Terrain type 3>

    <Terrain type 4>
	    name "Swamp"
	    layer 20
	    difficulty 0.5
	  </Terrain type 4>

    <Terrain type 5>
	    name "Rocky grass"
	    layer 25
	    difficulty 0.5
	  </Terrain type 5>

    <Terrain type 6>
      name "Rocks"
      layer 30
      difficulty 0
    </Terrain type 6>

    <Terrain type 7>
      name "Unmoveable"
      layer 255
      difficulty 0.255
    </Terrain type 7>

    <Terrain type 8>
      name "Nonexist"
      layer 0
      difficulty 0.255
    </Terrain type 8>

    #
    #  Terrain Fragments
    #

	  <Fragments>
		  tg_default_id "default"
		  default_terrain_id 10
		  count 80

	    <Fragment 0>
		    size 5
		    tg_id "grass"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 0>

	    <Fragment 1>
		    size 5
		    tg_id "rocks_s"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 1>

      <Fragment 2>
		    size 5
		    tg_id "rocks_sw"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 2>

      <Fragment 3>
		    size 5
		    tg_id "rocks_w"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 3>

      <Fragment 4>
		    size 5
		    tg_id "rocks_se"
		    terrain_id 30 30 30 30 30 10 30 30 30 30 10 10 10 30 30 10 10 10 10 30 10 10 10 10 10
	    </Fragment 4>

      <Fragment 5>
		    size 5
		    tg_id "rocks_e"
		    terrain_id 10 10 10 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 10 10 10 10 10
	    </Fragment 5>

      <Fragment 6>
		    size 5
		    tg_id "rocks_nw"
		    terrain_id 10 10 10 10 10 30 10 10 10 10 30 30 10 10 10 30 30 30 10 10 30 30 30 10 10
	    </Fragment 6>

      <Fragment 7>
		    size 5
		    tg_id "rocks_n"
		    terrain_id 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10
	    </Fragment 7>

      <Fragment 8>
		    size 5
		    tg_id "rocks_ne"
		    terrain_id 10 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 10 10 10 10 10 10 10
	    </Fragment 8>

      <Fragment 9>
		    size 5
		    tg_id "grass_border_w"
		    terrain_id 255 255 255 255 255 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 9>

      <Fragment 10>
		    size 5
		    tg_id "grass_borderh_w"
		    terrain_id 255 255 255 255 255 255 255 255 255 255 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 10>

      <Fragment 11>
		    size 5
		    tg_id "rocks_s_border_w"
		    terrain_id 255 255 255 255 255 255 255 255 255 255 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 11>

      <Fragment 12>
		    size 5
		    tg_id "grass_border_s"
		    terrain_id 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10
	    </Fragment 12>

      <Fragment 13>
		    size 5
		    tg_id "grass_borderh_s"
		    terrain_id 255 255 10 10 10 255 255 10 10 10 255 255 10 10 10 255 255 10 10 10 255 255 10 10 10
	    </Fragment 13>

      <Fragment 14>
		    size 5
		    tg_id "grass_border_sw"
		    terrain_id 255 255 255 255 255 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10
	    </Fragment 14>

      <Fragment 15>
		    size 5
		    tg_id "grass_borderh_nw"
		    terrain_id 255 255 255 255 255 255 255 255 255 255 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 15>

      <Fragment 16>
		    size 5
		    tg_id "grass_borderh_se"
		    terrain_id 255 255 10 10 10 255 255 10 10 10 255 255 10 10 10 255 255 10 10 10 255 255 10 10 10
	    </Fragment 16>

      <Fragment 17>
		    size 5
		    tg_id "rocks_w_border_s"
		    terrain_id 255 30 30 30 30 255 30 30 30 30 255 30 30 30 30 255 30 30 30 30 255 30 30 30 30
	    </Fragment 17>

      <Fragment 18>
		    size 5
		    tg_id "rocks_s_border_e"
		    terrain_id 30 30 30 30 255 30 30 30 30 255 30 30 30 30 255 30 30 30 30 255 30 30 30 30 30
	    </Fragment 18>

      <Fragment 19>
		    size 5
		    tg_id "grass_border_e"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 255 255 255 255 255
	    </Fragment 19>

      <Fragment 20>
		    size 5
		    tg_id "rocks_n_border_e"
		    terrain_id 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10 10 30 30 30 255
	    </Fragment 20>

      <Fragment 21>
		    size 5
		    tg_id "rocks_ws"
		    terrain_id 30 30 30 30 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 10
	    </Fragment 21>

      <Fragment 22>
		    size 5
		    tg_id "rocks_wn"
		    terrain_id 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30
	    </Fragment 22>

      <Fragment 23>
		    size 5
		    tg_id "rocks_s_end_w"
		    terrain_id 25 25 25 25 25 25 25 25 25 25 25 25 25 25 25 25 30 30 30 30 30 30 30 30 30
	    </Fragment 23>

      <Fragment 24>
		    size 5
		    tg_id "rocks_s_end_e"
		    terrain_id 30 30 30 30 30 25 30 30 30 30 25 25 30 30 30 25 25 25 25 25 25 25 25 25 25
	    </Fragment 24>

      <Fragment 25>
		    size 5
		    tg_id "rocks_w_end_n"
		    terrain_id 30 30 25 25 25 30 30 30 25 25 30 30 30 30 25 30 30 30 30 25 30 30 30 30 25
	    </Fragment 25>

      <Fragment 26>
		    size 5
		    tg_id "rocks_w_end_s"
		    terrain_id 25 25 25 25 30 25 25 25 30 30 25 25 25 30 30 25 25 25 30 30 25 25 25 30 30
	    </Fragment 26>

      <Fragment 27>
		    size 5
		    tg_id "rocks_w_border_n"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 255 30 30 30 30 30
	    </Fragment 27>

      <Fragment 28>
		    size 5
		    tg_id "rocks_e_border_n"
		    terrain_id 10 10 10 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 255 10 10 10 10 255
	    </Fragment 28>

	    <Fragment 29>
		    size 5
		    tg_id "sea"
		    terrain_id 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
	    </Fragment 29>
	    
	    <Fragment 30>
		    size 5
		    tg_id "coast_n"
		    terrain_id 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5
	    </Fragment 30>
	    
	    <Fragment 31>
		    size 5
		    tg_id "coast_w"
		    terrain_id 5 5 5 5 5 7 7 7 7 7 8 8 8 8 8 10 10 10 10 10 10 10 10 10 10
	    </Fragment 31>
	    
	    <Fragment 32>
		    size 5
		    tg_id "coast_e"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 8 8 8 8 8 7 7 7 7 7 5 5 5 5 5
	    </Fragment 32>
	    
	    <Fragment 33>
		    size 5
		    tg_id "coast_s"
		    terrain_id 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10
	    </Fragment 33>
	    
	    <Fragment 34>
		    size 5
		    tg_id "coast_wn"
		    terrain_id 10 10 8 7 5 10 10 8 7 7 10 10 10 8 8 10 10 10 10 10 10 10 10 10 10
	    </Fragment 34>
	    
	    <Fragment 35>
		    size 5
		    tg_id "coast_en"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 8 8 10 10 8 7 7 10 10 8 7 5
	    </Fragment 35>
	    
	    <Fragment 36>
		    size 5
		    tg_id "coast_es"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 8 8 10 10 10 7 7 8 10 10 5 7 8 10 10
	    </Fragment 36>
	    
	    <Fragment 37>
		    size 5
		    tg_id "coast_ne"
		    terrain_id 10 10 8 7 5 10 8 8 7 5 8 8 8 7 5 7 7 7 7 5 5 5 5 5 5
	    </Fragment 37>
	    
	    <Fragment 38>
		    size 5
		    tg_id "coast_nw"
		    terrain_id 5 5 5 5 5 7 7 7 7 5 8 8 8 7 5 10 8 8 7 5 10 10 5 7 5
	    </Fragment 38>
	    
	    <Fragment 39>
		    size 5
		    tg_id "coast_se"
		    terrain_id 5 7 8 10 10 5 7 8 8 10 5 7 8 8 8 5 7 7 7 7 5 5 5 5 5
	    </Fragment 39>
	    
	    <Fragment 40>
		    size 5
		    tg_id "coast_sw"
		    terrain_id 5 5 5 5 5 5 7 7 7 7 5 7 8 8 8 5 7 8 8 10 5 7 8 10 10
	    </Fragment 40>
	    
	    <Fragment 41>
		    size 5
		    tg_id "coast_ws"
		    terrain_id 5 7 8 10 10 7 7 8 10 10 8 8 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 41>
	    
	    <Fragment 42>
		    size 5
		    tg_id "rocks_n_border_w"
		    terrain_id 255 255 255 255 255 255 255 30 30 10 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10
	    </Fragment 42>
	    
	    <Fragment 43>
		    size 5
		    tg_id "grass"
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 43>
	    
	    <Fragment 44>
		    size 5
		    tg_id "cliff_e"
		    terrain_id 10 10 10 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 5 5 5 5 5
	    </Fragment 44>
	    
	    <Fragment 45>
		    size 5
		    tg_id "cliff_n"
		    terrain_id 10 30 30 30 5 10 30 30 30 5 10 30 30 30 5 10 30 30 30 5 10 30 30 30 5
	    </Fragment 45>
	    
	    <Fragment 46>
		    size 5
		    tg_id "cliff_ne"
		    terrain_id 10 30 30 30 5 30 30 30 30 5 30 30 30 30 5 30 30 30 5 5 5 5 5 5 5
	    </Fragment 46>
	    
	    <Fragment 47>
		    size 5
		    tg_id "cliff_nw"
		    terrain_id 5 5 5 5 5 30 5 5 5 5 30 30 5 5 5 30 30 30 5 5 30 30 30 5 5
	    </Fragment 47>
	    
	    <Fragment 48>
		    size 5
		    tg_id "cliff_s"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 48>
	    
	    <Fragment 49>
		    size 5
		    tg_id "cliff_se"
		    terrain_id 30 30 30 30 30 5 30 30 30 30 5 5 5 30 30 5 5 5 5 30 5 5 5 5 5
	    </Fragment 49>
	    
	    <Fragment 50>
		    size 5
		    tg_id "cliff_sw"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 50>
	    
	    <Fragment 51>
		    size 5
		    tg_id "cliff_w"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 51>
	    
	    <Fragment 52>
		    size 5
		    tg_id "cliff_wn"
		    terrain_id 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30
	    </Fragment 52>
	    
	    <Fragment 53>
		    size 5
		    tg_id "cliff_ws"
		    terrain_id 30 30 30 30 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 10
	    </Fragment 53>
	    
	    <Fragment 54>
		    size 5
		    tg_id "rocks_n_coast_e"
		    terrain_id 10 30 30 30 10 10 30 30 30 10 10 30 30 30 8 10 30 30 30 7 10 30 30 30 5
	    </Fragment 54>
	    
	    <Fragment 55>
		    size 5
		    tg_id "rocks_n_coast_w"
		    terrain_id 10 30 30 30 5 10 30 30 30 7 10 30 30 30 8 10 30 30 30 10 10 30 30 30 10
	    </Fragment 55>
	    
	    <Fragment 56>
		    size 5
		    tg_id "rocks_s_coast_e"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 56>
	    
	    <Fragment 57>
		    size 5
		    tg_id "rocks_s_coast_w"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 57>
	    
	    <Fragment 58>
		    size 5
		    tg_id "rocks_e_coast_n"
		    terrain_id 10 10 10 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 10 10 8 7 5
	    </Fragment 58>
	    
	    <Fragment 59>
		    size 5
		    tg_id "rocks_e_coast_s"
		    terrain_id 5 7 8 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 10 10 10 10 10
	    </Fragment 59>
	    
      <Fragment 60>
		    size 5
		    tg_id "rocks_w_coast_e"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 60>
	    
	    <Fragment 61>
		    size 5
		    tg_id "rocks_w_coast_n"
		    terrain_id 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 61>
	    
	    <Fragment 62>
		    size 5
		    tg_id "grass_border_n"
		    terrain_id 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255 10 10 10 10 255
	    </Fragment 62>
	    
	    <Fragment 63>
		    size 5
		    tg_id "sea_border_n"
		    terrain_id 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255
	    </Fragment 63>
	    
	    <Fragment 64>
		    size 5
		    tg_id "sea_border_e"
		    terrain_id 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 255 255 255 255 255
	    </Fragment 64>
	    
	    <Fragment 65>
		    size 5
		    tg_id "sea_border_s"
		    terrain_id 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5 255 5 5 5 5
	    </Fragment 65>
	    
	    <Fragment 66>
		    size 5
		    tg_id "sea_border_w"
		    terrain_id 255 255 255 255 255 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
	    </Fragment 66>
	    
	    <Fragment 67>
		    size 5
		    tg_id "coast_e_border_n"
		    terrain_id 10 10 10 10 255 10 10 10 10 255 8 8 8 8 255 7 7 7 7 255 5 5 5 5 255
	    </Fragment 67>
	    
	    <Fragment 68>
		    size 5
		    tg_id "coast_w_border_n"
		    terrain_id 5 5 5 5 255 7 7 7 7 255 8 8 8 8 255 10 10 10 10 255 10 10 10 10 255
	    </Fragment 68>
	    
	    <Fragment 69>
		    size 5
		    tg_id "coast_n_border_w"
		    terrain_id 255 255 255 255 255 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5 10 10 8 7 5
	    </Fragment 69>
	    
	    <Fragment 70>
		    size 5
		    tg_id "coast_s_border_w"
		    terrain_id 255 255 255 255 255 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10 5 7 8 10 10
	    </Fragment 70>
	    
	    <Fragment 71>
		    size 5
		    tg_id "rocks_e_border_s"
		    terrain_id 255 10 10 10 10 255 30 30 30 30 255 30 30 30 30 255 30 30 30 30 255 10 10 10 10
	    </Fragment 71>
	    
	    <Fragment 72>
		    size 5
		    tg_id "rocks_es"
		    terrain_id 10 10 10 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 72>
	    
	    <Fragment 73>
		    size 5
		    tg_id "rocks_en"
		    terrain_id 10 10 10 10 10 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 10
	    </Fragment 73>
	    
	    <Fragment 74>
		    size 5
		    tg_id "cliff_es"
		    terrain_id 10 10 10 10 10 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
	    </Fragment 74>
	    
	    <Fragment 75>
		    size 5
		    tg_id "cliff_en"
		    terrain_id 10 10 10 10 10 10 30 30 30 30 10 30 30 30 30 10 30 30 30 30 10 30 30 30 5
	    </Fragment 75>
	    
	    <Fragment 76>
		    size 5
		    tg_id "rocks_e_end_n"
		    terrain_id 10 10 10 10 10 30 30 30 25 25 30 30 30 25 25 30 30 30 25 25 10 10 10 10 10
	    </Fragment 76>
	    
	    <Fragment 77>
		    size 5
		    tg_id "rocks_e_end_s"
		    terrain_id 10 10 10 10 10 25 25 25 30 30 25 25 25 30 30 25 25 25 30 30 10 10 10 10 10
	    </Fragment 77>
	    
	    <Fragment 78>
		    size 5
		    tg_id "rocks_n_end_e"
		    terrain_id 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10 10 25 25 25 10 10 25 25 25 10
	    </Fragment 78>
	    
	    <Fragment 79>
		    size 5
		    tg_id "rocks_n_end_w"
		    terrain_id 10 25 25 25 10 10 25 25 25 10 10 30 30 30 10 10 30 30 30 10 10 30 30 30 10
	    </Fragment 79>

	  </Fragments>

    #
    #  Terrain Layers
    #


    <Layers>
      count 1

      <Layer 0>
        id "sand"
        name "Sand"
        width 5
        height 5
        tg_id "send"
        terrain_id 10 10 10 10 10 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 10 10 10 10 10
      </Layer 0>
    </Layers>

    #
    #  Terrain Objects
    #

    <Objects>
      count 4

      <Object 0>
        id "runestone"
        name "Runestone"
        width 2
        height 2
        tg_id "runestone"
        terrain_id 30 30 30 30
      </Object 0>
      
      <Object 1>
        id "deadtree"
        name "Dead Tree"
        width 1
        height 1
        tg_id "deadtree"
        terrain_id 30
      </Object 1>
      
      <Object 2>
        id "boat_w"
        name "Boat"
        width 2
        height 2
        tg_id "boat_w"
        terrain_id 30 30 30 30
      </Object 2>
      
      <Object 3>
        id "fishman"
        name "Fishman"
        width 2
        height 2
        tg_id "fishman"
        terrain_id 30 30 30 30
      </Object 3>

    </Objects>

  </Segment 1>


  <Segment 2>
    count 3
    name "Air"

    <Terrain type 0>
      name "Air"
      layer 10
      difficulty 0
    </Terrain type 0>

    <Terrain type 1>
      name "Clouds"
      layer 20
      difficulty 0.1
    </Terrain type 1>

	<Terrain type 2>
      name "Nonexist"
      layer 0
      difficulty 0.255
    </Terrain type 2>

    #
    #  Terrain Fragments
    #

	  <Fragments>
      tg_default_id none
      default_terrain_id 10
	    count 2

	    <Fragment 0>
		    size 5
		    tg_id none
		    terrain_id 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
	    </Fragment 0>

	    <Fragment 1>
		    size 5
		    tg_id "clouds"
		    terrain_id 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20
	    </Fragment 1>

	  </Fragments>

    #
    #  Terrain Layers
    #


    <Layers>
      count 0
    </Layers>

    #
    #  Terrain Objects
    #

    <Objects>
      count 0
    </Objects>

  </Segment 2>

</Segments>
