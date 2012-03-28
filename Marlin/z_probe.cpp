#include "z_probe.h"
#if defined(PROBE_PIN) && (PROBE_PIN > -1)
#include "Marlin.h"
#include "stepper.h"

#define PROBE_DOWN

#ifdef PROBE_DOWN
float Probe_Bed(float x_pos, float y_pos, int n)
{
    //returns Probed Z average height
    float ProbeDepth[n];
    float ProbeDepthAvg=0;
    
    if (Z_HOME_DIR==-1)
    {
      //int probe_flag =1;
      int fails = 0;
      saved_feedrate = feedrate;
      saved_feedmultiply = feedmultiply;
      feedmultiply = 100;
      //previous_millis_cmd = millis();
      if (x_pos >= 0) destination[X_AXIS]=x_pos;
      if (y_pos >= 0) destination[Y_AXIS]=y_pos;
      destination[Z_AXIS]=current_position[Z_AXIS];
      feedrate = 9000;
      prepare_move();

	  enable_endstops(true);
      SERIAL_ECHO("PRE-PROBE current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);

	  SERIAL_ECHOLN("Ready to probe...");

        //Probe bed n times
        //*******************************************************************************************Bed Loop*************************************
        for(int8_t i=0; i < n ; i++)
        {
            //int z = 0;

			//current_position[Z_AXIS] = 0; 
			plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]); 
			destination[Z_AXIS] = 1.1 * Z_MAX_LENGTH * Z_HOME_DIR; 
			feedrate = fast_home_feedrate[Z_AXIS]; 
			plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder); 
			st_synchronize();
		
			//current_position[Z_AXIS] = 0;
			plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
			destination[Z_AXIS] = current_position[Z_AXIS] - Z_HOME_RETRACT_MM * Z_HOME_DIR;
			plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder); 
			st_synchronize();
		
			plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
			destination[Z_AXIS] = current_position[Z_AXIS] + 2*Z_HOME_RETRACT_MM * Z_HOME_DIR;
			feedrate = homing_feedrate[Z_AXIS];  
			plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder); 
			st_synchronize();
		
			//current_position[Z_AXIS] = (Z_HOME_DIR == -1) ? Z_HOME_POS : Z_MAX_LENGTH;
			//destination[Z_AXIS] = current_position[Z_AXIS];
			feedrate = 0.0;
			//endstops_hit_on_purpose();
            
            SERIAL_ECHO("current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);
            if(endstop_z_hit == true)
            {
	            ProbeDepth[i]= endstops_trigsteps[Z_AXIS] / axis_steps_per_unit[Z_AXIS];
	            SERIAL_ECHO("ProbeDepth[");SERIAL_ECHO(i);SERIAL_ECHO("]=");SERIAL_ECHOLN(ProbeDepth[i]);
            	//*************************************************************************************************************
		        if (i > 0 ) //Second probe has happened so compare results
		        {
		            if (abs(ProbeDepth[i] - ProbeDepth[i - 1]) > .05)
		            { //keep going until readings match to avoid sticky bed
		              SERIAL_ECHO("Probing again: ");
		              SERIAL_ECHO(ProbeDepth[i]); SERIAL_ECHO(" - "); SERIAL_ECHO(ProbeDepth[i - 1]);SERIAL_ECHO(" = "); SERIAL_ECHOLN(abs(ProbeDepth[i] - ProbeDepth[i - 1]));
		              i--; i--; //Throw out both that don't match because we don't know which one is accurate
		              if(fails++ > 4) break;
		            }
		        }
	        }else{
	        	SERIAL_ECHOLN("Probe not triggered.");
	        	i=n-1;
	        }
            //**************************************************************************************************************************************************
            //fast move clear
            //z = 0;
		    //current_position[Z_AXIS] = 0;
		    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
		    destination[Z_AXIS] = current_position[Z_AXIS]-Z_HOME_RETRACT_MM * Z_HOME_DIR;
		    feedrate = fast_home_feedrate[Z_AXIS];
		    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
		    st_synchronize();
		    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

            //check z stop isn't still triggered
            if ( READ(X_MIN_PIN) != X_ENDSTOPS_INVERTING )
            {
                SERIAL_ECHOLN("Poking Stuck Bed:");
                destination[Z_AXIS] = -1; prepare_move();
                destination[Z_AXIS] = Z_HOME_RETRACT_MM; prepare_move();
			    st_synchronize();
                i--; //Throw out this meaningless measurement
            }
            feedrate = 0;
        } //end probe loop
		#ifdef ENDSTOPS_ONLY_FOR_HOMING
		  enable_endstops(false);
		#endif
		 
		feedrate = saved_feedrate;
		feedmultiply = saved_feedmultiply;
		//previous_millis_cmd = millis();
		endstops_hit_on_purpose();
    }
    for(int8_t i=0;i<n;i++)
    {
    	ProbeDepthAvg += ProbeDepth[i];
    }
    ProbeDepthAvg /= n;
    SERIAL_ECHO("Probed Z="); SERIAL_ECHOLN(ProbeDepthAvg);
    SERIAL_ECHO("RAW current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);
//    current_position[Z_AXIS]+=ProbeDepthAvg;
//    SERIAL_ECHO("ADJUSTED current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

    return ProbeDepthAvg;
 }
#else
//Crash1 - Probes bed at least twice until distances are similar then takes average of 2
float Probe_Bed(float x_pos, float y_pos) 
{   
    //returns Probed Z average height
    float ProbeDepth[6], ProbeDepthAvg;
    if (Z_HOME_DIR==-1)
    {
        saved_feedrate = feedrate;        
        destination[Z_AXIS] = 3; //* Z_HOME_DIR;  //Lift over bed for initial Move
        feedrate = homing_feedrate[Z_AXIS];
        prepare_move();
        
         //Move to Probe Coordinates, Use current Position if none given
        if (x_pos < 0) 
            destination[X_AXIS] = current_position[X_AXIS];
        else 
            destination[X_AXIS] = x_pos;   
        if (y_pos < 0) 
            destination[Y_AXIS] = current_position[Y_AXIS];
        else 
            destination[Y_AXIS] = y_pos;
        feedrate = 7500; // 500 is way too slow- WTF  - max_feedrate[X_AXIS]; //homing_feedrate[X_AXIS]; //250;
        prepare_move();
        
        destination[Z_AXIS] = .75; //* Z_HOME_DIR;  //move close to Z Home - bed should be within .75mm of level 
        feedrate = homing_feedrate[Z_AXIS];
        prepare_move();
         
        //Plunge down final distance slowly until bed breaks contact and pin true
        //*******************************************************************************************Bed Loop*************************************
        for(int8_t i=0; i < 2 ; i++) 
        {    //probe 2 or more times to get repeatable reading    
            //2DO If bed currently true then it is stuck - need to do something smart here.         
            int probeSteps = 500;  //distance to move down and max distance to scan up in one step.
            int z = 0;
            while(READ(PROBE_PIN) == false && z < probeSteps)
            {  //if it takes more than 500 steps then something is wrong
                destination[Z_AXIS] = current_position[Z_AXIS] - Z_INCREMENT; //* Z_HOME_DIR;
                feedrate = homing_feedrate[Z_AXIS];
                prepare_move();
                z++;
            }
              //move up in small increments until switch makes
            z = 0;
            while(READ(PROBE_PIN) == true && z < probeSteps)
            {  //if it takes more than 100 steps then bed is likely stuck - still need to error on this to stop process
                destination[Z_AXIS] = current_position[Z_AXIS] + Z_INCREMENT; //* Z_HOME_DIR;
                feedrate = homing_feedrate[Z_AXIS];
                prepare_move(); 
                z++;
            }
            //**************************************************************************************************************************************************          
            //if Z is probeSteps here then we have a stuck bed and it will keep on advancing upward. So send hot end toward Zstop to try to unstick.
            if (z == probeSteps) 
            {
                SERIAL_ECHOLN("Poking Stuck Bed:"); 
                destination[Z_AXIS] = 1; feedrate = homing_feedrate[Z_AXIS]; prepare_move();
                destination[Z_AXIS] = .2; feedrate = homing_feedrate[Z_AXIS]; prepare_move();
                destination[Z_AXIS] = 1; feedrate = homing_feedrate[Z_AXIS]; prepare_move();
                i--; //Throw out this meaningless probe
                z == 0;
            }  
            //*************************************************************************************************************
            ProbeDepth[i]= current_position[Z_AXIS];
            if (i == 1 ) 
            {
                if (abs(ProbeDepth[i] - ProbeDepth[i - 1]) > .02) 
                {     //keep going until readings match to avoid sticky bed
                    SERIAL_ECHO("Probing again - difference:"); SERIAL_ECHO(abs(ProbeDepth[i] - ProbeDepth[i - 1])); SERIAL_ECHO(", Z="); SERIAL_ECHOLN(ProbeDepth[i]); 
                    i -= 2;    //Throw out both that don't match because we don't know which one is accurate
                }
            }  
            feedrate = 0;
        } //end probe loop
        feedrate = saved_feedrate;        
    }
    ProbeDepthAvg = (ProbeDepth[0] + ProbeDepth[1]) / 2;
    SERIAL_ECHO("Z="); SERIAL_ECHOLN(ProbeDepthAvg); 
    return ProbeDepthAvg;
 }
//Crash1 END Add Probe Bed Function
#endif
void probe_init()
{
    SET_INPUT(PROBE_PIN);
    WRITE(PROBE_PIN,HIGH);
}

/*Crash1 - G29 to Probe and stop on Bed
G29 will probe bed at least twice at 3 points and take an average. G30 will probe bed at it's current location.
Z stop should be set slightly below bed height. Solder stub wire to each hole in huxley bed and attach a ring terminal under spring.
Wire bed probe to A2 on Melzi and duplicate cap/resistor circuit in schematic.
  
Use something like this in the start.gcode file:
G29 		;Probe bed for Z height
G92 Z0		;Set Z to Probed Depth
G1 Z5 F200	;Lift Z out of way
*/
void probe_3points()
{
    float Probe_Avg, Point1, Point2, Point3;
    Point1 = Probe_Bed(15,15,PROBE_N);
    Point2 = Probe_Bed(X_MAX_LENGTH - 20,15,PROBE_N) ;
    Point3 = Probe_Bed(X_MAX_LENGTH/2,Y_MAX_LENGTH - 5,PROBE_N);
    Probe_Avg = (Point1 + Point2 + Point3) / 3;
    //destination[2] = Probe_Avg;
    //feedrate = homing_feedrate[Z_AXIS];
    //prepare_move();
    SERIAL_ECHOLN("**************************************");       
    SERIAL_ECHO("Point1 ="); SERIAL_ECHOLN(Point1);
    SERIAL_ECHO("Point2 ="); SERIAL_ECHOLN(Point2);
    SERIAL_ECHO("Point3 ="); SERIAL_ECHOLN(Point3);
    SERIAL_ECHO("Probed Average="); SERIAL_ECHOLN(Probe_Avg);
    SERIAL_ECHOLN("**************************************");       
}

void probe_1point()
{
    float  Point;
    Point = Probe_Bed(-1,-1,PROBE_N);
    //destination[2] = Point +1;
    //feedrate = homing_feedrate[Z_AXIS];
    //prepare_move();
    SERIAL_ECHOLN("**************************************");       
    SERIAL_ECHO("Probed Z="); SERIAL_ECHOLN(Point);
}

void probe_status()
{
    SERIAL_ECHO("Probe Status = "); SERIAL_ECHOLN(READ(PROBE_PIN));
}

#endif //defined(PROBE_PIN) > -1


